#pragma once

#include "rgle/gfx/CharRect.h"

namespace rgle::ui {
  struct TextAttributes {
		std::string face = res::FontType::REGULAR;
		UnitValue fontSize = UnitValue{ 16.0f, Unit::PT };
		UnitVector2D dimensions = UnitVector2D(0.0f, 0.0f, Unit::ND);
		UnitVector2D topLeft = UnitVector2D(0.0f, 0.0f, Unit::ND);
		float zIndex = 0.0f;
		int tabstop = 4;
		float lineSpacing = 2.0f;
		bool underlined = false;
		bool wrapWord = true;
	};

  class Text : public ui::Element {
	public:
		Text(std::string shaderid, std::string fontFamily, std::string text, TextAttributes attributes = {});
		virtual ~Text();

		void onMessage(std::string eventname, EventMessage* message);

		void generate(std::string text, TextAttributes attributes = {});

		void update(std::string text);

		void append(std::string text, TextAttributes attributes);
		void append(std::string text);

		virtual void render();

		virtual void onBoxUpdate();

		virtual const char* typeName() const;

	protected:
		float _scale(float value);
		void _getOffsetWrapWidth(UnitVector2D& offset, std::shared_ptr<res::Glyph>& glyph);
		void _getOffsetWrapWord(UnitVector2D& offset, std::shared_ptr<res::Glyph>& glyph, size_t& wordIndex, size_t& index, bool& firstWord);

		struct {
			GLint location;
			glm::mat4 matrix;
		} _model;
		std::vector<gfx::CharRect> _characters;
		std::string _text;
		float _pixelSize;
		float _maxSize;
		TextAttributes _attributes;
		std::shared_ptr<res::FontFamily> _fontFamily;
	};
}