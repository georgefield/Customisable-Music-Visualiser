#include "miniaudio_api.h"
#include "VisVars.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <cassert>
#include <Vengine/MyErrors.h>
#include <Vengine/MyTiming.h>

ma_engine _engine;

ma_sound _sound;

ma_decoder_config _decoderConfig;
ma_decoder _decoder;

miniaudio_api::miniaudio_api() :
    _initialised(false),
    _fileLoaded(false),
    _memoryAllocated(false),
    _filePath(""),
    _loadedAudioData(nullptr),
    _currentSampleTimerId(-1)
{
}

miniaudio_api::~miniaudio_api()
{
    ma_engine_uninit(&_engine);
}

void miniaudio_api::init(){
    _initialised = initEngine();
    Vengine::MyTiming::createTimer(_currentSampleTimerId);
}

bool miniaudio_api::loadAudio(const std::string& filePath)
{
    assert(_initialised);

    _filePath = filePath;

    if (!initDecoderFromFile()) {
        return false;
    }
    _decoderInitialised = true;

    if (!initSoundFromFile()) {
        return false;
    }
    _soundInitialised = true;

    ma_uint64 numPCMframes;
    ma_result resultGetNumFrames = ma_decoder_get_length_in_pcm_frames(&_decoder, &numPCMframes);
    if (resultGetNumFrames != MA_SUCCESS) {
        Vengine::warning("Could not get audio length");
        return false;
    }
    if (numPCMframes > INT32_MAX) {
        Vengine::warning("Audio longer than max audio allowed (2,147,483,647 samples)");
        return false;
    }
    _loadedAudioDataLength = int(numPCMframes); //we know it can be safely truncated, int32 max gives us ~6 hours audio max length

    _loadedAudioData = new float[_loadedAudioDataLength];
    _memoryAllocated = true;

    ma_uint64 framesRead;
    ma_result resultGetData = ma_decoder_read_pcm_frames(&_decoder, _loadedAudioData, _loadedAudioDataLength, &framesRead);
    if (resultGetData != MA_SUCCESS) {
        Vengine::warning("Could not get audio data");
        return false;
    }

    if (framesRead != _loadedAudioDataLength) {
        Vengine::warning("frames read =/= num frames, possible problem, file still loaded");
    }

    std::cout << ma_engine_get_sample_rate(&_engine) << " Engine sample rate" << std::endl;
    std::cout << _decoder.outputSampleRate << " Decoder sample rate" << std::endl;

    Vengine::MyTiming::resetTimer(_currentSampleTimerId);

    _fileLoaded = true;
    return true;
}

void miniaudio_api::unloadAudio()
{
    _fileLoaded = false;
    _filePath = "";

    if (_decoderInitialised) {
        std::cout << "uninit decoder";
        ma_result decResult = ma_decoder_uninit(&_decoder);
        if (decResult != MA_SUCCESS) {
            Vengine::warning("Error unloading decoder");
        }
        _decoderInitialised = false;
    }

    if (_soundInitialised) {
        std::cout << "uninit sound";
        ma_sound_uninit(&_sound);
        _soundInitialised = false;
    }

    if (_memoryAllocated) {
        delete[] _loadedAudioData;
        _memoryAllocated = false;
    }
}

void miniaudio_api::playAudio()
{
    assert(_fileLoaded);

    ma_sound_start(&_sound);
    Vengine::MyTiming::startTimer(_currentSampleTimerId);
}

void miniaudio_api::pauseAudio()
{
    assert(_fileLoaded);

    ma_sound_stop(&_sound);
    Vengine::MyTiming::stopTimer(_currentSampleTimerId);
}

void miniaudio_api::seekToSample(int sample)
{
    assert(_fileLoaded);
    assert(sample >= 0);
    assert(sample < _loadedAudioDataLength);

    Vengine::MyTiming::setTimer(_currentSampleTimerId, float(sample) / float(getSampleRate()));
    ma_sound_seek_to_pcm_frame(&_sound, sample);
    if (isPlaying())
        Vengine::MyTiming::startTimer(_currentSampleTimerId);
}

bool miniaudio_api::isPlaying()
{
    assert(_fileLoaded);

    return bool(ma_sound_is_playing(&_sound));
}

bool miniaudio_api::isFinished()
{
    assert(_fileLoaded);

    return bool(ma_sound_at_end(&_sound));
}

bool miniaudio_api::isLoaded()
{
    return _fileLoaded;
}

void miniaudio_api::setVolume(float volume)
{
    assert(_fileLoaded);

    ma_sound_set_volume(&_sound, volume);
}

