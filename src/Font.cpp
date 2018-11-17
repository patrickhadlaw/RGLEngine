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

float cppogl::Font::lineHeight(int size)
{
	FT_Set_Pixel_Sizes(_face, 0, size);
	return (float) _face->size->metrics.height / 64;
}

std::string & cppogl::Font::typeName()
{
	return std::string("cppogl::Font");
}

cppogl::CharRect::CharRect()
{
	this->shader = nullptr;
	this->window = nullptr;
}

cppogl::CharRect::CharRect(sWindow window, sShaderProgram shader, sGlyph glyph, UnitVector2D offset, float zIndex, UnitValue baselineOffset, UnitVector2D dimensions)
{
	this->shader = shader;
	this->window = window;
	this->offset = offset;
	this->zIndex = zIndex;
	this->baselineOffset = baselineOffset;
	this->dimensions = dimensions;
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", EXCEPT_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", EXCEPT_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(shader->programId(), "texture_coords");
	if (this->uv.location < 0) {
		throw Exception("failed to locate shader attribute: texture coordinates", EXCEPT_DETAIL_DEFAULT);
	}
	this->model.enabled = false;
	float width = this->dimensions.x.resolve(this->window, Window::X);
	float height = this->dimensions.y.resolve(this->window, Window::Y);
	glm::vec3 scaled = glm::vec3(
		offset.x.resolve(window),
		-offset.y.resolve(window, Window::Y) + baselineOffset.resolve(window, Window::Y),
		zIndex
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
}

cppogl::CharRect::CharRect(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
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
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
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
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
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
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
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
	float width = this->dimensions.x.resolve(this->window, Window::X);
	float height = this->dimensions.y.resolve(this->window, Window::Y);
	glm::vec3 scaled = glm::vec3(
		offset.x.resolve(window),
		-offset.y.resolve(window, Window::Y) + baselineOffset.resolve(window, Window::Y),
		zIndex
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
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index.list.size()), GL_UNSIGNED_SHORT, nullptr);
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
			if (static_cast<unsigned int>(i) < this->border.width && static_cast<unsigned int>(j) < this->border.height) {
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
	this->shader = nullptr;
	this->_context.window = nullptr;
	this->_fontFamily = nullptr;
}

cppogl::Text::Text(Context context, std::string shader, std::string fontFamily, std::string text, TextAttributes attributes)
{
	this->_context = context;
	this->shader = context.manager.shader->operator[](shader);
	this->_attributes = attributes;
	this->_elementAttributes = UI::ElementAttributes{ _attributes.zIndex };
	this->_topLeft = _attributes.topLeft;
	this->_dimensions.x = _attributes.dimensions.x;
	this->_fontFamily = context.manager.resource->getResource<FontFamily>(fontFamily);
	this->_text = text;
	float resolvedSize = attributes.fontSize.resolve(this->_context.window);
	int pointSize = static_cast<int>(_context.window->pointValue(_context.window->height() * resolvedSize, Window::Y));
	sFont font = _fontFamily->get(attributes.face);
	this->_maxSize = font->lineHeight(pointSize);
	this->_model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (this->_model.location < 0) {
		throw Exception("failed to locate shader uniform: model", EXCEPT_DETAIL_DEFAULT);
	}
	if (_attributes.dimensions.y.isZero()) {
		_dimensions.y = UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
	else {
		_dimensions.y = _attributes.dimensions.y;
	}

	this->_context.window->registerListener("resize", this);

	this->_model.matrix = this->transform();

	this->generate(text, attributes);
}

cppogl::Text::~Text()
{
}

void cppogl::Text::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->_model.matrix = this->transform();
		for (int i = 0; i < _charecters.size(); i++) {
			_charecters[i].recalculate();
		}
	}
}

