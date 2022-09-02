#include "rgle/gfx/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

rgle::Image::Image() :
	image(nullptr),
	width(0),
	height(0),
	depth(0),
	channels(0),
	channelSize(0)
{
}

rgle::Image::Image(std::string imagefile) : depth(1), channelSize(1)
{
	RGLE_DEBUG_ONLY(rgle::Logger::debug("loading image: " + imagefile, LOGGER_DETAIL_DEFAULT);)
		int w, h, c;
	image = stbi_load(imagefile.data(), &w, &h, &c, STBI_rgb_alpha);
	if (this->image == nullptr) {
		throw IOException("failed to load image: '" + imagefile + '\'', LOGGER_DETAIL_DEFAULT);
	}
	if (w <= 0 || h <= 0 || c <= 0) {
		throw IOException("failed to load image: '" + imagefile + "', invalid image", LOGGER_DETAIL_DEFAULT);
	}
	this->width = static_cast<size_t>(w);
	this->height = static_cast<size_t>(h);
	this->channels = static_cast<size_t>(c);
}

rgle::Image::Image(size_t width, size_t height, size_t depth, size_t channels, size_t channelSize) :
	width(width),
	height(height),
	depth(depth),
	channels(channels),
	channelSize(channelSize)
{
	this->image = new unsigned char[this->size()];
}

rgle::Image::Image(const Image & other)
{
	RGLE_DEBUG_ASSERT(other.image != nullptr)
		this->width = other.width;
	this->height = other.height;
	this->depth = other.depth;
	this->channels = other.channels;
	this->channelSize = other.channelSize;
	this->image = new unsigned char[this->size()];
	std::memcpy(this->image, other.image, this->size());
}

rgle::Image::Image(Image && rvalue)
{
	this->width = rvalue.width;
	this->height = rvalue.height;
	this->depth = rvalue.depth;
	this->channels = rvalue.channels;
	this->channelSize = rvalue.channelSize;
	this->image = rvalue.image;
	rvalue.image = nullptr;
}

void rgle::Image::operator=(const Image& other)
{
	RGLE_DEBUG_ASSERT(other.image != nullptr)
		delete[] image;
	image = nullptr;
	this->width = other.width;
	this->height = other.height;
	this->depth = other.depth;
	this->channels = other.channels;
	this->image = other.image;
	this->image = new unsigned char[this->size()];
	for (size_t i = 0; i < this->size(); i++) {
		this->image[i] = other.image[i];
	}
}

void rgle::Image::operator=(Image && rvalue)
{
	this->width = rvalue.width;
	this->height = rvalue.height;
	this->depth = rvalue.depth;
	this->channels = rvalue.channels;
	this->channelSize = rvalue.channelSize;
	std::swap(this->image, rvalue.image);
}

void rgle::Image::set(const size_t & x, const size_t & y, unsigned char * data, const size_t & size)
{
	if (size != this->channels * this->channelSize) {
		throw IllegalArgumentException("invalid image set, payload size invalid", LOGGER_DETAIL_DEFAULT);
	}
	else if (x >= this->width || y >= this->height) {
		throw OutOfBoundsException(LOGGER_DETAIL_DEFAULT);
	}
	else {
		std::memcpy(
			this->image + (y * this->width * this->channels * this->channelSize) + (x * this->channels * this->channelSize),
			data,
			size
		);
	}
}

void rgle::Image8::set(const size_t & x, const size_t & y, const float & intensity)
{
	unsigned char data = static_cast<unsigned char>(intensity * 255);
	Image::set(x, y, &data, 1);
}

void rgle::Image8::set(const size_t & x, const size_t & y, const glm::vec2 & ia)
{
	unsigned char data[2] = {
		static_cast<unsigned char>(ia.r * 255),
		static_cast<unsigned char>(ia.g * 255)
	};
	Image::set(x, y, data, 2);
}

void rgle::Image8::set(const size_t & x, const size_t & y, const glm::vec3 & rgb)
{
	unsigned char data[3] = {
		static_cast<unsigned char>(rgb.r * 255),
		static_cast<unsigned char>(rgb.g * 255),
		static_cast<unsigned char>(rgb.b * 255)
	};
	Image::set(x, y, data, 3);
}

