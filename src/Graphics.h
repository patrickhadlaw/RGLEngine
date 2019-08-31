#pragma once

#include "Color.h"

namespace rgle {

	class GraphicsException : public Exception {
	public:
		GraphicsException();
		GraphicsException(std::string except, Logger::Detail& detail);
		virtual ~GraphicsException();

	protected:
		std::string _type();
	};

	struct Image {
		Image();
		Image(std::string imagefile);
		Image(const Image& other);
		Image(Image&& rvalue);
		virtual ~Image();

		void operator=(const Image& other);

		unsigned char* image;
		int width;
		int height;
		int bpp;
	};
	
	class Texture {
	public:

		struct Format {
			GLint internal;
			GLint target;
		};

		Texture();
		Texture(std::string imagefile, GLenum texture = GL_TEXTURE0, Format format = { GL_RGBA8, GL_RGBA });
		Texture(const Texture& other);
		Texture(Texture&& rvalue);
		Texture(const std::shared_ptr<Image>& image, GLenum texture = GL_TEXTURE0, Format format = { GL_RGBA8, GL_RGBA });
		virtual ~Texture();

		void operator=(const Texture& other);
		void operator=(Texture&& rvalue);

		std::shared_ptr<Image> image;

		GLuint textureID;
		GLenum texture;

		Format format;

	private:
		void _generate();
	};

	struct Sampler2D {
		Sampler2D();
		Sampler2D(const Sampler2D& other);
		Sampler2D(std::shared_ptr<ShaderProgram> shader, std::string imagefile, GLenum texture = GL_TEXTURE0, Texture::Format format = { GL_RGBA8, GL_RGBA });
		Sampler2D(std::shared_ptr<ShaderProgram> shader, const std::shared_ptr<Texture>& texture);
		virtual ~Sampler2D();

		void operator=(const Sampler2D& other);

		void use();

		GLint samplerLocation;
		GLint enableLocation;

		bool enabled;

		std::shared_ptr<Texture> texture;

		std::shared_ptr<ShaderProgram> shader;
	protected:
		void _generate();
	};

	class Geometry3D {
	public:
		class Face {
		public:
			Face(glm::vec3& p1, unsigned short& i1, glm::vec3& p2, unsigned short& i2, glm::vec3& p3, unsigned short& i3);

			glm::vec3& p1;
			unsigned short& i1;
			glm::vec3& p2;
			unsigned short& i2;
			glm::vec3& p3;
			unsigned short& i3;
		};
		Geometry3D();
		Geometry3D(const Geometry3D& other);
		Geometry3D(Geometry3D&& rvalue);
		virtual ~Geometry3D();

		void operator=(const Geometry3D& other);
		void operator=(Geometry3D&& rvalue);

		int numFaces();
		Face getFace(int index);

		virtual void generate();

		void standardRender(std::shared_ptr<ShaderProgram> shader);

		void standardFill(Fill colorFill);

		void updateVertexBuffer();
		void updateIndexBuffer();
		void updateColorBuffer();
		void updateUVBuffer();

		GLuint vertexArray;

		struct {
			std::vector<glm::vec3> list;
			GLint location = 0;
			GLuint buffer;
		} vertex;
		struct {
			glm::mat4 matrix;
			GLint location = 0;
			bool enabled = true;
		} model;
		struct {
			std::vector<unsigned short> list;
			GLuint buffer;
		} index;
		struct {
			std::vector<glm::vec4> list;
			GLint location = 0;
			GLuint buffer;
		} color;
		struct {
			std::vector<glm::vec2> list;
			GLint location = 0;
			GLuint buffer;
		} uv;
		std::vector<Sampler2D> samplers;

	protected:
		virtual void _cleanup();
	};

	typedef Geometry3D Material;

	class SceneLayer : public RenderLayer {
	public:
		SceneLayer(std::string id);
		virtual ~SceneLayer();

		virtual void add(std::shared_ptr<Geometry3D> geometry);
	private:
		std::vector<std::shared_ptr<Geometry3D>> _renderables;
	};
	typedef std::shared_ptr<SceneLayer> sSceneLayer;

	class Shape : public Geometry3D, public Renderable {
	public:
		Shape();
		Shape(Context context, std::string shader, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors);
		Shape(Context context, std::string shader, std::vector<glm::vec3> verticies, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Shape();

		virtual void render();

		virtual std::string& typeName();

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);

	};

	class Triangle : public Shape {
	public:
		Triangle();
		Triangle(Context context, std::string shader, float a, float b, float theta, std::vector<glm::vec4> colors);
		Triangle(Context context, std::string shader, float a, float b, float theta, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Triangle();

	protected:
		void _construct(Context& context, std::string& shader, float& a, float& b, float& theta, std::vector<glm::vec4> colors);
	};

	class Rect : public Shape {
	public:
		Rect();
		Rect(Context context, std::string shader, float width, float height, std::vector<glm::vec4> colors);
		Rect(Context context, std::string shader, float width, float height, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Rect();

		void changeDimensions(float width, float height);

	protected:
		void _construct(Context& context, std::string& shader, float& width, float& height, std::vector<glm::vec4> colors);
	};

	class Torus : public Shape {
	public:
		Torus(Context context, std::string shader, float& c, float& a);
		virtual ~Torus();


	protected:
		void _construct(Context context, std::string shader, float& a, float& b, std::vector<glm::vec4> colors);
	};

	class ImageRect : public Shape {
	public:
		ImageRect();
		ImageRect(Context context, std::string shader, float width, float height, std::string image);
		virtual ~ImageRect();

		void render();

	};

	// TODO: use ASSIMP to load common 3D formats
	std::vector<Material> loadModel(std::string file);

	class Model {
	public:
		Model();
		Model(std::string file);
		virtual ~Model();

		virtual void render();
		virtual void update();

		std::vector<Material> materials;
	};
}
