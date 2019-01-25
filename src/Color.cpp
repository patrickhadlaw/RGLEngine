#include "Color.h"


cppogl::Fill::Fill()
{
}

cppogl::Fill::Fill(glm::vec4 solid)
{
	this->_solid = solid;
}

cppogl::Fill::~Fill()
{
}

glm::vec4 cppogl::Fill::evaluate(float u, float v)
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
