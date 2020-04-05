#include "rgle/Font.h"
#include "rgle/Graphics.h"


rgle::Font::Font()
{
	this->_window = nullptr;
}

rgle::Font::Font(std::shared_ptr<Window> window, std::string fontfile)
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

rgle::Font::~Font()
{
}

void rgle::Font::generate(int size)
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

float rgle::Font::lineHeight(int size)
{
	FT_Set_Pixel_Sizes(_face, 0, size);
	return (float) _face->size->metrics.height / 64;
}

const char * rgle::Font::typeName() const
{
	return "rgle::Font";
}

rgle::CharRect::CharRect()
{
}

rgle::CharRect::CharRect(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Glyph> glyph, UnitVector2D offset, float zIndex, UnitValue baselineOffset, UnitVector2D dimensions)
{
	auto window = this->context().window.lock();
	this->shader() = shader;
	this->offset = offset;
	this->zIndex = zIndex;
	this->baselineOffset = baselineOffset;
	this->dimensions = dimensions;
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	this->uv.location = glGetAttribLocation(shader->programId(), "texture_coords");
	if (this->uv.location < 0) {
		throw Exception("failed to locate shader attribute: texture coordinates", LOGGER_DETAIL_DEFAULT);
	}
	this->model.enabled = false;
	float width = this->dimensions.x.resolve(window, Window::X);
	float height = this->dimensions.y.resolve(window, Window::Y);
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

rgle::CharRect::CharRect(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

rgle::CharRect::CharRect(CharRect && rvalue)
{
	_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
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

rgle::CharRect::~CharRect()
{
}

void rgle::CharRect::operator=(const CharRect & other)
{
	_cleanup();
	width = other.width;
	height = other.height;
	zIndex = other.zIndex;
	offset = other.offset;
	baselineOffset = other.baselineOffset;
	dimensions = other.dimensions;
	glyph = other.glyph;
	color = other.color;
	samplers = other.samplers;
	vertexArray = other.vertexArray;
	vertex = other.vertex;
	index = other.index;
	uv = other.uv;
	this->generate();
}

void rgle::CharRect::operator=(CharRect && rvalue)
{
	this->_cleanup();
	width = rvalue.width;
	height = rvalue.height;
	zIndex = rvalue.zIndex;
	offset = std::move(rvalue.offset);
	baselineOffset = rvalue.baselineOffset;
	dimensions = rvalue.dimensions;
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

void rgle::CharRect::recalculate()
{
	auto window = this->context().window.lock();
	float width = this->dimensions.x.resolve(window, Window::X);
	float height = this->dimensions.y.resolve(window, Window::Y);
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

void rgle::CharRect::render()
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

rgle::Glyph::Glyph()
{
	this->image = nullptr;
}

rgle::Glyph::Glyph(char charecter, FT_Glyph& glyph, FT_Glyph_Metrics& metrics)
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
	this->image->channels = 1;
	this->image->image = new unsigned char[this->image->width * this->image->channels * this->image->height];
	for (int i = 0; i < this->image->width; i++) {
		for (int j = 0; j < this->image->height; j++) {
			int index = this->image->channels * (i + j * this->image->width);
			if (static_cast<unsigned int>(i) < this->border.width && static_cast<unsigned int>(j) < this->border.height) {
				this->image->image[index] = bitmap.buffer[j * this->border.width + i];
			}
			else {
				this->image->image[index] = 0;
			}
		}
	}
	this->texture = std::make_shared<Texture2D>(this->image, GL_TEXTURE0, Texture2D::Format{ GL_R8, GL_RED });
}

rgle::Glyph::~Glyph()
{
}

rgle::Text::Text(std::string shaderid, std::string fontFamily, std::string text, TextAttributes attributes)
{
	auto window = this->context().window.lock();
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	this->_attributes = attributes;
	this->_elementAttributes = UI::ElementAttributes{ _attributes.zIndex };
	this->_topLeft = _attributes.topLeft;
	this->_dimensions.x = _attributes.dimensions.x;
	this->_fontFamily = this->context().manager.resource.lock()->getResource<FontFamily>(fontFamily);
	this->_text = text;
	float resolvedSize = attributes.fontSize.resolve(window, Window::Y);
	int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));
	auto font = this->_fontFamily->get(attributes.face);
	this->_maxSize = font->lineHeight(pointSize);
	this->_model.location = glGetUniformLocation(shader->programId(), "model");
	if (this->_model.location < 0) {
		throw Exception("failed to locate shader uniform: model", LOGGER_DETAIL_DEFAULT);
	}
	if (this->_attributes.dimensions.y.isZero()) {
		this->_dimensions.y = UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
	}
	else {
		this->_dimensions.y = this->_attributes.dimensions.y;
	}

	if (!this->_attributes.dimensions.x.isZero()) {
		this->_dimensions.x = this->_attributes.dimensions.x;
	}

	window->registerListener("resize", this);

	this->_model.matrix = this->transform();

	this->generate(text, attributes);
}

rgle::Text::~Text()
{
}

void rgle::Text::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->_model.matrix = this->transform();
		for (int i = 0; i < this->_characters.size(); i++) {
			this->_characters[i].recalculate();
		}
	}
}