void rgle::Image8::set(const size_t & x, const size_t & y, const glm::vec4& rgba)
{
	unsigned char data[4] = {
		static_cast<unsigned char>(rgba.r * 255),
		static_cast<unsigned char>(rgba.g * 255),
		static_cast<unsigned char>(rgba.b * 255),
		static_cast<unsigned char>(rgba.a * 255)
	};
	Image::set(x, y, data, 4);
}

void rgle::Image::write(const std::string& imagefile) const
{
	size_t idx = imagefile.find_last_of('.');
	if (idx == std::string::npos) {
		throw IOException("failed to write image to file: " + imagefile + ", missing extension", LOGGER_DETAIL_DEFAULT);
	}
	else {
		std::string ext = imagefile.substr(idx + 1);
		if (ext == "png") {
			this->write(imagefile, Format::PNG);
		}
		else if (ext == "jpg") {
			this->write(imagefile, Format::JPEG);
		}
		else {
			throw IOException("failed to write image to file: " + imagefile + ", format not supported", LOGGER_DETAIL_DEFAULT);
		}
	}
}

void rgle::Image::write(const std::string& imagefile, const Format& format) const
{
	rgle::Logger::info("writing image to file: " + imagefile, LOGGER_DETAIL_DEFAULT);
	int status;
	switch (format) {
	case Format::JPEG:
		status = stbi_write_jpg(imagefile.c_str(), this->width, this->height, this->channels, this->image, 100);
		break;
	case Format::PNG:
		status = stbi_write_png(imagefile.c_str(), this->width, this->height, this->channels, this->image, this->width * this->channels);
		break;
	}
	if (status < 0) {
		throw IOException(
			"failed to write image to file: " + imagefile + ", error code: " + std::to_string(status),
			LOGGER_DETAIL_DEFAULT
		);
	}
}

size_t rgle::Image::size() const
{
	return this->width * this->height * this->depth * this->channels * this->channelSize;
}

rgle::Image::~Image()
{
	delete[] image;
	image = nullptr;
}

rgle::Image8::Image8() : Image()
{
}

rgle::Image8::Image8(std::string imagefile) : Image(imagefile)
{
}

rgle::Image8::Image8(int width, int height, int channels) : Image(width, height, 1, channels, 1)
{
}

rgle::Texture::Texture() : Texture(nullptr, 0)
{
}

rgle::Texture::Texture(std::shared_ptr<Image> image, int index) : _image(image), _index(index)
{
	glGenTextures(1, &this->_id);
}

rgle::Texture::Texture(const Texture& other) : Texture(std::make_shared<Image>(*other._image), other._index)
{
}

rgle::Texture::Texture(Texture&& rvalue)
{
	this->_image = rvalue._image;
	rvalue._image = nullptr;
	this->_id = rvalue._id;
	rvalue._id = 0;
}

rgle::Texture::~Texture()
{
	glDeleteTextures(1, &this->_id);
}

void rgle::Texture::operator=(const Texture& other)
{
	this->_image = other._image;
	glGenTextures(1, &this->_id);
}

void rgle::Texture::operator=(Texture&& rvalue)
{
	this->_image = rvalue._image;
	std::swap(this->_id, rvalue._id);
}

void rgle::Texture::update()
{
	rgle::Logger::warn("unimplemented method called", LOGGER_DETAIL_DEFAULT);
}

void rgle::Texture::bind()
{
	rgle::Logger::warn("unimplemented method called", LOGGER_DETAIL_DEFAULT);
}

GLuint & rgle::Texture::id()
{
	return this->_id;
}

const GLuint & rgle::Texture::id() const
{
	return this->_id;
}

std::shared_ptr<rgle::Image>& rgle::Texture::image()
{
	return this->_image;
}

const std::shared_ptr<rgle::Image>& rgle::Texture::image() const
{
	return this->_image;
}

int & rgle::Texture::index()
{
	return this->_index;
}

const int & rgle::Texture::index() const
{
	return this->_index;
}

