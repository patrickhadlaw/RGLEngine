#pragma once

#include "Window.h"

namespace cppogl {

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
		sImage image;
		sTexture texture;
		unsigned size;
		struct {
			unsigned width;
			unsigned height;
		} border;
		int lineSpacing;
	};

	typedef std::shared_ptr<Glyph> sGlyph;
	typedef std::vector<sGlyph> sGlyphv;
	
	class Font
	{
		friend class Text;
	public:
		Font();
		Font(sWindow window, std::string fontfile);
		virtual ~Font();

		void generate(int size);

		int lineHeight(int size);

	private:
		sWindow _window;
		FT_Library _library;
		FT_Face _face;
		std::map< unsigned, std::map<char, sGlyph> > _generated;
	};

	typedef std::shared_ptr<Font> sFont;

	class FontFamily {
	public:
		static const std::string REGULAR;
		static const std::string BOLD;
		static const std::string ITALIC;
		static const std::string ITALIC_BOLD;
		static const std::string LIGHT;
		static const std::string ITALIC_LIGHT;


		FontFamily();
		FontFamily(std::string family, std::vector<std::pair<std::string, sFont>> fonts);
		virtual ~FontFamily();

		sFont get(const std::string& fontface);

	private:
		std::string _family;
		std::map<std::string, sFont> _fonts;
	};

	typedef std::shared_ptr<FontFamily> sFontFamily;

	struct TextAttributes {
		std::string face = FontFamily::REGULAR;
		UnitValue fontSize = UnitValue{ 16.0f, Unit::PT };
		float width = 0.0f;
		float height = 0.0f;
		UnitVector3D position = UnitVector3D{ 0.0f, 0.0f, 0.0f, Unit::ND };
		int tabstop = 4;
		float lineSpacing = 2.0f;
		bool underlined = false;
		bool wrapWord = true;
	};

	class CharRect : public Shape {
	public:
		CharRect();
		CharRect(sWindow window, sShaderProgram shader, sGlyph glyph, glm::vec3 offset, float baselineOffset, Unit unit = Unit::PT);
		CharRect(const CharRect& other);
		CharRect(CharRect&& rvalue);
		virtual ~CharRect();

		void operator=(const CharRect& other);
		void operator=(CharRect&& rvalue);

		void recalculate();

		void render();

		static int counter;

		float width;
		float height;
		glm::vec3 offset;
		float baselineOffset;
		Unit unit;
		sGlyph glyph;
		sWindow window;
	};

	class Text : public EventListener {
	public:
		Text();
		Text(sWindow window, sShaderProgram shader, sFontFamily fontfamily, std::string text, TextAttributes attributes = {});
		virtual ~Text();

		void onMessage(std::string eventname, EventMessage* message);

		void generate(std::string text, TextAttributes attributes = {});

		void update(std::string text);

		void append(std::string text, TextAttributes attributes);
		void append(std::string text);

		void render();

	protected:

		void _getOffsetWrapWidth(glm::vec2& offset, sGlyph& glyph);
		void _getOffsetWrapWord(glm::vec2& offset, sGlyph& glyph, int& wordIndex, int& index, bool& firstWord);

		struct {
			GLint location;
			glm::mat4 matrix;
		} _model;
		std::vector<CharRect> _charecters;
		std::string _text;
		int _maxSize;
		glm::vec3 _topLeft;
		TextAttributes _attributes;
		sShaderProgram _shader;
		sFontFamily _fontFamily;
		sWindow _window;
	};
}