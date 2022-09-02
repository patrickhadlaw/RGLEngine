#include "rgle.h"

int main() {
	return rgle::Tester::run([](rgle::Tester& tester) {
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
  });
}
