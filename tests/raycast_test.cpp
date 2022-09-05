#include "rgle.h"

int main() {
	return rgle::util::Tester::run([](rgle::util::Tester& tester) {
    tester.expect("point clip should be inside", []() {
      auto point = glm::vec3(1.0f, 1.0f, 1.0f);
      auto plane = rgle::ray::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			return plane.clip(point) == rgle::ray::ClipResult(rgle::ray::Inside { .point = glm::vec3(1.0f, 1.0f, 1.0f) });
		});

    tester.expect("point clip should be outside", []() {
      auto point = glm::vec3(1.0f, 1.0f, -1.0f);
      auto plane = rgle::ray::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			return plane.clip(point) == rgle::ray::ClipResult(rgle::ray::Outside { .point = glm::vec3(1.0f, 1.0f, 0.0f) });
		});

    tester.expect("ray transform should be correct", []() {
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(),
        .target = glm::vec3(0.0f, 0.0f, 5.0f)
      };
      auto transformed = ray.transform(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
      return transformed.eye == glm::vec3(1.0f, 1.0f, 1.0f) && transformed.target == glm::vec3(1.0f, 1.0f, 6.0f);
    });

    tester.expect("plane intersect should be a miss", []() {
      auto plane = rgle::ray::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(0.0f, 0.0f, 1.0f),
        .target = glm::vec3(0.0f, 0.0f, 2.0f)
      };
      return plane.intersect(ray) == rgle::ray::IntersectResult(rgle::ray::Miss {});
    });

    tester.expect("plane intersect should be a hit", []() {
      auto plane = rgle::ray::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(0.0f, 0.0f, -1.0f),
        .target = glm::vec3(0.0f, 0.0f, 0.0f)
      };
      return plane.intersect(ray) == rgle::ray::IntersectResult(rgle::ray::HitOnce {
        .hit = rgle::ray::Intersection {
          .position = glm::vec3(),
          .normal = glm::vec3(0.0f, 0.0f, -1.0f)
        }
      });
    });
    
    tester.expect("ball intersect should be a double hit", []() {
      auto ball = rgle::ray::Ball(5.0f);
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(0.0f, 0.0f, -10.0f),
        .target = glm::vec3(0.0f, 0.0f, 0.0f)
      };
      auto result = rgle::ray::IntersectResult(rgle::ray::HitTwice {
        .closer = rgle::ray::Intersection {
          .position = glm::vec3(0.0f, 0.0f, -5.0f),
          .normal = glm::vec3(0.0f, 0.0f, -1.0f)
        },
        .farther = rgle::ray::Intersection {
          .position = glm::vec3(0.0f, 0.0f, 5.0f),
          .normal = glm::vec3(0.0f, 0.0f, 1.0f)
        }
      });
      return ball.intersect(ray) == result;
    });

    tester.expect("ball intersect should be a single hit", []() {
      auto ball = rgle::ray::Ball(5.0f);
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(5.0f, 0.0f, -10.0f),
        .target = glm::vec3(5.0f, 0.0f, 0.0f)
      };
      auto result = rgle::ray::IntersectResult(rgle::ray::HitOnce {
        .hit = rgle::ray::Intersection {
          .position = glm::vec3(5.0f, 0.0f, 0.0f),
          .normal = glm::vec3(1.0f, 0.0f, 0.0f)
        }
      });
      return ball.intersect(ray) == result;
    });

    tester.expect("model intersect should be a hit", []() {
      auto model = rgle::ray::Model(rgle::ray::Transform {
        .transform = rgle::ray::RayTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f))),
        .model = std::make_unique<rgle::ray::Model>(rgle::ray::Model(rgle::ray::And {
          .lhs = rgle::ray::transform(rgle::ray::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f))),
          .rhs = rgle::ray::transform(rgle::ray::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))),
        }))
      });
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(-5.0f, 0.0f, 5.0f),
        .target = glm::vec3(-4.0f, 0.0f, 5.0f)
      };
      return model.intersect(ray) == std::make_optional(rgle::ray::Intersection {
        .position = glm::vec3(-2.0f, 0.0f, 5.0f),
        .normal = glm::vec3(-1.0f, 0.0f, 0.0f)
      });
    });

    tester.expect("model intersect should be a miss", []() {
      auto model = rgle::ray::Model(rgle::ray::Transform {
        .transform = rgle::ray::RayTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f))),
        .model = std::make_unique<rgle::ray::Model>(rgle::ray::Model(rgle::ray::And {
          .lhs = rgle::ray::transform(rgle::ray::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f))),
          .rhs = rgle::ray::transform(rgle::ray::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))),
        }))
      });
      auto ray = rgle::ray::Ray {
        .eye = glm::vec3(-2.0f, 0.0f, 0.0f),
        .target = glm::vec3(-2.0f, 0.0f, 1.0f)
      };
      return model.intersect(ray) == std::nullopt;
    });
  });
}
