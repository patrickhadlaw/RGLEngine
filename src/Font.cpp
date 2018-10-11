#include "Font.h"
#include "Graphics.h"



cppogl::Font::Font()
{
	this->_window = nullptr;
}

cppogl::Font::Font(sWindow window, std::string fontfile)
{
	this->_window = window;
	int error = FT_Init_FreeType(&_library);
	if (error) {
		throw Exception("failed to initialise FreeType library module, error code: " + std::to_string(error), EXCEPT_DETAIL_DEFAULT);
	}
	error = FT_New_Face(_library, fontfile.data(), 0, &_face);
	if (error == FT_Err_Unknown_File_Format) {
		throw Exception("failed to load font, invalid file format", EXCEPT_DETAIL_DEFAULT);
	}
	else if (error) {
		throw Exception("failed to load font: " + fontfile, EXCEPT_DETAIL_DEFAULT);
	}
}

cppogl::Font::~Font()
{
}

void cppogl::Font::generate(int size)
{
	if (this->_window == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	FT_Set_Pixel_Sizes(_face, 0, size);
	std::map<char, sGlyph> glyphs;
	for (unsigned char c = 1; c < 128; c++) {
		if (FT_Load_Glyph(_face, FT_Get_Char_Index(_face, c), FT_LOAD_DEFAULT)) {
			throw Exception("failed to load glyph", EXCEPT_DETAIL_DEFAULT);
		}
		FT_Glyph ft_glyph;
		if (FT_Get_Glyph(_face->glyph, &ft_glyph)) {
			throw Exception("failed to get glyph", EXCEPT_DETAIL_DEFAULT);
		}
		glyphs[c] = std::make_shared<Glyph>(Glyph(c, ft_glyph, _face->glyph->metrics));
	}
	this->_generated[size] = glyphs;
}

int cppogl::Font::lineHeight(int size)
{
	if (this->_window == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	FT_Set_Pixel_Sizes(_face, 0, size);
	return _face->height / 64;
}

cppogl::CharRect::CharRect()
{
	this->shader = nullptr;
	this->window = nullptr;
}

int cppogl::CharRect::counter = 0;

cppogl::CharRect::CharRect(sWindow window, sShaderProgram shader, sGlyph glyph, glm::vec3 offset, float baselineOffset, Unit unit)
{
	this->shader = shader;
	this->window = window;
	this->offset = offset;
	this->baselineOffset = baselineOffset;
	this->unit = unit;
	this->color.location = glGetAttribLocation(shader->id(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", EXCEPT_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->id(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", EXCEPT_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(shader->id(), "texture_coords");
	if (this->uv.location < 0) {
		throw Exception("failed to locate shader attribute: texture coordinates", EXCEPT_DETAIL_DEFAULT);
	}
	float width = window->parse(UnitValue{ (float)glyph->border.width, unit }, Window::X);
	float height = window->parse(UnitValue{ (float)glyph->border.height, unit }, Window::Y);
	glm::vec3 scaled = glm::vec3(
		window->parse(UnitValue{ offset.x, unit }, Window::X),
		window->parse(UnitValue{ offset.y + baselineOffset, unit }, Window::Y),
		offset.z
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(width, 0.0, 0.0) + scaled,
		glm::vec3(0.0, height, 0.0) + scaled,
		glm::vec3(width, height, 0.0) + scaled
	};
	float uvWidth = (float)glyph->border.width / glyph->image->width;
	float uvHeight = (float)glyph->border.height / glyph->image->height;
	this->uv.list = {
		glm::vec2(0.0, uvHeight),
		glm::vec2(uvWidth, uvHeight),
		glm::vec2(0.0, 0.0),
		glm::vec2(uvWidth, 0.0)
	};
	this->index.list = {
		0,
		1,
		2,
		1,
		2,
		3
	};
	this->glyph = glyph;
	this->samplers = { Sampler2D(shader, glyph->texture) };
	this->generate();
	counter++;
}

cppogl::CharRect::CharRect(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	unit = other.unit;
	shader = other.shader;
	window = other.window;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

cppogl::CharRect::CharRect(CharRect && rvalue)
{
	_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	offset = rvalue.offset;
	baselineOffset = rvalue.baselineOffset;
	unit = rvalue.unit;
	shader = rvalue.shader;
	window = rvalue.window;
	glyph = rvalue.glyph;
	color = std::move(rvalue.color);
	samplers = rvalue.samplers;
	vertex = std::move(rvalue.vertex);
	index = std::move(rvalue.index);
	uv = std::move(rvalue.uv);
	model = std::move(rvalue.model);
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

cppogl::CharRect::~CharRect()
{
}

void cppogl::CharRect::operator=(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	unit = other.unit;
	shader = other.shader;
	window = other.window;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertexArray = other.vertexArray;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

void cppogl::CharRect::operator=(CharRect && rvalue)
{
	this->_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	offset = rvalue.offset;
	baselineOffset = rvalue.baselineOffset;
	unit = rvalue.unit;
	shader = rvalue.shader;
	window = rvalue.window;
	glyph = rvalue.glyph;
	color = std::move(rvalue.color);
	samplers = rvalue.samplers;
	vertex = std::move(rvalue.vertex);
	index = std::move(rvalue.index);
	uv = std::move(rvalue.uv);
	model = std::move(rvalue.model);
	vertexArray = rvalue.vertexArray;
	rvalue.vertexArray = 0;
	rvalue.vertex = {};
	rvalue.index = {};
	rvalue.uv = {};
}

void cppogl::CharRect::recalculate()
{
	float width = window->parse(UnitValue{ (float)glyph->border.width, unit }, Window::X);
	float height = window->parse(UnitValue{ (float)glyph->border.height, unit }, Window::Y);
	glm::vec3 scaled = glm::vec3(
		window->parse(UnitValue{ offset.x, unit }, Window::X),
		window->parse(UnitValue{ offset.y + baselineOffset, unit }, Window::Y),
		offset.z
	);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0) + scaled,
		glm::vec3(width, 0.0, 0.0) + scaled,
		glm::vec3(0.0, height, 0.0) + scaled,
		glm::vec3(width, height, 0.0) + scaled
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void cppogl::CharRect::render()
{
	glBindVertexArray(vertexArray);

	this->samplers[0].use();

	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glEnableVertexAttribArray(vertex.location);
	glVertexAttribPointer(vertex.location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uv.buffer);
	glEnableVertexAttribArray(uv.location);
	glVertexAttribPointer(uv.location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index.buffer);
	glDrawElements(GL_TRIANGLES, index.list.size(), GL_UNSIGNED_SHORT, nullptr);
}

cppogl::Glyph::Glyph()
{
	this->image = nullptr;
}

cppogl::Glyph::Glyph(char charecter, FT_Glyph& glyph, FT_Glyph_Metrics& metrics)
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
	this->image = std::make_shared<Image>(Image());
	this->image->width = next_power_n<2>(this->border.width);
	this->image->height = next_power_n<2>(this->border.height);
	this->image->bpp = 1;
	this->image->image = new unsigned char[this->image->width * this->image->bpp * this->image->height];
	for (int i = 0; i < this->image->width; i++) {
		for (int j = 0; j < this->image->height; j++) {
			int index = this->image->bpp * (i + j * this->image->width);
			if (i < this->border.width && j < this->border.height) {
				this->image->image[index] = bitmap.buffer[j * this->border.width + i];
			}
			else {
				this->image->image[index] = 0;
			}
		}
	}
	this->texture = std::make_shared<Texture>(Texture(this->image, GL_TEXTURE0, Texture::Format{ GL_R8, GL_RED }));
}

cppogl::Glyph::~Glyph()
{
}

cppogl::Text::Text()
{
	this->_shader = nullptr;
	this->_window = nullptr;
	this->_fontFamily = nullptr;
}

cppogl::Text::Text(sWindow window, sShaderProgram shader, sFontFamily fontfamily, std::string text, TextAttributes attributes)
{
	this->_shader = shader;
	this->_attributes = attributes;
	this->_fontFamily = fontfamily;
	this->_text = text;
	this->_window = window;
	this->_maxSize = _fontFamily->get(attributes.face)->lineHeight(_window->pixelValue(_attributes.fontSize));
	this->_model.location = glGetUniformLocation(this->_shader->id(), "model");
	if (this->_model.location < 0) {
		throw Exception("failed to locate shader uniform: model", EXCEPT_DETAIL_DEFAULT);
	}

	this->_window->registerListener("resize", this);

	this->_model.matrix = glm::mat4(1.0f);
	this->_model.matrix[3][0] = _attributes.position.x;
	this->_model.matrix[3][1] = _attributes.position.y;
	this->_model.matrix[3][2] = _attributes.position.z;

	this->generate(text, attributes);
}

cppogl::Text::~Text()
{
}

void cppogl::Text::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		for (int i = 0; i < _charecters.size(); i++) {
			_charecters[i].recalculate();
		}
	}
}

void cppogl::Text::generate(std::string text, TextAttributes attributes)
{
	_attributes = attributes;
	if (_fontFamily == nullptr || _window == nullptr || _shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	sFont font = _fontFamily->get(attributes.face);
	if (font->_generated.find(attributes.fontSize.value) == font->_generated.end()) {
		font->generate(attributes.fontSize.value);
	}
	_charecters.clear();
	this->_charecters.reserve(text.length());
	glm::vec2 offset = glm::vec2(0.0f, _maxSize);
	int wordIndex = 0;
	bool firstWord = true;
	for (int i = 0; i < text.length(); i++) {
		sGlyph& glyph = font->_generated[attributes.fontSize.value][text[i]];
		float baselineOffset = ((float)glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height;
		this->_charecters.push_back(CharRect(
			_window,
			_shader,
			glyph,
			glm::vec3(offset.x, -offset.y, 0.0),
			baselineOffset,
			_attributes.fontSize.unit
		));
		if (_attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
}

void cppogl::Text::update(std::string text)
{
	_maxSize = _attributes.fontSize.value;
	sFont font = _fontFamily->get(_attributes.face);
	glm::vec2 offset = glm::vec2(0.0f, _maxSize);
	int wordIndex = 0;
	bool firstWord = true;
	for (int i = 0; i < text.length(); i++) {
		sGlyph glyph = font->_generated[_attributes.fontSize.value][text[i]];
		if (i < _text.length()) {
			if (text[i] != _text[i] || _charecters[i].glyph->size != _attributes.fontSize.value) {
				_charecters[i] = CharRect(_window, _shader, glyph, glm::vec3(offset.x, -offset.y, _charecters.back().offset.z), -(glyph->border.height - glyph->metrics.horiBearingY / 64), _attributes.fontSize.unit);
			}
		}
		else {
			_charecters.push_back(CharRect(_window, _shader, glyph, glm::vec3(offset.x, -offset.y, _charecters.back().offset.z), -(glyph->border.height - glyph->metrics.horiBearingY / 64), _attributes.fontSize.unit));
		}
		if (_attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
	_charecters.resize(text.length());
	_text = text;
}

void cppogl::Text::append(std::string text, TextAttributes attributes)
{
	size_t len = _text.length() + text.length();
	_charecters.reserve(len);
	sFont font = _fontFamily->get(attributes.face);
	_maxSize = std::max(font->lineHeight(_window->pixelValue(_attributes.fontSize)), _maxSize);
	glm::vec2 offset = glm::vec2(0.0f, _maxSize);
	int wordIndex = 0;
	bool firstWord = true;
	for (int i = 0; i < len; i++) {
		sGlyph glyph = font->_generated[_attributes.fontSize.value][text[i]];
		_charecters.push_back(CharRect(
			_window,
			_shader,
			glyph,
			glm::vec3(offset.x, -offset.y, _charecters.back().offset.z),
			-(glyph->border.height - glyph->metrics.horiBearingY / 64),
			_attributes.fontSize.unit
		));
		if (_attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
	_text = _text + text;
}

void cppogl::Text::append(std::string text)
{
	this->append(text, _attributes);
}

void cppogl::Text::render()
{
	if (_fontFamily == nullptr || _window == nullptr || _shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	glUniformMatrix4fv(this->_model.location, 1, GL_FALSE, &this->_model.matrix[0][0]);
	for (int i = 0; i < _charecters.size(); i++) {
		_charecters[i].render();
	}
}

void cppogl::Text::_getOffsetWrapWidth(glm::vec2 & offset, sGlyph& glyph)
{
	if (glyph->charecter == '\n') {
		offset.x = 0.0f;
		offset.y += this->_fontFamily->get(_attributes.face)->lineHeight(_maxSize);
	}
	else if (_attributes.width == 0.0f) {
		offset.x += (glyph->glyph->advance.x >> 16);
	}
	else if (offset.x + (glyph->glyph->advance.x >> 16) < _attributes.width) {
		offset.x += (glyph->glyph->advance.x >> 16);
	}
	else {
		offset.x = 0.0f;
		offset.y += this->_fontFamily->get(_attributes.face)->lineHeight(_maxSize);
	}
}

void cppogl::Text::_getOffsetWrapWord(glm::vec2 & offset, sGlyph& glyph, int& wordIndex, int& index, bool& firstWord)
{
	if (glyph->charecter == '\n') {
		offset.x = 0.0f;
		wordIndex = index + 1;
		offset.y += this->_fontFamily->get(_attributes.face)->lineHeight(_maxSize);
		firstWord = true;
	}
	else if (_attributes.width == 0.0f) {
		offset.x += (glyph->glyph->advance.x >> 16);
	}
	else if (glyph->charecter == ' ' || glyph->charecter == '\t') {
		if (!firstWord) {
			offset.x += (glyph->glyph->advance.x >> 16);
		}
		else if (wordIndex == index) {
			
		}
		else {
			firstWord = false;
			offset.x += (glyph->glyph->advance.x >> 16);
		}
		wordIndex = index + 1;
	}
	else if (offset.x + (glyph->glyph->advance.x >> 16) < _attributes.width) {
		offset.x += (glyph->glyph->advance.x >> 16);
	}
	else {
		if (firstWord) {
			offset.x = 0.0f;
			offset.y += this->_fontFamily->get(_attributes.face)->lineHeight(_maxSize);
		}
		else {
			offset.x = 0.0f;
			offset.y += this->_fontFamily->get(_attributes.face)->lineHeight(_maxSize);
			for (int i = wordIndex; i <= index; i++) {
				_charecters[i].offset = glm::vec3(offset.x, -offset.y, _charecters[i].offset.z);
				_charecters[i].recalculate();
				_getOffsetWrapWidth(offset, _charecters[i].glyph);
			}
		}
	}
}

const std::string cppogl::FontFamily::REGULAR = std::string("regular");
const std::string cppogl::FontFamily::BOLD = std::string("bold");
const std::string cppogl::FontFamily::ITALIC = std::string("italic");
const std::string cppogl::FontFamily::ITALIC_BOLD = std::string("italic_bold");
const std::string cppogl::FontFamily::LIGHT = std::string("light");
const std::string cppogl::FontFamily::ITALIC_LIGHT = std::string("italic_light");


cppogl::FontFamily::FontFamily()
{
}

cppogl::FontFamily::FontFamily(std::string family, std::vector<std::pair<std::string, sFont>> fonts)
{
	_family = family;
	for (int i = 0; i < fonts.size(); i++) {
		_fonts[fonts[i].first] = fonts[i].second;
	}
}

cppogl::FontFamily::~FontFamily()
{
}

cppogl::sFont cppogl::FontFamily::get(const std::string& fontface) {
	if (_fonts.find(fontface) == _fonts.end()) {
		if (fontface == REGULAR) {
			throw Exception(std::string("could not find default font face in family: ") + _family, EXCEPT_DETAIL_DEFAULT);
		}
		return this->get(REGULAR);
	}
	else {
		return _fonts[fontface];
	}
}