void rgle::Text::generate(std::string text, TextAttributes attributes)
{
	this->context().executeInContext([this, &text, &attributes]() {
		auto window = this->context().window.lock();
		auto shader = this->shaderLocked();
		this->_attributes = attributes;
		if (this->_fontFamily == nullptr) {
			throw NullPointerException(LOGGER_DETAIL_DEFAULT);
		}
		auto font = this->_fontFamily->get(attributes.face);
		float resolvedSize = this->_attributes.fontSize.resolve(window, Window::Y);
		int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));
		if (font->_generated.find(pointSize) == font->_generated.end()) {
			font->generate(pointSize);
		}
		this->_characters.clear();
		this->_characters.reserve(text.length());
		UnitVector2D offset = UnitVector2D(0.0f, _scale(_maxSize), _attributes.fontSize.unit);
		int wordIndex = 0;
		bool firstWord = true;
		for (int i = 0; i < text.length(); i++) {
			std::shared_ptr<Glyph>& glyph = font->_generated[pointSize][text[i]];
			float baselineOffset = _scale(((float)glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height);
			this->_characters.push_back(CharRect(
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
			if (_attributes.dimensions.x.isZero()) {
				if (offset.x.greaterThan(_dimensions.x, window)) {
					_dimensions.x = offset.x;
				}
			}
		}
	});
}

void rgle::Text::update(std::string text)
{
	this->context().executeInContext([this, &text]() {
		auto font = this->_fontFamily->get(this->_attributes.face);
		auto window = this->context().window.lock();
		auto shader = this->shaderLocked();
		UnitVector2D offset = UnitVector2D(0.0f, this->_scale(this->_maxSize), this->_attributes.fontSize.unit);
		int wordIndex = 0;
		bool firstWord = true;
		float resolvedSize = _attributes.fontSize.resolve(window, Window::Y);
		int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));
		if (font->_generated.find(pointSize) == font->_generated.end()) {
			font->generate(pointSize);
		}
		for (int i = 0; i < text.length(); i++) {
			std::shared_ptr<Glyph> glyph = font->_generated[pointSize][text[i]];
			if (i < this->_text.length()) {
				if (text[i] != this->_text[i] || this->_characters[i].glyph->size != this->_attributes.fontSize.value) {
					this->_characters[i] = CharRect(
						shader,
						glyph,
						offset,
						this->_elementAttributes.zIndex,
						UnitValue{ static_cast<float>(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), this->_attributes.fontSize.unit },
						UnitVector2D(this->_scale(glyph->border.width), this->_scale(glyph->border.height), this->_attributes.fontSize.unit)
					);
				}
			}
			else {
				this->_characters.push_back(CharRect(
					shader,
					glyph,
					offset,
					this->_elementAttributes.zIndex,
					UnitValue{ static_cast<float>(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), this->_attributes.fontSize.unit },
					UnitVector2D(this->_scale(glyph->border.width), this->_scale(glyph->border.height), this->_attributes.fontSize.unit))
				);
			}
			if (this->_attributes.wrapWord) {
				this->_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
			}
			else {
				this->_getOffsetWrapWidth(offset, glyph);
			}
			if (_attributes.dimensions.x.isZero()) {
				if (offset.x.greaterThan(this->_dimensions.x, window)) {
					this->_dimensions.x = offset.x;
				}
			}
		}
		this->_characters.resize(text.length());
		this->_text = text;
	});
}

