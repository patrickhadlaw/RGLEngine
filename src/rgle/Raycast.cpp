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

bool rgle::Ray::intersect(const gfx::Geometry3D * geometry) const
{
	if (geometry == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	for (int i = 0; i < geometry->triangleCount(); i++) {
		if (this->intersect(
			geometry->triangleVertex(i, gfx::Geometry3D::TrianglePoint::A),
			geometry->triangleVertex(i, gfx::Geometry3D::TrianglePoint::B),
			geometry->triangleVertex(i, gfx::Geometry3D::TrianglePoint::C)))
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

rgle::raycast::Ray rgle::raycast::Ray::transform(const glm::mat4& matrix) const
{
	return Ray(
		(matrix * glm::vec4(this->eye, 1.0f)).xyz,
		(matrix * glm::vec4(this->target, 1.0f)).xyz
	);
}

glm::vec3 rgle::raycast::Ray::delta() const
{
	return this->target - this->eye;
}

rgle::raycast::Intersection rgle::raycast::Intersection::transform(const glm::mat4& positionTransform, const glm::mat3& normalTransform) const {
	return Intersection {
		.position = (positionTransform * glm::vec4(this->position, 1.0f)).xyz,
		.normal = glm::normalize(normalTransform * this->normal)
	};
}

float rgle::raycast::Intersection::distance(const Ray& ray) const {
	return glm::distance(this->position, ray.eye);
}

std::optional<rgle::raycast::Intersection> rgle::raycast::IntersectResult::closest() const {
	if (auto val = std::get_if<HitOnce>(this)) {
		return val->hit;
	}
	else if (auto val = std::get_if<HitTwice>(this)) {
		return val->closer;
	}
	return std::nullopt;
}

std::optional<rgle::raycast::Intersection> rgle::raycast::IntersectResult::farthest() const {
	if (auto val = std::get_if<HitOnce>(this)) {
		return val->hit;
	}
	else if (auto val = std::get_if<HitTwice>(this)) {
		return val->farther;
	}
	return std::nullopt;
}

rgle::raycast::Intersect::~Intersect()
{
}

rgle::raycast::Plane::Plane(glm::vec3 position, glm::vec3 normal) : position(position), normal(normal)
{	
}

rgle::raycast::Plane::~Plane()
{
}

rgle::raycast::IntersectResult rgle::raycast::Plane::intersect(const Ray& ray) const {
	auto delta = ray.delta();
	float denom = glm::dot(delta, this->normal);
	if (denom == 0.0f) {
		return IntersectResult(Miss {});
	}
	else {
		float t = (glm::dot(this->position, this->normal) - glm::dot(ray.eye, this->normal)) / denom;
		if (t >= 0.0f) {
			auto position = ray.eye + t * delta;
			if (denom < 0.0f) {
				return IntersectResult(HitOnce {
					.hit = Intersection {
						.position = position,
						.normal = this->normal
					}
				});
			}
			else {
				return IntersectResult(HitOnce {
					.hit = Intersection {
						.position = position,
						.normal = -this->normal
					}
				});
			}
		}
		else {
			return IntersectResult(Miss {});
		}
	}
	return IntersectResult(Miss {});
}

rgle::raycast::ClipResult rgle::raycast::Plane::clip(const glm::vec3& point) const {
	auto v = point - this->position;
	float dist = glm::dot(v, this->normal);
	if (dist >= 0.0f) {
		return ClipResult(Inside {
			.point = point
		});
	}
	return ClipResult(Outside {
		.point = point - dist * this->normal
	});
}

rgle::raycast::Ball::Ball(float radius) : radius(radius) 
{
}

rgle::raycast::Ball::~Ball()
{
}

rgle::raycast::IntersectResult rgle::raycast::Ball::intersect(const Ray& ray) const
{
	auto delta = ray.delta();
	float a = glm::dot(delta, delta);
	float b = 2.0f * glm::dot(ray.eye, delta);
	float c = glm::dot(ray.eye, ray.eye) - this->radius * this->radius;
	auto roots = math::quadratic::compute_roots(a, b, c);
	if (auto val = std::get_if<math::quadratic::One>(&roots)) {
		auto p = ray.eye + val->x0 * delta;
		return IntersectResult(HitOnce {
			.hit = Intersection {
				.position = p,
				.normal = glm::normalize(p)
			}
		});
	}
	else if (auto val = std::get_if<math::quadratic::Two>(&roots)) {
		auto p1 = ray.eye + val->x0 * delta;
		auto p2 = ray.eye + val->x1 * delta;
		if (glm::distance(ray.eye, p1) > glm::distance(ray.eye, p2)) {
			std::swap(p1, p2);
		}
		return IntersectResult(HitTwice {
			.closer = Intersection {
				.position = p1,
				.normal = glm::normalize(p1)
			},
			.farther = Intersection {
				.position = p2,
				.normal = glm::normalize(p2)
			}
		});
	}
	return IntersectResult(Miss {});
}

rgle::raycast::RayTransform::RayTransform() : RayTransform(glm::mat4(1.0f))
{}

rgle::raycast::RayTransform::RayTransform(const glm::mat4& transform) : affine(transform), affineInverse(glm::inverse(transform))
{
	auto normal = glm::mat3(transform[0].xyz, transform[1].xyz, transform[2].xyz);
	this->normal = glm::transpose(glm::inverse(normal));
}

rgle::raycast::Ray rgle::raycast::RayTransform::applyForward(const Ray& ray) const {
	return Ray {
		.eye = (this->affineInverse * glm::vec4(ray.eye, 1.0f)).xyz,
		.target = (this->affineInverse * glm::vec4(ray.target, 1.0f)).xyz
	};
}

rgle::raycast::Intersection rgle::raycast::RayTransform::applyBackward(const Intersection& intersection) const {
	return Intersection {
		.position = (this->affine * glm::vec4(intersection.position, 1.0f)).xyz,
		.normal = glm::normalize(this->normal * intersection.normal)
	};
}

std::optional<rgle::raycast::Intersection> rgle::raycast::Model::intersect(const Ray& ray) const
{
	if (auto val = std::get_if<Object>(this)) {
		return val->object->intersect(ray).closest();
	}
	else if (auto val = std::get_if<Scene>(this)) {
		std::optional<Intersection> closest = std::nullopt;
		for (const auto& model : val->scene) {
			if (auto intersect = model.intersect(ray)) {
				if (closest.has_value()) {
					if (intersect->distance(ray) < closest->distance(ray)) {
						closest = std::move(intersect);
					}
				}
				else {
					closest = std::move(intersect);
				}
			}
		}
		return closest;
	}
	else if (auto val = std::get_if<Transform>(this)) {
		auto transformed = val->transform.applyForward(ray);
		if (auto intersect = val->model->intersect(transformed)) {
			return val->transform.applyBackward(*intersect);
		}
		return std::nullopt;
	}
	else if (auto val = std::get_if<Clip>(this)) {
		if (auto intersect = val->model->intersect(ray)) {
			auto clipped = val->clipPlane.clip(intersect->position);
			if (std::holds_alternative<Inside>(clipped)) {
				return intersect;
			}
		}
		return std::nullopt;
	}
	else if (auto val = std::get_if<And>(this)) {
		if (auto intersect1 = val->lhs->intersect(ray)) {
			if (auto intersect2 = val->rhs->intersect(ray)) {
				if (intersect1->distance(ray) < intersect2->distance(ray)) {
					return intersect1;
				}
				else {
					return intersect2;
				}
			}
		}
		return std::nullopt;
	}
	else if (auto val = std::get_if<Or>(this)) {
		auto intersect1 = val->lhs->intersect(ray);
		auto intersect2 = val->rhs->intersect(ray);
		if (intersect1 && intersect2) {
			if (intersect1->distance(ray) < intersect2->distance(ray)) {
				return intersect1;
			}
			else {
				return intersect2;
			}
		}
		else {
			if (intersect1) {
				return intersect1;
			}
			return intersect2;
		}
		return std::nullopt;
	}
	return std::nullopt;
}
