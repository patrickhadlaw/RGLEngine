#pragma once

#include "ShaderProgram.h"

namespace cppogl {

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
	
	typedef std::shared_ptr<Image> sImage;

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
		Texture(const sImage& image, GLenum texture = GL_TEXTURE0, Format format = { GL_RGBA8, GL_RGBA });
		virtual ~Texture();

		void operator=(const Texture& other);
		void operator=(Texture&& rvalue);

		sImage image;

		GLuint textureID;
		GLenum texture;

		Format format;

	private:
		void _generate();
	};

	typedef std::shared_ptr<Texture> sTexture;

	struct Sampler2D {
		Sampler2D();
		Sampler2D(const Sampler2D& other);
		Sampler2D(sShaderProgram shader, std::string imagefile, GLenum texture = GL_TEXTURE0, Texture::Format format = { GL_RGBA8, GL_RGBA });
		Sampler2D(sShaderProgram shader, const sTexture& texture);
		virtual ~Sampler2D();

		void operator=(const Sampler2D& other);

		void use();

		GLint samplerLocation;
		GLint enableLocation;

		bool enabled;

		sTexture texture;

		sShaderProgram shader;
	protected:
		void _generate();
	};

	typedef std::shared_ptr<Sampler2D> sSampler2D;

	class Geometry3D {
	public:
		struct Face {
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

		virtual void update();
		virtual void render();

		void generate();

		sShaderProgram shader;

		GLuint vertexArray;

		struct {
			std::vector<glm::vec3> list;
			GLint location;
			GLuint buffer;
		} vertex;
		struct {
			glm::mat4 matrix;
			GLint location;
		} model;
		struct {
			std::vector<unsigned short> list;
			GLuint buffer;
		} index;
		struct {
			std::vector<glm::vec4> list;
			GLint location;
			GLuint buffer;
		} color;
		struct {
			std::vector<glm::vec2> list;
			GLint location;
			GLuint buffer;
		} uv;
		std::vector<Sampler2D> samplers;

	protected:
		virtual void _cleanup();
	};

	typedef Geometry3D Material;

	// TODO: organize all geometry in Scene class allowing for operations on a scene, such as triangle sorting
	class Scene {
	public:
		Scene();
		virtual ~Scene();

		virtual void add(Geometry3D* geometry);
	private:
		std::vector<Geometry3D*> geometry;
	};

	// TODO: implement binary-space-partition with triangle sort and potential use for collision detection
	struct BSPNode {
		struct {
			Geometry3D* geometry;
			unsigned short index;
		} face;
		BSPNode* front = nullptr;
		BSPNode* back = nullptr;
	};

	class BSPTree {
	public:
		BSPTree();
		~BSPTree();
	};

	class Shape : public Geometry3D {
	public:
		Shape();
		Shape(sShaderProgram shader, std::vector<glm::vec3> verticies, std::vector<glm::vec4> colors);
		Shape(sShaderProgram shader, std::vector<glm::vec3> verticies, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Shape();

		virtual void render();

		virtual const std::string& typeName();

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);

	};

	class Triangle : public Shape {
	public:
		Triangle();
		Triangle(sShaderProgram shader, float a, float b, float theta, std::vector<glm::vec4> colors);
		Triangle(sShaderProgram shader, float a, float b, float theta, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Triangle();

	protected:
		void _construct(sShaderProgram& shader, float& a, float& b, float& theta, std::vector<glm::vec4> colors);
	};

	class Rect : public Shape {
	public:
		Rect();
		Rect(sShaderProgram shader, float width, float height, std::vector<glm::vec4> colors);
		Rect(sShaderProgram shader, float width, float height, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		virtual ~Rect();

	protected:
		void _construct(sShaderProgram& shader, float& width, float& height, std::vector<glm::vec4> colors);
	};

	class Torus : public Shape {
	public:
		Torus(sShaderProgram shader, float& a, float& b);


	protected:
		void _construct(sShaderProgram shader, float& a, float& b, std::vector<glm::vec4> colors);
	};

	class ImageRect : public Shape {
	public:
		ImageRect();
		ImageRect(sShaderProgram shader, float width, float height, std::string image);
		virtual ~ImageRect();

		void render();

	};

	class RenderLayer {
		// TODO: render layer with viewport per layer
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
