#include "rgle/ui/Text.h"

rgle::ui::Text::Text(std::string shaderid, std::string fontFamily, std::string text, TextAttributes attributes)
{
	auto window = this->context().window.lock();
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	this->_attributes = attributes;
	this->_elementAttributes = ui::ElementAttributes{ _attributes.zIndex };
	this->_topLeft = _attributes.topLeft;
	this->_dimensions.x = _attributes.dimensions.x;
	this->_fontFamily = this->context().manager.resource.lock()->getResource<res::FontFamily>(fontFamily);
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

rgle::ui::Text::~Text()
{
}

void rgle::ui::Text::onMessage(std::string eventname, EventMessage *)
{
	if (eventname == "resize") {
		this->_model.matrix = this->transform();
		for (size_t i = 0; i < this->_characters.size(); i++) {
			this->_characters[i].recalculate();
		}
	}
}

void rgle::ui::Text::generate(std::string text, TextAttributes attributes)
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
		size_t wordIndex = 0;
		bool firstWord = true;
		for (size_t i = 0; i < text.length(); i++) {
			std::shared_ptr<res::Glyph>& glyph = font->_generated[pointSize][text[i]];
			float baselineOffset = _scale(((float)glyph->bbox.yMin / (glyph->bbox.yMax - glyph->bbox.yMin)) * glyph->border.height);
			this->_characters.push_back(gfx::CharRect(
				shader,
				glyph,
				offset,
				this->_elementAttributes.zIndex,
				UnitValue{ baselineOffset, _attributes.fontSize.unit },
				UnitVector2D(
					this->_scale(static_cast<float>(glyph->border.width)),
					this->_scale(static_cast<float>(glyph->border.height)),
					this->_attributes.fontSize.unit
				)
			));
			if (_attributes.wrapWord) {
				this->_getOffsetWrapWord(offset, glyph, wordIndex, i, firstWord);
			}
			else {
				this->_getOffsetWrapWidth(offset, glyph);
			}
			if (_attributes.dimensions.x.isZero()) {
				if (offset.x.greaterThan(_dimensions.x, window)) {
					_dimensions.x = offset.x;
				}
			}
		}
	});
}

void rgle::ui::Text::update(std::string text)
{
	this->context().executeInContext([this, &text]() {
		auto font = this->_fontFamily->get(this->_attributes.face);
		auto window = this->context().window.lock();
		auto shader = this->shaderLocked();
		UnitVector2D offset = UnitVector2D(0.0f, this->_scale(this->_maxSize), this->_attributes.fontSize.unit);
		size_t wordIndex = 0;
		bool firstWord = true;
		float resolvedSize = _attributes.fontSize.resolve(window, Window::Y);
		int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));
		if (font->_generated.find(pointSize) == font->_generated.end()) {
			font->generate(pointSize);
		}
		for (size_t i = 0; i < text.length(); i++) {
			std::shared_ptr<res::Glyph> glyph = font->_generated[pointSize][text[i]];
			if (i < this->_text.length()) {
				if (text[i] != this->_text[i] || this->_characters[i].glyph->size != this->_attributes.fontSize.value) {
					this->_characters[i] = gfx::CharRect(
						shader,
						glyph,
						offset,
						this->_elementAttributes.zIndex,
						UnitValue{ -static_cast<float>(glyph->border.height - glyph->metrics.horiBearingY / 64), this->_attributes.fontSize.unit },
						UnitVector2D(
							this->_scale(static_cast<float>(glyph->border.width)),
							this->_scale(static_cast<float>(glyph->border.height)),
							this->_attributes.fontSize.unit
						)
					);
				}
			}
			else {
				this->_characters.push_back(gfx::CharRect(
					shader,
					glyph,
					offset,
					this->_elementAttributes.zIndex,
					UnitValue{ -static_cast<float>(glyph->border.height - glyph->metrics.horiBearingY / 64), this->_attributes.fontSize.unit },
					UnitVector2D(
						this->_scale(static_cast<float>(glyph->border.width)),
						this->_scale(static_cast<float>(glyph->border.height)),
						this->_attributes.fontSize.unit
					)
				));
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

void rgle::ui::Text::append(std::string text, TextAttributes attributes)
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
		size_t wordIndex = 0;
		bool firstWord = true;
		for (size_t i = 0; i < len; i++) {
			std::shared_ptr<res::Glyph> glyph = font->_generated[pointSize][text[i]];
			this->_characters.push_back(gfx::CharRect(
				shader,
				glyph,
				offset,
				this->_elementAttributes.zIndex,
				UnitValue{
					this->_scale(-static_cast<float>(glyph->border.height - glyph->metrics.horiBearingY / 64)),
					this->_attributes.fontSize.unit
				},
				UnitVector2D(
					this->_scale(static_cast<float>(glyph->border.width)),
					this->_scale(static_cast<float>(glyph->border.height)),
					this->_attributes.fontSize.unit
				)
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

void rgle::ui::Text::append(std::string text)
{
	this->append(text, _attributes);
}

void rgle::ui::Text::render()
{
	glUniformMatrix4fv(this->_model.location, 1, GL_FALSE, &this->_model.matrix[0][0]);
	for (size_t i = 0; i < this->_characters.size(); i++) {
		this->_characters[i].render();
	}
}

void rgle::ui::Text::onBoxUpdate()
{
	this->_model.matrix = this->transform();
}

const char * rgle::ui::Text::typeName() const
{
	return "rgle::ui::Text";
}

float rgle::ui::Text::_scale(float value)
{
	auto window = this->context().window.lock();
	float resolvedSize = _attributes.fontSize.resolve(window, Window::Y);
	int pointSize = static_cast<int>(window->pointValue(window->height() * resolvedSize, Window::Y));

	return  (value / (float) pointSize) * this->_attributes.fontSize.value;
}

void rgle::ui::Text::_getOffsetWrapWidth(UnitVector2D & offset, std::shared_ptr<res::Glyph>& glyph)
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
		offset.x += UnitValue{
			this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)),
			this->_attributes.fontSize.unit
		};
	}
	else if ((offset.x + UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit })
		.lessThan(this->_attributes.dimensions.x, window))
	{
		offset.x += UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit };
	}
	else {
		offset.x = this->_topLeft.x;
		offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		if (_attributes.dimensions.y.isZero()) {
			this->_dimensions.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
	}
}

void rgle::ui::Text::_getOffsetWrapWord(
	UnitVector2D & offset,
	std::shared_ptr<res::Glyph> & glyph,
	size_t & wordIndex,
	size_t & index,
	bool & firstWord)
{
	auto window = this->context().window.lock();
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
		offset.x += UnitValue{ _scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit };
	}
	else if (glyph->charecter == ' ' || glyph->charecter == '\t') {
		if (!firstWord) {
			offset.x += UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit };
		}
		else if (wordIndex == index) {
			
		}
		else {
			firstWord = false;
			offset.x += UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit };
		}
		wordIndex = index + 1;
	}
	else if ((offset.x + UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit })
		.lessThan(this->_attributes.dimensions.x, window))
	{
		offset.x += UnitValue{ this->_scale(static_cast<float>(glyph->glyph->advance.x >> 16)), this->_attributes.fontSize.unit };
	}
	else {
		if (firstWord) {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
		}
		else {
			offset.x = this->_topLeft.x;
			offset.y += UnitValue{ this->_scale(this->_maxSize), this->_attributes.fontSize.unit };
			for (size_t i = wordIndex; i <= index; i++) {
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
