#include "rgle.h"

int main() {
	return rgle::util::Tester::run([](rgle::util::Tester& tester) {
    tester.expect("point clip should be inside", []() {
      auto point = glm::vec3(1.0f, 1.0f, 1.0f);
      auto plane = rgle::raycast::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			return plane.clip(point) == rgle::raycast::ClipResult(rgle::raycast::Inside { .point = glm::vec3(1.0f, 1.0f, 1.0f) });
		});

    tester.expect("point clip should be outside", []() {
      auto point = glm::vec3(1.0f, 1.0f, -1.0f);
      auto plane = rgle::raycast::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			return plane.clip(point) == rgle::raycast::ClipResult(rgle::raycast::Outside { .point = glm::vec3(1.0f, 1.0f, 0.0f) });
		});

    tester.expect("ray transform should be correct", []() {
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(),
        .target = glm::vec3(0.0f, 0.0f, 5.0f)
      };
      auto transformed = ray.transform(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
      return transformed.eye == glm::vec3(1.0f, 1.0f, 1.0f) && transformed.target == glm::vec3(1.0f, 1.0f, 6.0f);
    });

    tester.expect("plane intersect should be a miss", []() {
      auto plane = rgle::raycast::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(0.0f, 0.0f, 1.0f),
        .target = glm::vec3(0.0f, 0.0f, 2.0f)
      };
      return plane.intersect(ray) == rgle::raycast::IntersectResult(rgle::raycast::Miss {});
    });

    tester.expect("plane intersect should be a hit", []() {
      auto plane = rgle::raycast::Plane(glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(0.0f, 0.0f, -1.0f),
        .target = glm::vec3(0.0f, 0.0f, 0.0f)
      };
      return plane.intersect(ray) == rgle::raycast::IntersectResult(rgle::raycast::HitOnce {
        .hit = rgle::raycast::Intersection {
          .position = glm::vec3(),
          .normal = glm::vec3(0.0f, 0.0f, -1.0f)
        }
      });
    });
    
    tester.expect("ball intersect should be a double hit", []() {
      auto ball = rgle::raycast::Ball(5.0f);
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(0.0f, 0.0f, -10.0f),
        .target = glm::vec3(0.0f, 0.0f, 0.0f)
      };
      auto result = rgle::raycast::IntersectResult(rgle::raycast::HitTwice {
        .closer = rgle::raycast::Intersection {
          .position = glm::vec3(0.0f, 0.0f, -5.0f),
          .normal = glm::vec3(0.0f, 0.0f, -1.0f)
        },
        .farther = rgle::raycast::Intersection {
          .position = glm::vec3(0.0f, 0.0f, 5.0f),
          .normal = glm::vec3(0.0f, 0.0f, 1.0f)
        }
      });
      return ball.intersect(ray) == result;
    });

    tester.expect("ball intersect should be a single hit", []() {
      auto ball = rgle::raycast::Ball(5.0f);
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(5.0f, 0.0f, -10.0f),
        .target = glm::vec3(5.0f, 0.0f, 0.0f)
      };
      auto result = rgle::raycast::IntersectResult(rgle::raycast::HitOnce {
        .hit = rgle::raycast::Intersection {
          .position = glm::vec3(5.0f, 0.0f, 0.0f),
          .normal = glm::vec3(1.0f, 0.0f, 0.0f)
        }
      });
      return ball.intersect(ray) == result;
    });

    tester.expect("model intersect should be a hit", []() {
      auto model = rgle::raycast::Model(rgle::raycast::Transform {
        .transform = rgle::raycast::RayTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f))),
        .model = std::make_unique<rgle::raycast::Model>(rgle::raycast::Model(rgle::raycast::And {
          .lhs = rgle::raycast::transform(rgle::raycast::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f))),
          .rhs = rgle::raycast::transform(rgle::raycast::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))),
        }))
      });
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(-5.0f, 0.0f, 5.0f),
        .target = glm::vec3(-4.0f, 0.0f, 5.0f)
      };
      return model.intersect(ray) == std::make_optional(rgle::raycast::Intersection {
        .position = glm::vec3(-2.0f, 0.0f, 5.0f),
        .normal = glm::vec3(-1.0f, 0.0f, 0.0f)
      });
    });

    tester.expect("model intersect should be a miss", []() {
      auto model = rgle::raycast::Model(rgle::raycast::Transform {
        .transform = rgle::raycast::RayTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f))),
        .model = std::make_unique<rgle::raycast::Model>(rgle::raycast::Model(rgle::raycast::And {
          .lhs = rgle::raycast::transform(rgle::raycast::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f))),
          .rhs = rgle::raycast::transform(rgle::raycast::Ball(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))),
        }))
      });
      auto ray = rgle::raycast::Ray {
        .eye = glm::vec3(-2.0f, 0.0f, 0.0f),
        .target = glm::vec3(-2.0f, 0.0f, 1.0f)
      };
      return model.intersect(ray) == std::nullopt;
    });
  });
}
