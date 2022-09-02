#pragma once

#include "rgle/gfx/Graphics.h"
#include "rgle/res/Font.h"

namespace rgle::gfx {
  class CharRect : public Shape {
	public:
		CharRect();
		CharRect(
			std::shared_ptr<ShaderProgram> shader,
			std::shared_ptr<res::Glyph> glyph,
			UnitVector2D offset,
			float zIndex,
			UnitValue baselineOffset,
			UnitVector2D dimensions
		);
		CharRect(const CharRect& other);
		CharRect(CharRect&& rvalue);
		virtual ~CharRect();

		void operator=(const CharRect& other);
		void operator=(CharRect&& rvalue);

		void recalculate();

		void render();

		float width;
		float height;
		float zIndex;
		UnitVector2D offset;
		UnitValue baselineOffset;
		UnitVector2D dimensions;
		std::shared_ptr<res::Glyph> glyph;
	};
}