void cppogl::Text::generate(std::string text, TextAttributes attributes)
{
	_attributes = attributes;
	if (_fontFamily == nullptr || _context.window == nullptr || shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	sFont font = _fontFamily->get(attributes.face);
	float resolvedSize = _attributes.fontSize.resolve(this->_context.window);
	int pointSize = static_cast<int>(_context.window->pointValue(_context.window->height() * resolvedSize, Window::Y));
	if (font->_generated.find(pointSize) == font->_generated.end()) {
		font->generate(pointSize);
	}
	_charecters.clear();
	this->_charecters.reserve(text.length());
	UnitVector2D offset = UnitVector2D(0.0f, _scale(_maxSize), _attributes.fontSize.unit);
	offset += _attributes.topLeft;
	int wordIndex = 0;
	bool firstWord = true;
	for (int i = 0; i < text.length(); i++) {
		sGlyph& glyph = font->_generated[pointSize][text[i]];
		float baselineOffset = _scale(((float)glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height);
		this->_charecters.push_back(CharRect(
			_context.window,
			shader,
			glyph,
			offset,
			_elementAttributes.zIndex,
			UnitValue{ baselineOffset, _attributes.fontSize.unit },
			UnitVector2D(_scale(glyph->border.width), _scale(glyph->border.height), _attributes.fontSize.unit)
		));
		if (_attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
	if (_attributes.dimensions.y.isZero()) {
		this->_dimensions.y = offset.y + UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
}

void cppogl::Text::update(std::string text)
{
	sFont font = _fontFamily->get(_attributes.face);
	UnitVector2D offset = UnitVector2D(0.0f, _scale(_maxSize), _attributes.fontSize.unit);
	offset += _attributes.topLeft;
	int wordIndex = 0;
	bool firstWord = true;
	float resolvedSize = _attributes.fontSize.resolve(this->_context.window);
	int pointSize = static_cast<int>(_context.window->pointValue(_context.window->height() * resolvedSize, Window::Y));
	if (font->_generated.find(pointSize) == font->_generated.end()) {
		font->generate(pointSize);
	}
	for (int i = 0; i < text.length(); i++) {
		sGlyph glyph = font->_generated[pointSize][text[i]];
		if (i < _text.length()) {
			if (text[i] != _text[i] || _charecters[i].glyph->size != _attributes.fontSize.value) {
				_charecters[i] = CharRect(_context.window,
					shader,
					glyph,
					offset,
					_elementAttributes.zIndex,
					UnitValue{ static_cast<float>(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), _attributes.fontSize.unit },
					UnitVector2D(_scale(glyph->border.width), _scale(glyph->border.height), _attributes.fontSize.unit)
				);
			}
		}
		else {
			_charecters.push_back(CharRect(_context.window,
				shader,
				glyph,
				offset,
				_elementAttributes.zIndex,
				UnitValue{ static_cast<float>(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), _attributes.fontSize.unit },
				UnitVector2D(_scale(glyph->border.width), _scale(glyph->border.height), _attributes.fontSize.unit))
			);
		}
		if (_attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
	if (_attributes.dimensions.y.isZero()) {
		this->_dimensions.y = offset.y + UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
	_charecters.resize(text.length());
	_text = text;
}

void cppogl::Text::append(std::string text, TextAttributes attributes)
{
	size_t len = _text.length() + text.length();
	_charecters.reserve(len);
	sFont font = _fontFamily->get(attributes.face);
	float resolvedSize = attributes.fontSize.resolve(this->_context.window);
	int pointSize = static_cast<int>(_context.window->pointValue(_context.window->height() * resolvedSize, Window::Y));
	_maxSize = std::max(font->lineHeight(pointSize), _maxSize);
	UnitVector2D offset = UnitVector2D(0.0f, _scale(_maxSize), _attributes.fontSize.unit);
	offset += _attributes.topLeft;
	int wordIndex = 0;
	bool firstWord = true;
	for (int i = 0; i < len; i++) {
		sGlyph glyph = font->_generated[pointSize][text[i]];
		_charecters.push_back(CharRect(
			_context.window,
			shader,
			glyph,
			offset,
			_elementAttributes.zIndex,
			UnitValue{ _scale(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), _attributes.fontSize.unit },
			UnitVector2D(_scale(glyph->border.width), _scale(glyph->border.height), _attributes.fontSize.unit)
		));
		if (attributes.wrapWord) {
			_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
		}
		else {
			_getOffsetWrapWidth(offset, glyph);
		}
	}
	if (_attributes.dimensions.y.isZero()) {
		this->_dimensions.y = offset.y + UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
	_text = _text + text;
}

void cppogl::Text::append(std::string text)
{
	this->append(text, _attributes);
}

void cppogl::Text::render()
{
	if (_fontFamily == nullptr || _context.window == nullptr || shader == nullptr) {
		throw NullPointerException(EXCEPT_DETAIL_DEFAULT);
	}
	glUniformMatrix4fv(this->_model.location, 1, GL_FALSE, &this->_model.matrix[0][0]);
	for (int i = 0; i < _charecters.size(); i++) {
		_charecters[i].render();
	}
}

void cppogl::Text::onBoxUpdate()
{
	this->_model.matrix = this->transform();
}

std::string & cppogl::Text::typeName()
{
	return std::string("cppogl::Text");
}

float cppogl::Text::_scale(float value)
{
	float resolvedSize = _attributes.fontSize.resolve(this->_context.window);
	int pointSize = static_cast<int>(_context.window->pointValue(_context.window->height() * resolvedSize, Window::Y));

	return  (value / (float) pointSize) * _attributes.fontSize.value;
}

void cppogl::Text::_getOffsetWrapWidth(UnitVector2D & offset, sGlyph& glyph)
{
	if (glyph->charecter == '\n') {
		offset.x = this->_topLeft.x;
		offset.y += UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
	else if (_attributes.dimensions.x.isZero()) {
		offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
	}
	else if ((offset.x + UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit }).lessThan(_attributes.dimensions.x, _context.window)) {
		offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
	}
	else {
		offset.x = this->_topLeft.x;
		offset.y += UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
	}
}

void cppogl::Text::_getOffsetWrapWord(UnitVector2D & offset, sGlyph& glyph, int& wordIndex, int& index, bool& firstWord)
{
	float resolved = offset.x.resolve(_context.window);
	if (glyph->charecter == '\n') {
		offset.x = this->_topLeft.x;
		wordIndex = index + 1;
		offset.y += UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
		firstWord = true;
	}
	else if (_attributes.dimensions.x.isZero()) {
		offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
	}
	else if (glyph->charecter == ' ' || glyph->charecter == '\t') {
		if (!firstWord) {
			offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
		}
		else if (wordIndex == index) {
			
		}
		else {
			firstWord = false;
			offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
		}
		wordIndex = index + 1;
	}
	else if ((offset.x + UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit }).lessThan(_attributes.dimensions.x, _context.window)) {
		offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), _attributes.fontSize.unit };
	}
	else {
		if (firstWord) {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
		}
		else {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ _scale(_maxSize), _attributes.fontSize.unit };
			for (int i = wordIndex; i <= index; i++) {
				_charecters[i].offset = offset;
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
	id = family;
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
			throw Exception(std::string("could not find default font face in family: ") + id, EXCEPT_DETAIL_DEFAULT);
		}
		return this->get(REGULAR);
	}
	else {
		return _fonts[fontface];
	}
}

std::string & cppogl::FontFamily::typeName()
{
	return std::string("cppogl::FontFamily");
}
