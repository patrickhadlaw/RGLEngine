#pragma once

#include "ShaderProgram.h"

namespace cppogl {

	struct Image {
		Image();
		Image(std::string imagefile);
		Image(Image&& rvalue);
		~Image();

		void operator=(const Image& other);

		unsigned char* image;
		int width;
		int height;
		int bpp;
	};

	struct Sampler2D : public Image {
		Sampler2D();
		Sampler2D(sShaderProgram shader, std::string imagefile, GLenum texture = GL_TEXTURE0);
		Sampler2D(const Sampler2D& other);
		Sampler2D(Sampler2D&& rvalue);
		~Sampler2D();

		void operator=(const Sampler2D& other);

		void use();
		void disable();

		GLuint textureID;
		GLenum texture;
		GLint samplerLocation;
		GLint enableLocation;

		sShaderProgram shader;
	protected:
		void _generate();
	};

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
		~Geometry3D();

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
		std::vector<unsigned short> indicies;
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
	};

	typedef Geometry3D Material;

	// TODO: organize all geometry in Scene class allowing for operations on a scene, such as triangle sorting
	class Scene {
	public:
		Scene();
		~Scene();

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
		~Shape();

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
		~Triangle();

	protected:
		void _construct(sShaderProgram& shader, float& a, float& b, float& theta, std::vector<glm::vec4> colors);
	};

	class Rect : public Shape {
	public:
		Rect();
		Rect(sShaderProgram shader, float width, float height, std::vector<glm::vec4> colors);
		Rect(sShaderProgram shader, float width, float height, glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0));
		~Rect();

	protected:
		void _construct(sShaderProgram& shader, float& width, float& height, std::vector<glm::vec4> colors);
	};

	class ImageRect : public Shape {
	public:
		ImageRect();
		ImageRect(sShaderProgram shader, float width, float height, std::string image);
		~ImageRect();

		void render();

	};


	// TODO: use ASSIMP to load common 3D formats
	std::vector<Material> loadModel(std::string file);

	class Model {
	public:
		Model();
		Model(std::string file);
		~Model();

		virtual void render();
		virtual void update();

		std::vector<Material> materials;
	};
}