#pragma once

#include "rgle/Graphics.h"

namespace rgle {

	// Calculate barycentric coordinates using Cramer's rule to solve the system: p = a1p1 + a2p2 + a3p3, a1 + a2 + a3 = 1
	glm::vec3 barycentric(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point);

	class Ray {
	public:
		Ray();
		Ray(glm::vec3 u, glm::vec3 p);
		virtual ~Ray();

		bool intersect(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const;
		bool intersect(const Geometry3D* geometry) const;

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