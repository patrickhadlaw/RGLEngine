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
		break;
	}
}
