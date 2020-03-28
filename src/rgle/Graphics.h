#pragma once

#include "rgle/Color.h"

namespace rgle {

	class GraphicsException : public Exception {
	public:
		GraphicsException(std::string except, Logger::Detail& detail);
	};

	constexpr size_t aligned_buffer_size(const size_t& size);

	enum class ShaderModel {
		DEFAULT,
		INSTANCED
	};

	struct Image {

		enum class Format {
			JPEG,
			PNG
		};

		Image();
		Image(std::string imagefile);
		Image(int width, int height, int channels, size_t channelSize);
		Image(const Image& other);
		Image(Image&& rvalue);
		virtual ~Image();

		void operator=(const Image& other);
		void operator=(Image&& rvalue);

		void set(const size_t& x, const size_t& y, unsigned char* data, const size_t& size);

		void write(const std::string& imagefile) const;
		void write(const std::string& imagefile, const Format& format) const;

		unsigned char* image;
		int width;
		int height;
		int channels;
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

		struct Format {
			GLint internal;
			GLint target;
		};

		Texture();
		Texture(std::string imagefile, GLenum texture = GL_TEXTURE0, Format format = { GL_RGBA8, GL_RGBA }, GLenum type = GL_UNSIGNED_BYTE);
		Texture(std::shared_ptr<Image> image, GLenum texture = GL_TEXTURE0, Format format = { GL_RGBA8, GL_RGBA }, GLenum type = GL_UNSIGNED_BYTE);
		Texture(
			size_t width,
			size_t height,
			GLenum texture = GL_TEXTURE0,
			Format format = { GL_RGBA8, GL_RGBA },
			GLenum type = GL_UNSIGNED_BYTE
		);
		Texture(const Texture& other);
		Texture(Texture&& rvalue);
		virtual ~Texture();

		void operator=(const Texture& other);
		void operator=(Texture&& rvalue);

		void update();

		std::shared_ptr<Image> image;

		GLuint textureID;
		GLenum texture;
		GLenum type;

		size_t width;
		size_t height;

		Format format;

	private:
		void _generate();
	};

	struct Sampler2D {
		Sampler2D();
		Sampler2D(const Sampler2D& other);
		Sampler2D(
			std::weak_ptr<ShaderProgram> shader,
			std::string imagefile,
			GLenum texture = GL_TEXTURE0,
			Texture::Format format = { GL_RGBA8, GL_RGBA },
			std::string samplerUniform = "texture_0"
		);
		Sampler2D(
			std::weak_ptr<ShaderProgram> shader,
			std::shared_ptr<Texture> texture,
			std::string samplerUniform = "texture_0"
		);
		virtual ~Sampler2D();

		void operator=(const Sampler2D& other);

		void use();

		GLint samplerLocation;
		GLint enableLocation;

		bool enabled;

		std::shared_ptr<Texture> texture;

		std::weak_ptr<ShaderProgram> shader;
	protected:
		void _generate(const std::string& samplerUniform);
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
		Shape(std::string shaderid, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors);
		Shape(std::string shaderid, std::vector<glm::vec3> verticies, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Shape();

		virtual void render();

		virtual const char* typeName() const;

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);

	};

	class Triangle : public Shape {
	public:
		Triangle();
		Triangle(std::string shaderid, float a, float b, float theta, std::vector<glm::vec4> colors);
		Triangle(std::string shaderid, float a, float b, float theta, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Triangle();

	protected:
		void _construct(std::string& shaderid, float& a, float& b, float& theta, std::vector<glm::vec4> colors);
	};

	class Rect : public Shape {
	public:
		Rect();
		Rect(std::string shaderid, float width, float height, std::vector<glm::vec4> colors);
		Rect(std::string shaderid, float width, float height, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Rect();

		void changeDimensions(float width, float height);

	protected:
		void _construct(std::string& shaderid, float& width, float& height, std::vector<glm::vec4> colors);
	};

	class Torus : public Shape {
	public:
		Torus(std::string shaderid, float& c, float& a);
		virtual ~Torus();


	protected:
		void _construct(std::string shaderid, float& a, float& b, std::vector<glm::vec4> colors);
	};

	class ImageRect : public Shape {
	public:
		ImageRect();
		ImageRect(std::string shaderid, float width, float height, std::string image, ShaderModel shadermodel = ShaderModel::DEFAULT);
		ImageRect(Sampler2D sampler, float width, float height, ShaderModel shadermodel = ShaderModel::DEFAULT);
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

	class InstancedRenderer : public RenderLayer {
	public:
		InstancedRenderer(std::string id, std::shared_ptr<ViewTransformer> transformer, float allocationFactor = 5.0f, size_t minAllocated = 10);
		InstancedRenderer(const InstancedRenderer&) = delete;
		virtual ~InstancedRenderer();

		void operator=(const InstancedRenderer&) = delete;

		void addModel(std::string key, std::shared_ptr<Geometry3D> geometry, size_t payloadsize = 16 * sizeof(GLfloat));
		void setModelBindFunc(std::string key, std::function<void()> bindfunc);
		void setModelShader(std::string key, std::shared_ptr<ShaderProgram> shader);
		void removeModel(std::string key);

		size_t addInstance(std::string key, void* payload, size_t size);
		size_t addInstance(std::string key, glm::mat4 model = glm::mat4(1.0f));
		void updateInstance(size_t id, void* payload, size_t offset, size_t size);
		void removeInstance(size_t id);

		virtual void render();
		virtual void update();

		virtual const char* typeName() const;

	private:
		std::shared_ptr<ViewTransformer> _transformer;

		struct InstanceSet {
			std::weak_ptr<ShaderProgram> shader;
			std::function<void()> bindFunc;
			std::shared_ptr<Geometry3D> geometry;
			std::map<size_t, size_t> allocationMap;
			unsigned char* instanceData;
			size_t numInstances;
			size_t numAllocated;
			size_t payloadSize;
			GLuint ssbo;
		};
		std::unordered_map<std::string, InstanceSet> _setMap;
		std::map<size_t, std::string> _keyLookupTable;
		float _allocationFactor;
		size_t _minAllocated;

		static RGLE_DLLEXPORTED size_t _idCounter;
	};
}
