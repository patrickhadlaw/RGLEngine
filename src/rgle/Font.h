#pragma once

#include "rgle/Graphics.h"
#include "rgle/Interface.h"

namespace rgle {

	template<unsigned N>
	inline unsigned next_power_n(unsigned x) {
		unsigned value = 1;
		while (value < x) {
			value = value * N;
		}
		return value;
	}

	struct Glyph {
		Glyph();
		Glyph(char charecter, FT_Glyph& glyph, FT_Glyph_Metrics& metrics);
		~Glyph();
		char charecter;
		FT_Glyph glyph;
		FT_Glyph_Metrics metrics;
		FT_BBox bbox;
		std::shared_ptr<Image> image;
		std::shared_ptr<Texture> texture;
		unsigned size;
		struct {
			unsigned width;
			unsigned height;
		} border;
		int lineSpacing;
	};
	
	class Font : public Resource {
		friend class Text;
	public:
		Font();
		Font(std::shared_ptr<Window> window, std::string fontfile);
		virtual ~Font();

		void generate(int size);

		float lineHeight(int size);

		virtual const char* typeName() const;

	private:
		std::shared_ptr<Window> _window;
		FT_Library _library;
		FT_Face _face;
		std::map<unsigned, std::map<char, std::shared_ptr<Glyph>>> _generated;
	};

	namespace FontType {
		const std::string REGULAR = std::string("regular");
		const std::string BOLD = std::string("bold");
		const std::string ITALIC = std::string("italic");
		const std::string ITALIC_BOLD = std::string("italic_bold");
		const std::string LIGHT = std::string("light");
		const std::string ITALIC_LIGHT = std::string("italic_light");
	}

	class FontFamily : public Resource {
	public:
		FontFamily();
		FontFamily(std::string family, std::vector<std::pair<std::string, std::shared_ptr<Font>>> fonts);
		virtual ~FontFamily();

		std::shared_ptr<Font> get(const std::string& fontface);

		virtual const char* typeName() const;

	private:
		std::map<std::string, std::shared_ptr<Font>> _fonts;
	};

	struct TextAttributes {
		std::string face = FontType::REGULAR;
		UnitValue fontSize = UnitValue{ 16.0f, Unit::PT };
		UnitVector2D dimensions = UnitVector2D(0.0f, 0.0f, Unit::ND);
		UnitVector2D topLeft = UnitVector2D(0.0f, 0.0f, Unit::ND);
		float zIndex = 0.0f;
		int tabstop = 4;
		float lineSpacing = 2.0f;
		bool underlined = false;
		bool wrapWord = true;
	};

	class CharRect : public Shape {
	public:
		CharRect();
		CharRect(
			std::shared_ptr<ShaderProgram> shader,
			std::shared_ptr<Glyph> glyph,
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
		std::shared_ptr<Glyph> glyph;
	};

	class Text : public UI::Element {
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
		void _getOffsetWrapWidth(UnitVector2D& offset, std::shared_ptr<Glyph>& glyph);
		void _getOffsetWrapWord(UnitVector2D& offset, std::shared_ptr<Glyph>& glyph, int& wordIndex, int& index, bool& firstWord);

		struct {
			GLint location;
			glm::mat4 matrix;
		} _model;
		std::vector<CharRect> _characters;
		std::string _text;
		float _pixelSize;
		float _maxSize;
		TextAttributes _attributes;
		std::shared_ptr<FontFamily> _fontFamily;
	};
}