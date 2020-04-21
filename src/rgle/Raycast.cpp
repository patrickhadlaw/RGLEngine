#include "rgle/Raycast.h"


rgle::Ray::Ray()
{
}

rgle::Ray::Ray(glm::vec3 u, glm::vec3 p)
{
	this->_u = u;
	this->_p = p;
}

rgle::Ray::~Ray()
{
}

bool rgle::Ray::intersect(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const
{
	glm::vec3 n = glm::cross(p2 - p1, p3 - p2);
	float denom = glm::dot(n, this->_p);
	float r;
	if (denom != 0.0f) {
		r = glm::dot(n, p1 - this->_p) / denom;
	}
	else {
		r = 0.0f;
	}
	glm::vec3 point = this->_p + r * this->_u;
	glm::vec3 alpha = barycentric(p1, p2, p3, point);
	return alpha.x <= 1.0 && alpha.x >= 0.0 && alpha.y <= 1.0 && alpha.y >= 0.0 && alpha.z <= 1.0 && alpha.z >= 0.0;
}

bool rgle::Ray::intersect(const Geometry3D * geometry) const
{
	if (geometry == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	for (int i = 0; i < geometry->triangleCount(); i++) {
		if (this->intersect(
			geometry->triangleVertex(i, Geometry3D::TrianglePoint::A),
			geometry->triangleVertex(i, Geometry3D::TrianglePoint::B),
			geometry->triangleVertex(i, Geometry3D::TrianglePoint::C)))
		{
			return true;
		}
	}
	return false;
}

glm::vec3 rgle::barycentric(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point)
{
	glm::vec3 v0 = point - p3;
	glm::vec3 v1 = p1 - p3;
	glm::vec3 v2 = p2 - p3;
	glm::vec3 alpha;
	float denom = v1.x * v2.y - v2.x * v0.y;
	alpha.x = (v0.x * v2.y - v2.x * v0.y) / denom;
	alpha.y = (v1.x * v0.y - v0.x * v1.y) / denom;
	alpha.z = 1 - alpha.x - alpha.y;

	return alpha;
}

rgle::Raycastable::Raycastable()
{
}

rgle::Raycastable::~Raycastable()
{
}

bool rgle::Raycastable::raycast(Ray ray)
{
	return false;
}
