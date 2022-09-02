#pragma once

#include "rgle/util/Color.h"

namespace rgle {

	struct Image {
		enum class Format {
			JPEG,
			PNG
		};

		Image();
		Image(std::string imagefile);
		Image(size_t width, size_t height, size_t depth, size_t channels, size_t channelSize);
		Image(const Image& other);
		Image(Image&& rvalue);
		virtual ~Image();

		void operator=(const Image& other);
		void operator=(Image&& rvalue);

		void set(const size_t& x, const size_t& y, unsigned char* data, const size_t& size);

		void write(const std::string& imagefile) const;
		void write(const std::string& imagefile, const Format& format) const;

		size_t size() const;

		unsigned char* image;
		size_t width;
		size_t height;
		size_t depth;
		size_t channels;
		size_t channelSize;
	};

	class Image8 : public Image {
	public:
		Image8();
		Image8(std::string imagefile);
		Image8(int width, int height, int channels);

		void set(const size_t& x, const size_t& y, const float& intensity);
		void set(const size_t& x, const size_t& y, const glm::vec2& ia);
		void set(const size_t& x, const size_t& y, const glm::vec3& rgb);
		void set(const size_t& x, const size_t& y, const glm::vec4& rgba);

	};

	class Texture {
	public:
		Texture();
		Texture(std::shared_ptr<Image> image, int index = 0);
		Texture(const Texture& other);
		Texture(Texture&& rvalue);
		virtual ~Texture();

		void operator=(const Texture& other);
		void operator=(Texture&& rvalue);

		virtual void update();
		virtual void bind();

		GLuint& id();
		const GLuint& id() const;

		std::shared_ptr<Image>& image();
		const std::shared_ptr<Image>& image() const;

		int& index();
		const int& index() const;

	private:
		GLuint _id;
		int _index;
		std::shared_ptr<Image> _image;
	};

	class Texture2D : public Texture {
	public:
		struct Format {
			GLint internal;
			GLint target;
		};
		Texture2D(
			std::string imagefile,
			GLenum texture = GL_TEXTURE0,
			Format format = { GL_RGBA8, GL_RGBA }
		);
		Texture2D(
			std::shared_ptr<Image> image,
			GLenum texture = GL_TEXTURE0,
			Format format = { GL_RGBA8, GL_RGBA }
		);
		Texture2D(const Texture2D& other);
		Texture2D(Texture2D&& rvalue);
		virtual ~Texture2D();

		void operator=(const Texture2D& other);
		void operator=(Texture2D&& rvalue);

		virtual void update();
		virtual void bind();

	private:
		void _initialize();

		GLenum _texture;
		Format _format;
	};

	class PersistentTexture2D : public Texture {
	public:
		struct Format {
			GLint internal;
			GLint target;
		};
		PersistentTexture2D(std::shared_ptr<Image> image, int index, Format format, GLenum type, GLenum access = GL_READ_WRITE);
		PersistentTexture2D(const PersistentTexture2D& other);
		PersistentTexture2D(PersistentTexture2D&& rvalue);
		virtual ~PersistentTexture2D();

		void operator=(const PersistentTexture2D& other);
		void operator=(PersistentTexture2D&& rvalue);

		virtual void update();
		virtual void bind();

		virtual void bindImage2D();

		GLenum& access();
		const GLenum& access() const;

	private:
		void _initialize();

		Format _format;
		GLenum _type;
		GLenum _access;
	};
}