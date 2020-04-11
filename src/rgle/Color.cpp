#include "rgle/Color.h"


rgle::Fill::Fill()
{
}

rgle::Fill::Fill(glm::vec4 solid)
{
	this->_solid = solid;
}

rgle::Fill::~Fill()
{
}

glm::vec4 rgle::Fill::evaluate(float u, float v)
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

glm::vec4 rgle::Color::blend(std::initializer_list<glm::vec4> colors)
{
	glm::vec4 result = glm::vec4(0.0f);
	for (const glm::vec4 color : colors) {
		result += color;
	}
	return glm::vec4(result.r / result.a, result.g / result.a, result.b / result.a, result.a / colors.size());
}
