#pragma once

#include "rgle/Graphics.h"
#include "rgle/Math.h"

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

	namespace raycast {
		class Ray {
		public:
			Ray transform(const glm::mat4& matrix) const;
			glm::vec3 delta() const;

			glm::vec3 eye;
			glm::vec3 target;
		};

		class Intersection {
		public:
			Intersection transform(const glm::mat4& positionTransform, const glm::mat3& normalTransform) const;
			float distance(const Ray& ray) const;

			glm::vec3 position;
			glm::vec3 normal;
		};

		struct Miss {};

		struct HitOnce {
			Intersection hit;
		};

		struct HitTwice {
			Intersection closer;
			Intersection farther;
		};

		class IntersectResult : public std::variant<Miss, HitOnce, HitTwice> {
		public:
			using std::variant<Miss, HitOnce, HitTwice>::variant;

			std::optional<Intersection> closest() const;
			std::optional<Intersection> farthest() const;
		};

		class Intersect {
		public:
			virtual ~Intersect() = 0;

			virtual IntersectResult intersect(const Ray& ray) const = 0;
		};

		struct Inside {
			auto operator<=>(const Inside&) const = default;
			glm::vec3 point;
		};

		struct Outside {
			auto operator<=>(const Outside&) const = default;
			glm::vec3 point;
		};

		using ClipResult = std::variant<Inside, Outside>;

		class Plane : public Intersect {
		public:
			Plane(glm::vec3 position, glm::vec3 normal);
			virtual ~Plane();

			virtual IntersectResult intersect(const Ray& ray) const override;

			ClipResult clip(const glm::vec3& point) const;

			glm::vec3 position;
			glm::vec3 normal;
		};

		class Ball : public Intersect {
		public:
			Ball(float radius);
			virtual ~Ball();

			virtual IntersectResult intersect(const Ray& ray) const override;

			float radius;
		};

		class RayTransform {
		public:
			RayTransform();
			RayTransform(const glm::mat4& transform);

			Ray applyForward(const Ray& ray) const;
			Intersection applyBackward(const Intersection& intersection) const;

			glm::mat4 affine;
			glm::mat4 affineInverse;
			glm::mat3 normal;
		};

		class Model;

		struct Object {
			std::unique_ptr<Intersect> object;
		};

		struct Scene {
			std::vector<Model> scene;
		};

		struct Transform {
			RayTransform transform;
			std::unique_ptr<Model> model;
		};

		struct Clip {
			Plane clipPlane;
			std::unique_ptr<Model> model;
		};

		struct And {
			std::unique_ptr<Model> lhs;
			std::unique_ptr<Model> rhs;
		};

		struct Or {
			std::unique_ptr<Model> lhs;
			std::unique_ptr<Model> rhs;
		};

		class Model : public std::variant<Object, Scene, Transform, Clip, And, Or> {
		public:
			using base_type = std::variant<Object, Scene, Transform, Clip, And, Or>;
			using base_type::variant;

			std::optional<Intersection> intersect(const Ray& ray) const;
		};
	}
}