#pragma once

#include "rgle/Renderable.h"

namespace rgle {
	class Fill {
	public:
		struct RadialDescriptor {
			glm::vec3 point;
			glm::vec4 color;
			float radius = 0.0f;
		};
		struct LinearDescriptor {
			float percent = 0.0f;
			glm::vec4 color;
		};
		Fill();
		Fill(glm::vec4 solid);
		virtual ~Fill();

		enum Type {
			SOLID,
			LINEAR,
			RADIAL
		};

		static Fill& linearGradient(std::vector<LinearDescriptor> description);
		static Fill& radialGradient(std::vector<RadialDescriptor> description);

		glm::vec4 evaluate(float u, float v);

	protected:
		std::vector<LinearDescriptor> _linear;
		std::vector<RadialDescriptor> _radial;
		glm::vec4 _solid;
		Type _type = Type::SOLID;
	};
}