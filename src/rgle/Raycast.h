#pragma once

#include "rgle/Graphics.h"

namespace rgle {

	// Calculate barycentric coordinates using Cramer's rule to solve the system: p = a1p1 + a2p2 + a3p3, a1 + a2 + a3 = 1
	glm::vec3 barycentric(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& point);

	class Ray {
	public:
		Ray();
		Ray(glm::vec3 u, glm::vec3 p);
		virtual ~Ray();

		bool intersect(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
		bool intersect(Geometry3D::Face& face);
		bool intersect(Geometry3D* geometry);

	private:
		glm::vec3 _u;
		glm::vec3 _p;
	};

	class Raycastable {
	public:
		Raycastable();
		virtual ~Raycastable();

		virtual bool raycast(Ray ray);
	};
}