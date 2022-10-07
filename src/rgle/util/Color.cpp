#include "rgle/util/Color.h"

rgle::util::RGBA rgle::util::RGBA::blend(std::initializer_list<glm::vec4> colors)
{
	glm::vec4 result = glm::vec4(0.0f);
	for (const glm::vec4 color : colors) {
		result += color;
	}
	return RGBA(glm::vec4(result.r / result.a, result.g / result.a, result.b / result.a, result.a / colors.size()));
}

rgle::util::HSV rgle::util::HSV::blend(std::initializer_list<glm::vec3> colors)
{
	glm::vec3 result = glm::vec3(0.0f);
	for (const glm::vec3 color : colors) {
		result += color;
	}
	return HSV(glm::vec3(result.x / colors.size(), result.y / colors.size(), result.z / colors.size()));
}

rgle::util::Fill::Fill()
{
}

rgle::util::Fill::Fill(glm::vec4 solid)
{
	this->_solid = solid;
}

rgle::util::Fill::~Fill()
{
}

glm::vec4 rgle::util::Fill::evaluate(float, float)
{
	switch (this->_type)
	{
	case SOLID:
		return _solid;
		break;
	case LINEAR:
		return _solid;
		break;
	case RADIAL:
		return _solid;
		break;
	default:
		return glm::vec4(0.0f);
		break;
	}
}