void rgle::Text::append(std::string text, TextAttributes attributes)
{
	this->context().executeInContext([this, &text, &attributes]() {
		auto window = this->context().window.lock();
		auto shader = this->shaderLocked();
		size_t len = this->_text.length() + text.length();
		this->_characters.reserve(len);
		auto font = this->_fontFamily->get(attributes.face);
		float resolvedSize = attributes.fontSize.resolve(window, Window::Y);
		int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));
		this->_maxSize = std::max(font->lineHeight(pointSize), this->_maxSize);
		UnitVector2D offset = UnitVector2D(0.0f, this->_scale(this->_maxSize), this->_attributes.fontSize.unit);
		int wordIndex = 0;
		bool firstWord = true;
		for (int i = 0; i < len; i++) {
			std::shared_ptr<Glyph> glyph = font->_generated[pointSize][text[i]];
			this->_characters.push_back(CharRect(
				shader,
				glyph,
				offset,
				this->_elementAttributes.zIndex,
				UnitValue{ this->_scale(-(glyph->border.height - glyph->metrics.horiBearingY / 64)), this->_attributes.fontSize.unit },
				UnitVector2D(this->_scale(glyph->border.width), this->_scale(glyph->border.height), this->_attributes.fontSize.unit)
			));
			if (attributes.wrapWord) {
				this->_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
			}
			else {
				this->_getOffsetWrapWidth(offset, glyph);
			}
			if (this->_attributes.dimensions.x.isZero()) {
				if (offset.x.greaterThan(this->_dimensions.x, window)) {
					this->_dimensions.x = offset.x;
				}
			}
		}
		this->_text = this->_text + text;
	});
}

void rgle::Text::append(std::string text)
{
	this->append(text, _attributes);
}

void rgle::Text::render()
{
	glUniformMatrix4fv(this->_model.location, 1, GL_FALSE, &this->_model.matrix[0][0]);
	for (int i = 0; i < this->_characters.size(); i++) {
		this->_characters[i].render();
	}
}

void rgle::Text::onBoxUpdate()
{
	this->_model.matrix = this->transform();
}

const char * rgle::Text::typeName() const
{
	return "rgle::Text";
}

float rgle::Text::_scale(float value)
{
	auto window = this->context().window.lock();
	float resolvedSize = _attributes.fontSize.resolve(window, Window::Y);
	int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));

	return  (value / (float) pointSize) * this->_attributes.fontSize.value;
}

void rgle::Text::_getOffsetWrapWidth(UnitVector2D & offset, std::shared_ptr<Glyph>& glyph)
{
	auto window = this->context().window.lock();
	if (glyph->charecter == '\n') {
		offset.x = this->_topLeft.x;
		offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		if (_attributes.dimensions.y.isZero()) {
			this->_dimensions.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
	}
	else if (this->_attributes.dimensions.x.isZero()) {
		offset.x += UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
	}
	else if ((offset.x + UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit }).lessThan(this->_attributes.dimensions.x, window)) {
		offset.x += UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
	}
	else {
		offset.x = this->_topLeft.x;
		offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		if (_attributes.dimensions.y.isZero()) {
			this->_dimensions.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
	}
}

void rgle::Text::_getOffsetWrapWord(UnitVector2D & offset, std::shared_ptr<Glyph>& glyph, int& wordIndex, int& index, bool& firstWord)
{
	auto window = this->context().window.lock();
	float resolved = offset.x.resolve(window);
	if (glyph->charecter == '\n') {
		offset.x = this->_topLeft.x;
		wordIndex = index + 1;
		offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		if (this->_attributes.dimensions.y.isZero()) {
			this->_dimensions.y += UnitValue{ this->_scale(_maxSize), this->_attributes.fontSize.unit };
		}
		firstWord = true;
	}
	else if (this->_attributes.dimensions.x.isZero()) {
		offset.x += UnitValue{ _scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
	}
	else if (glyph->charecter == ' ' || glyph->charecter == '\t') {
		if (!firstWord) {
			offset.x += UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
		}
		else if (wordIndex == index) {
			
		}
		else {
			firstWord = false;
			offset.x += UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
		}
		wordIndex = index + 1;
	}
	else if ((offset.x + UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit }).lessThan(this->_attributes.dimensions.x, window)) {
		offset.x += UnitValue{ this->_scale(glyph->glyph->advance.x >> 16), this->_attributes.fontSize.unit };
	}
	else {
		if (firstWord) {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
		else {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
			for (int i = wordIndex; i <= index; i++) {
				this->_characters[i].offset = offset;
				this->_characters[i].recalculate();
				this->_getOffsetWrapWidth(offset, this->_characters[i].glyph);
			}
		}
		if (this->_attributes.dimensions.y.isZero()) {
			this->_dimensions.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
	}
}

rgle::FontFamily::FontFamily()
{
}

rgle::FontFamily::FontFamily(std::string family, std::vector<std::pair<std::string, std::shared_ptr<Font>>> fonts)
{
	id = family;
	for (int i = 0; i < fonts.size(); i++) {
		this->_fonts[fonts[i].first] = fonts[i].second;
	}
}

rgle::FontFamily::~FontFamily()
{
}

std::shared_ptr<rgle::Font> rgle::FontFamily::get(const std::string& fontface) {
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

const char * rgle::FontFamily::typeName() const
{
	return "rgle::FontFamily";
}