int miniaudio_api::getCurrentSample() {

    assert(_fileLoaded);

    //fuck off with your inconsistent current sample reading
    /*
    ma_uint64 sample = ma_sound_get_time_in_pcm_frames(&_sound);
    if (sample > INT32_MAX) {
        Vengine::warning("Audio played for longer than max audio allowed (2,147,483,647 samples)");
        return -1;
    }

    return int(sample);
    */
    return min(int(Vengine::MyTiming::readTimer(_currentSampleTimerId) * getSampleRate()), getAudioDataLength());
}

float* miniaudio_api::getAudioData()
{
    assert(_fileLoaded);

    return _loadedAudioData;
}

int miniaudio_api::getAudioDataLength()
{
    assert(_fileLoaded);

    return _loadedAudioDataLength;
}

int miniaudio_api::getSampleRate() {

    assert(_fileLoaded);
    //std::cout << _decoder.outputSampleRate << std::endl;
    return _decoder.outputSampleRate;
}

void my_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_device_handle_backend_data_callback(pDevice, pOutput, pInput, frameCount);
}

//private-----------

bool miniaudio_api::initEngine()
{
    ma_result result = ma_engine_init(NULL, &_engine);
    if (result != MA_SUCCESS) {
        Vengine::warning("Failed to initialize audio engine.");
        return false;
    }

    return true;
}

bool miniaudio_api::initDecoderFromFile()
{
    _decoderConfig = ma_decoder_config_init(ma_format_f32, 1, 0);
    std::cout << "initing" << std::endl;

    ma_result result = ma_decoder_init_file(_filePath.c_str(), &_decoderConfig, &_decoder);
    if (result != MA_SUCCESS) {
        Vengine::warning("Failed to initialize decoder for file '" + _filePath + "'");
        return false;
    }

    return true;
}

bool miniaudio_api::initSoundFromFile()
{
    assert(_initialised);

    ma_result result = ma_sound_init_from_file(&_engine, _filePath.c_str(), MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &_sound);
    
    if (result != MA_SUCCESS) {
        Vengine::warning("Failed to load sound '" + _filePath + "'");
        return false;
    }
    return true;
}


//loopback recording (completely separate) ---

History<float> _loopbackAudioData(Vis::consts._requiredLoopbackCacheLength + Vis::consts._loopbackCacheSafetyBuffer);
int _samplesAddedSinceGetLoopbackDataCall= 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    const float* f32pInput = (const float*)pInput;
    _samplesAddedSinceGetLoopbackDataCall += frameCount;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        // Access the left and right channels.
        float left = f32pInput[i * pDevice->capture.channels];
        float right = f32pInput[i * pDevice->capture.channels + 1];

        float avg = (left + right) * 0.5f;

        _loopbackAudioData.add(avg);
    }

    (void)pOutput;
}

bool loopbackStarted = false;
ma_device loopbackDevice;

bool miniaudio_api::startLoopback(int& internalSampleRate, std::string& loopbackDeviceName)
{
    ma_result result;
    ma_device_config deviceConfig;

    internalSampleRate = -1; //default to -1

    /* Loopback mode is currently only supported on WASAPI. */
    ma_backend backends[] = {
        ma_backend_wasapi
    };

    deviceConfig = ma_device_config_init(ma_device_type_loopback);
    deviceConfig.capture.pDeviceID = NULL; //use default device
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.capture.channels = 2;
    deviceConfig.sampleRate = NULL; //use default device sample rate
    deviceConfig.dataCallback = data_callback;

    result = ma_device_init_ex(backends, sizeof(backends) / sizeof(backends[0]), NULL, &deviceConfig, &loopbackDevice);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize loopback device.\n");
        return false;
    }

    result = ma_device_start(&loopbackDevice);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&loopbackDevice);
        printf("Failed to start device.\n");
        return false;
    }

    internalSampleRate = loopbackDevice.capture.internalSampleRate;
    loopbackDeviceName = loopbackDevice.capture.name;

    loopbackStarted = true;

    return true;
}

void miniaudio_api::stopLoopback()
{
    if (loopbackStarted) {
        ma_device_uninit(&loopbackDevice);
    }
    loopbackStarted = false;
}

History<float>* miniaudio_api::getLoopbackDataHistoryPtr(int& numNewSamples)
{
    numNewSamples = _samplesAddedSinceGetLoopbackDataCall;
    _samplesAddedSinceGetLoopbackDataCall = 0;
    return &_loopbackAudioData;
}

//---