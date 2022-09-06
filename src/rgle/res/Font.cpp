#include "rgle/res/Font.h"
#include "rgle/gfx/Graphics.h"


rgle::res::Font::Font()
{
	this->_window = nullptr;
}

rgle::res::Font::Font(std::shared_ptr<Window> window, std::string fontfile)
{
	RGLE_DEBUG_ONLY(Logger::debug(std::string("loading font file: ") + fontfile, LOGGER_DETAIL_DEFAULT);)
	this->_window = window;
	int error = FT_Init_FreeType(&_library);
	if (error) {
		throw Exception("failed to initialise FreeType library module, error code: " + std::to_string(error), LOGGER_DETAIL_DEFAULT);
	}
	error = FT_New_Face(_library, fontfile.data(), 0, &_face);
	if (error == FT_Err_Unknown_File_Format) {
		throw Exception("failed to load font, invalid file format", LOGGER_DETAIL_DEFAULT);
	}
	else if (error) {
		throw Exception("failed to load font: " + fontfile, LOGGER_DETAIL_DEFAULT);
	}
}

rgle::res::Font::~Font()
{
}

void rgle::res::Font::generate(int size)
{
	if (this->_window == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	FT_Set_Pixel_Sizes(_face, 0, size);
	std::map<char, std::shared_ptr<Glyph>> glyphs;
	for (unsigned char c = 1; c < 128; c++) {
		if (FT_Load_Glyph(_face, FT_Get_Char_Index(_face, c), FT_LOAD_DEFAULT)) {
			throw Exception("failed to load glyph", LOGGER_DETAIL_DEFAULT);
		}
		FT_Glyph ft_glyph;
		if (FT_Get_Glyph(_face->glyph, &ft_glyph)) {
			throw Exception("failed to get glyph", LOGGER_DETAIL_DEFAULT);
		}
		glyphs[c] = std::make_shared<Glyph>(Glyph(c, ft_glyph, _face->glyph->metrics));
	}
	this->_generated[size] = glyphs;
}

float rgle::res::Font::lineHeight(int size)
{
	FT_Set_Pixel_Sizes(_face, 0, size);
	return (float) _face->size->metrics.height / 64;
}

const char * rgle::res::Font::typeName() const
{
	return "rgle::res::Font";
}

rgle::res::Glyph::Glyph()
{
	this->image = nullptr;
}

rgle::res::Glyph::Glyph(char charecter, FT_Glyph& glyph, FT_Glyph_Metrics& metrics)
{
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	FT_Bitmap& bitmap = bitmap_glyph->bitmap;
	this->charecter = charecter;
	this->glyph = glyph;
	this->metrics = metrics;
	FT_Glyph_Get_CBox(this->glyph, FT_GLYPH_BBOX_UNSCALED, &this->bbox);
	this->border.width = bitmap.width;
	this->border.height = bitmap.rows;
	this->image = std::make_shared<gfx::Image>(gfx::Image());
	this->image->width = next_power_n<2>(this->border.width);
	this->image->height = next_power_n<2>(this->border.height);
	this->image->channels = 1;
	this->image->image = new unsigned char[this->image->width * this->image->channels * this->image->height];
	for (size_t i = 0; i < this->image->width; i++) {
		for (size_t j = 0; j < this->image->height; j++) {
			size_t index = this->image->channels * (i + j * this->image->width);
			if (static_cast<unsigned int>(i) < this->border.width && static_cast<unsigned int>(j) < this->border.height) {
				this->image->image[index] = bitmap.buffer[j * this->border.width + i];
			}
			else {
				this->image->image[index] = 0;
			}
		}
	}
	this->texture = std::make_shared<gfx::Texture2D>(this->image, GL_TEXTURE0, gfx::Texture2D::Format{ GL_R8, GL_RED });
}

rgle::res::Glyph::~Glyph()
{
}

rgle::res::FontFamily::FontFamily()
{
}

rgle::res::FontFamily::FontFamily(std::string family, std::vector<std::pair<std::string, std::shared_ptr<Font>>> fonts)
{
	id = family;
	for (size_t i = 0; i < fonts.size(); i++) {
		this->_fonts[fonts[i].first] = fonts[i].second;
	}
}

rgle::res::FontFamily::~FontFamily()
{
}

std::shared_ptr<rgle::res::Font> rgle::res::FontFamily::get(const std::string& fontface) {
	if (this->_fonts.find(fontface) == this->_fonts.end()) {
		if (fontface == FontType::REGULAR) {
			throw Exception(std::string("could not find default font face in family: ") + id, LOGGER_DETAIL_DEFAULT);
		}
		return this->get(FontType::REGULAR);
	}
	else {
		return this->_fonts[fontface];
	}
}

const char * rgle::res::FontFamily::typeName() const
{
	return "rgle::res::FontFamily";
}
