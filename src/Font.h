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
		Glyph(FT_Glyph& glyph);
		~Glyph();
		char c;
		FT_Glyph glyph;
		sImage image;
		unsigned size;
		struct {
			unsigned width;
			unsigned height;
		} border;
	};

	typedef std::shared_ptr<Glyph> sGlyph;
	typedef std::vector<sGlyph> sGlyphv;

	class Grid {
	public:


	private:

	};
	
	class Font
	{
		friend class Text;
	public:
		Font();
		Font(sWindow window, std::string fontfile);
		virtual ~Font();

		void generate(int size);

	private:
		sWindow _window;
		FT_Library _library;
		FT_Face _face;
		std::map< unsigned, std::map<char, sGlyph> > _generated;
	};

	typedef std::shared_ptr<Font> sFont;

	class CharRect : public Shape {
	public:
		CharRect();
		CharRect(sWindow window, sShaderProgram shader, sGlyph glyph, glm::vec3 offset = glm::vec3(0.0, 0.0, 0.0));
		CharRect(const CharRect& other);
		CharRect(CharRect&& rvalue);
		virtual ~CharRect();

		void operator=(CharRect&& rvalue);

		void recalculate();

		void render();

		float width;
		float height;
		glm::vec3 offset;
		sGlyph glyph;
		sWindow window;
	};

	class Text {
	public:
		Text();
		Text(sWindow window, sShaderProgram shader, sFont font, std::string text, int fontsize, glm::vec3 position = glm::vec3(0.0, 0.0, 0.0));
		virtual ~Text();

		void generate(std::string text, int fontsize);

		void update(std::string text);

		void render();

	protected:
		struct {
			GLint location;
			glm::mat4 matrix;
		} _model;
		std::vector<CharRect> _charecters;
		std::string _text;
		int _fontSize;
		sShaderProgram _shader;
		sFont _font;
		sWindow _window;
	};
}