rgle::Texture2D::Texture2D(std::string imagefile, GLenum texture, Format format) :
	Texture2D(std::make_shared<Image>(imagefile), texture, format)
{
}

rgle::Texture2D::Texture2D(std::shared_ptr<Image> image, GLenum texture, Format format) :
	_texture(texture),
	_format(format),
	Texture(image, texture - GL_TEXTURE0)
{
	this->_initialize();
}

rgle::Texture2D::Texture2D(const Texture2D & other)
{
	this->_format = other._format;
	this->_texture = other._texture;
	this->_initialize();
}

rgle::Texture2D::Texture2D(Texture2D && rvalue)
{
	this->_format = rvalue._format;
	this->_texture = rvalue._texture;
}

rgle::Texture2D::~Texture2D()
{
}

void rgle::Texture2D::operator=(const Texture2D& other)
{
	this->_format = other._format;
	this->_texture = other._texture;
	this->_initialize();
}

void rgle::Texture2D::operator=(Texture2D&& rvalue)
{
	this->_format = rvalue._format;
	this->_texture = rvalue._texture;
}

void rgle::Texture2D::update()
{
	glBindTexture(GL_TEXTURE_2D, this->id());
	glTexImage2D(GL_TEXTURE_2D,
		0,
		this->_format.internal,
		this->image()->width,
		this->image()->height,
		0,
		this->_format.target,
		GL_UNSIGNED_BYTE,
		this->image()->image
	);
}

void rgle::Texture2D::bind()
{
	glActiveTexture(this->_texture);
	glBindTexture(GL_TEXTURE_2D, this->id());
}

void rgle::Texture2D::_initialize()
{
	glBindTexture(GL_TEXTURE_2D, this->id());

	glTexImage2D(GL_TEXTURE_2D,
		0,
		this->_format.internal,
		this->image()->width,
		this->image()->height,
		0,
		this->_format.target,
		GL_UNSIGNED_BYTE,
		this->image()->image
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

rgle::PersistentTexture2D::PersistentTexture2D(std::shared_ptr<Image> image, int index, Format format, GLenum type, GLenum access) :
	_format(format),
	_type(type),
	_access(access),
	Texture(image, index)
{
	this->_initialize();
}

rgle::PersistentTexture2D::PersistentTexture2D(const PersistentTexture2D & other)
{
	this->_access = other._access;
	this->_format = other._format;
	this->_initialize();
}

rgle::PersistentTexture2D::PersistentTexture2D(PersistentTexture2D && rvalue)
{
	this->_access = rvalue._access;
	this->_format = rvalue._format;
}

rgle::PersistentTexture2D::~PersistentTexture2D()
{
}

void rgle::PersistentTexture2D::operator=(const PersistentTexture2D & other)
{
	this->_access = other._access;
	this->_format = other._format;
	this->_initialize();
}

void rgle::PersistentTexture2D::operator=(PersistentTexture2D && rvalue)
{
	this->_access = rvalue._access;
	this->_format = rvalue._format;
}

void rgle::PersistentTexture2D::update()
{
	glTextureSubImage2D(this->id(),
		0, 0, 0,
		this->image()->width,
		this->image()->height,
		this->_format.target,
		this->_type,
		this->image()->image
	);
}

void rgle::PersistentTexture2D::bind()
{
	glActiveTexture(GL_TEXTURE0 + this->index());
	glBindTexture(GL_TEXTURE_2D, this->id());
}

void rgle::PersistentTexture2D::bindImage2D()
{
	glBindImageTexture(this->index(), this->id(), 0, GL_FALSE, 0, this->_access, this->_format.internal);
}

GLenum & rgle::PersistentTexture2D::access()
{
	return this->_access;
}

const GLenum & rgle::PersistentTexture2D::access() const
{
	return this->_access;
}

void rgle::PersistentTexture2D::_initialize()
{
	glBindTexture(GL_TEXTURE_2D, this->id());
	glTexStorage2D(GL_TEXTURE_2D,
		1,
		this->_format.internal,
		this->image()->width,
		this->image()->height
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	this->update();
}
