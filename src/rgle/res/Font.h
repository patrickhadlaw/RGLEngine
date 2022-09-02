#pragma once

#include "rgle/gfx/Graphics.h"
#include "rgle/ui/Interface.h"

namespace rgle::res {

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
		std::shared_ptr<gfx::Image> image;
		std::shared_ptr<gfx::Texture> texture;
		unsigned size;
		struct {
			unsigned width;
			unsigned height;
		} border;
		int lineSpacing;
	};
	
	class Font : public Resource {
		friend class ui::Text;
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
}