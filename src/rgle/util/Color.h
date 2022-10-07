#pragma once

#include "rgle/gfx/Renderable.h"

namespace rgle::util {
	class RGB {
	public:
		RGB(float r, float g, float b) : r(r), g(g), b(b) {}
		RGB(glm::vec3 rgb) : vector(rgb) {}
		union {
			struct {
				float r;
				float g;
				float b;
			};
			glm::vec3 vector;
		};
	};
	class RGBA {
	public:
		RGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		RGBA(glm::vec4 rgba) : vector(rgba) {}
		static RGBA blend(std::initializer_list<glm::vec4> colors);
		union {
			struct {
				float r;
				float g;
				float b;
				float a;
			};
			glm::vec4 vector; 
		};
	};
	struct HSV {
		HSV(float h, float s, float v) : h(h), s(s), v(v) {}
		HSV(glm::vec3 hsv) : vector(hsv) {}
		static HSV blend(std::initializer_list<glm::vec3> colors);
		union {
			struct {
				float h;
				float s;
				float v;
			};
			glm::vec3 vector;
		};
	};

	class Color : public std::variant<RGB, RGBA, HSV> {
	public:
		using std::variant<RGB, RGBA, HSV>::variant;

		template<typename T>
		T into() const;

		template<>
		RGB into() const {
			if (auto rgb = std::get_if<RGB>(this)) {
				return *rgb;
			}
			else if (auto rgba = std::get_if<RGBA>(this)) {
				return RGB(rgba->r, rgba->g, rgba->b);
			}
			else if (auto hsv = std::get_if<HSV>(this)) {
				if (hsv->s == 0.0f) {
					return RGB(hsv->v, hsv->v, hsv->v);
				}
				int i = static_cast<int>(hsv->h * 6.0f);
				float f = (hsv->h * 6.0f) - i;
				float p = hsv->v * (1.0f - hsv->s);
				float q = hsv->v * (1.0f - hsv->s * f);
				float t = hsv->v * (1.0f - hsv->s * (1.0f - f));
				i = i % 6;
				switch (i) {
				case 0:
					return RGB(hsv->v, t, p);
				case 1:
					return RGB(q, hsv->v, p);
				case 2:
					return RGB(p, hsv->v, t);
				case 3:
					return RGB(p, q, hsv->v);
				case 4:
					return RGB(t, p, hsv->v);
				case 5:
					return RGB(hsv->v, p, q);
				}
			}
			return RGB(0.0f, 0.0f, 0.0f);
		}

		template<>
		HSV into() const {
			if (auto hsv = std::get_if<HSV>(this)) {
				return *hsv;
			}
			else {
				RGB rgb = this->into<RGB>();
				HSV result = HSV(0.0, 0.0, 0.0);
				float min = std::min(rgb.r, std::min(rgb.g, rgb.b));
				float max = std::max(rgb.r, std::max(rgb.g, rgb.b));
				result.v = max;
				float delta = max - min;
				if (delta < 0.00001f) {
					return HSV(0.0f, 0.0f, max);
				}
				if (max > 0.0f) {
					result.s = (delta / max);
				}
				else {
					return HSV(NAN, 0.0f, max);
				}
				if (rgb.r >= max) {
					result.h = (rgb.g - rgb.b) / delta;
				}
				else if (rgb.g >= max) {
					result.h = 2.0f + (rgb.b - rgb.r) / delta;
				}
				else {
					result.h = 4.0f + (rgb.r - rgb.g) / delta;
				}
				result.h *= 60.0f;
				if (result.h < 0.0f) {
					result.h += 360.0;
				}
				return result;
			}
		}
	};

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