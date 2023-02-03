#include "SignalProcessing.h"
#include <Vengine/DrawFunctions.h>

void SignalProcessing::updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding) {

	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->firstPartPtr(), 0, history->firstPartSize() * sizeof(float));
	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->secondPartPtr(), history->firstPartSize() * sizeof(float), history->secondPartSize() * sizeof(float));
}