#include "miniaudio_api.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <cassert>
#include <Vengine/MyErrors.h>

ma_engine _engine;

ma_sound _sound;

ma_decoder_config _decoderConfig;
ma_decoder _decoder;

miniaudio_api::miniaudio_api() :
    _initialised(false),
    _fileLoaded(false),
    _memoryAllocated(false),
    _filePath(""),
    _normalisedAudioData(nullptr)
{
}

miniaudio_api::~miniaudio_api()
{
    ma_engine_uninit(&_engine);
}

void miniaudio_api::init(){
    _initialised = initEngine();
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
    _audioDataLength = int(numPCMframes); //we know it can be safely truncated, int32 max gives us ~6 hours audio max length

    _normalisedAudioData = new float[_audioDataLength];
    _memoryAllocated = true;

    ma_uint64 framesRead;
    ma_result resultGetData = ma_decoder_read_pcm_frames(&_decoder, _normalisedAudioData, _audioDataLength, &framesRead);
    if (resultGetData != MA_SUCCESS) {
        Vengine::warning("Could not get audio data");
        return false;
    }

    if (framesRead != _audioDataLength) {
        Vengine::warning("frames read =/= num frames, possible problem, file still loaded");
    }

    _fileLoaded = true;
    return true;
}

void miniaudio_api::unloadAudio()
{
    _fileLoaded = false;
    _filePath = "";

    if (_decoderInitialised) {
        ma_result decResult = ma_decoder_uninit(&_decoder);
        if (decResult != MA_SUCCESS) {
            Vengine::warning("Error unloading decoder");
        }
        _decoderInitialised = false;
    }

    if (_soundInitialised) {
        ma_sound_uninit(&_sound);
        _soundInitialised = false;
    }

    if (_memoryAllocated) {
        delete[] _normalisedAudioData;
        _memoryAllocated = false;
    }
}

void miniaudio_api::playAudio()
{
    assert(_fileLoaded);

    ma_sound_start(&_sound);   
}

void miniaudio_api::pauseAudio()
{
    assert(_fileLoaded);

    ma_sound_stop(&_sound);
}

void miniaudio_api::seekToSample(int sample)
{
    assert(_fileLoaded);
    assert(sample >= 0);
    assert(sample < _audioDataLength);

    ma_sound_seek_to_pcm_frame(&_sound, sample);
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

    ma_uint64 sample = ma_sound_get_time_in_pcm_frames(&_sound);
    if (sample > INT32_MAX) {
        Vengine::warning("Audio played for longer than max audio allowed (2,147,483,647 samples)");
        return -1;
    }

    return int(sample);
}

float* miniaudio_api::getAudioData()
{
    assert(_fileLoaded);

    return _normalisedAudioData;
}

int miniaudio_api::getAudioDataLength()
{
    assert(_fileLoaded);

    return _audioDataLength;
}

int miniaudio_api::getSampleRate() {

    assert(_fileLoaded);

    return _decoder.outputSampleRate;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_device_handle_backend_data_callback(pDevice, pOutput, pInput, frameCount);
}

void miniaudio_api::getDevices()
{
    ma_context context;
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        // Error.
        printf("ERRRR");
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        // Error.
        printf("EROOR");
    }

    // Loop over each device info and do something with it. Here we just print the name with their index. You may want
    // to give the user the opportunity to choose which device they'd prefer.
    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1) {
        printf("playback: %d - %s\n", iDevice, pPlaybackInfos[iDevice].name);
    }

    for (ma_uint32 iDevice = 0; iDevice < captureCount; iDevice += 1) {
        printf("capture: %d - %s\n", iDevice, pCaptureInfos[iDevice].name);
    }

    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.pDeviceID = &pPlaybackInfos[0].id;
    config.capture.channelMixMode = ma_channel_mix_mode_default;
    config.capture.format = ma_format_f32;
    config.dataCallback = data_callback;
    config.sampleRate = 44100;
    config.pUserData = NULL;

    ma_device device;
    if (ma_device_init(&context, &config, &device) != MA_SUCCESS) {
        // Error
        printf("EOROR");
    }

    ma_device_start(&device);

    
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
