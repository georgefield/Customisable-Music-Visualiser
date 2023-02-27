#include "SignalProcessing.h"
#include <Vengine/DrawFunctions.h>

void SignalProcessing::updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding) {

	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->firstPartPtr(), 0, history->firstPartSize() * sizeof(float));
	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->secondPartPtr(), history->firstPartSize() * sizeof(float), history->secondPartSize() * sizeof(float));
}

void SignalProcessing::updateSSBOwithVector(std::vector<float> vector, GLuint id, GLint binding)
{
	Vengine::DrawFunctions::updateSSBO(id, binding, &(vector[0]), vector.size() * sizeof(float));
}
