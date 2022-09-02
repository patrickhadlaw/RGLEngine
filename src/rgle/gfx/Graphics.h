#pragma once

#include "rgle/gfx/Image.h"

namespace rgle {

	class GraphicsException : public Exception {
	public:
		GraphicsException(std::string except, Logger::Detail detail);
	};

	// Gets the padded size of a uniform storage block
	// @remarks
	// Uniform storage blocks are rounded to the largest base alignment of any of its members,
	// rounded to the nearest sizeof(vec4)
	// @note that the exception to this rule is 3 component vectors (which are treated as 4
	// component vectors), avoid using them in uniform blocks
	size_t aligned_std140_size(const size_t& size, const size_t& largestMember);
	// Gets the padded size of a shader storage block
	// @remarks
	// Shader storage blocks are rounded to the largest base alignment of any of its members
	// @note that the exception to this rule is 3 component vectors (which are treated as 4
	// component vectors), avoid using them in storage blocks
	size_t aligned_std430_size(const size_t& size, const size_t& largestMember);

	enum class ShaderModel {
		DEFAULT,
		INSTANCED
	};

	struct Sampler2D {
		Sampler2D();
		Sampler2D(
			std::weak_ptr<ShaderProgram> shader,
			std::shared_ptr<Texture> texture,
			std::string samplerUniform = "texture_0"
		);
		Sampler2D(const Sampler2D& other);
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
		enum class TrianglePoint {
			A,
			B,
			C
		};

		Geometry3D();
		Geometry3D(const Geometry3D& other);
		Geometry3D(Geometry3D&& rvalue);
		virtual ~Geometry3D();

		void operator=(const Geometry3D& other);
		void operator=(Geometry3D&& rvalue);

		int triangleCount() const;
		const glm::vec3& triangleVertex(int faceIndex, TrianglePoint point) const;

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
		void updateInstance(size_t instanceId, void* payload, size_t offset, size_t size);
		void removeInstance(size_t instanceId);

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
