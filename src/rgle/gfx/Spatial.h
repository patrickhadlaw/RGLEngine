#pragma once

#include "rgle/gfx/Graphics.h"

namespace rgle::gfx {

	namespace OctreeIndex {
		enum X {
			LEFT,
			RIGHT
		};
		enum Y {
			TOP,
			BOTTOM
		};
		enum Z {
			FRONT,
			BACK
		};
		
		size_t to_index(X x, Y y, Z z);
		void from_index(const size_t& index, X& x, Y& y, Z& z);
	}

	class SparseVoxelOctree;

	struct SparseVoxelNodePayload {
		glm::vec4 color;
		glm::vec3 position;
		GLuint depth;
		GLint next;

		void mapToBuffer(unsigned char* buffer) const;

		static const size_t SIZE;
	};

	class SparseVoxelNode {
		friend class SparseVoxelOctree;
	public:
		SparseVoxelNode(const SparseVoxelNode&) = delete;
		SparseVoxelNode(SparseVoxelNode&& rvalue);
		~SparseVoxelNode();

		void operator=(const SparseVoxelNode&) = delete;
		void operator=(SparseVoxelNode&& rvalue);

		SparseVoxelNode* child(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const;
		SparseVoxelNode* parent() const;

		void insertChildren(std::array<glm::vec4, 8> colors);

		bool leaf() const;
		bool root() const;

		size_t index() const;
		size_t depth() const;

		float& size();
		const float& size() const;

		glm::vec4& color();
		const glm::vec4& color() const;

		void update();

		SparseVoxelNodePayload toPayload() const;

	private:
		SparseVoxelNode();
		SparseVoxelNode(SparseVoxelOctree* octree);

		glm::vec3 _childPosition(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const;
		void _propagateChanges();

		glm::vec4 _color;
		glm::vec3 _position;
		float _size;
		size_t _index;
		size_t _depth;
		SparseVoxelNode* _parent;
		SparseVoxelNode* _children;
		SparseVoxelOctree* _octree;
	};

	class SparseVoxelOctree : public Node {
		friend class SparseVoxelNode;
	public:
		static const size_t BLOCK_SIZE;

		SparseVoxelOctree();
		SparseVoxelOctree(const SparseVoxelOctree&) = delete;
		SparseVoxelOctree(SparseVoxelOctree&&) = delete;
		virtual ~SparseVoxelOctree();

		void operator=(const SparseVoxelOctree&) = delete;
		void operator=(SparseVoxelOctree&&) = delete;

		void bind() const;

		SparseVoxelNode* root() const;

		void flush();

		virtual const char* typeName() const;

	private:
		const size_t MIN_ALLOCATED = 10;
		const float ALLOCATION_FACTOR = 10.0f;

		size_t _aquireBlock();
		void _realloc(float factor);
		unsigned char* _buffer(const size_t& at);

		// Root of octree
		SparseVoxelNode* _root;

		// Queue of free blocks of 8
		std::deque<size_t> _freeBlocks;

		// Queue storing the modified blocks (used for buffer flush)
		util::CollectingQueue<size_t> _modifiedBlocks;

		// The top index of buffer
		size_t _top;

		// Allocated size of buffer in # of nodes
		size_t _size;

		// Persistent buffer storage
		GLuint _octreeBuffer;

		// Mapped pointer to octree buffer
		unsigned char* _octreeData;
	};

	struct SparseVoxelRayPayload {
		GLuint pixel;
		GLint offset;

		void mapToBuffer(unsigned char* buffer) const;

		static const size_t SIZE;
	};

	class SparseVoxelCamera : public ViewTransformer {
	public:
		SparseVoxelCamera(float near, float far, float fieldOfView);
		virtual ~SparseVoxelCamera();

		virtual void update(float deltaT);
		virtual void bind(std::shared_ptr<ShaderProgram> program);

		void translate(const float& x, const float& y, const float& z);
		void translate(const glm::vec3& by);
		void rotate(const float& yaw, const float& pitch, const float& roll);

		float& fieldOfView();
		const float& fieldOfView() const;

		const glm::vec3& position() const;
		const glm::vec3& direction() const;
		const glm::vec3& up() const;
		const glm::vec3& right() const;

	private:
		glm::vec3 _position;
		glm::vec3 _direction;
		glm::vec3 _up;
		glm::vec3 _right;
		glm::quat _rotation;
		float _fieldOfView;
		float _near;
		float _far;
	};

	class NoClipSparseVoxelCamera : public SparseVoxelCamera, public EventListener {
	public:
		NoClipSparseVoxelCamera(float near, float far, float fieldOfView, std::shared_ptr<Window> window);

		virtual void onMessage(std::string eventname, EventMessage* message);

		virtual void update(float deltaT);

		virtual void grab();
		virtual void unGrab();

		void move(float forward, float horizontal, float vertical);

	private:
		struct {
			double deltaX = 0.0f;
			double deltaY = 0.0f;
		} _mouse;
		float _speed = 0.1f;
		std::weak_ptr<Window> _window;
	};

	// A renderer utilizing a pass based traversal through a GPU octree
	// @todo use the smallest possible octree root to avoid unessesary passes
	class SparseVoxelRenderer : public RenderLayer {
	public:
		static const int RAY_BUFFER;
		static const int OCTREE_BUFFER;
		static const int PASS_READ_BUFFER;
		static const int PASS_WRITE_BUFFER;
		static const int PASS_WRTIE_COUNTER;

		SparseVoxelRenderer(
			std::string id,
			std::shared_ptr<SparseVoxelOctree> octree,
			std::string computeShaderId,
			std::string realizeShaderId,
			unsigned int width,
			unsigned int height,
			std::shared_ptr<SparseVoxelCamera> camera
		);
		SparseVoxelRenderer(const SparseVoxelRenderer&) = delete;
		virtual ~SparseVoxelRenderer();

		void operator=(const SparseVoxelRenderer&) = delete;

		std::shared_ptr<SparseVoxelCamera>& camera();
		const std::shared_ptr<SparseVoxelCamera>& camera() const;

		virtual void update();
		virtual void render();

		virtual const char* typeName() const;

	private:

		struct SubPass {
			unsigned int offset;
			unsigned int count;
		};
		void _bootstrap(const size_t& index);

		void _clearCounter(const GLuint& buffer);
		void _setCounter(const GLuint& buffer, const GLuint& value);
		size_t _unwindStack(const size_t& top);

		GLuint _numWorkGroups(const GLuint& subPassSize) const;
		constexpr bool _lastSubPass(const size_t& top) const;
		constexpr unsigned int _subPassSize(const size_t& top) const;
		
		GLuint _rayBuffer;
		std::unique_ptr<GLuint[]> _passBuffers;
		std::unique_ptr<unsigned int[]> _bufferSizes;
		std::unique_ptr<GLuint[]> _counterBuffers;
		std::unique_ptr<SubPass[]> _subPassStack;
		glm::ivec2 _resolution;
		size_t _maxBufferDepth;
		std::chrono::system_clock::time_point _lastTime;

		std::shared_ptr<PersistentTexture2D> _depthTexture;
		std::shared_ptr<PersistentTexture2D> _outTexture;
		ImageRect _imageRect;

		std::shared_ptr<ShaderProgram> _realizeShader;
		std::shared_ptr<SparseVoxelCamera> _camera;
		std::shared_ptr<SparseVoxelOctree> _octree;

		struct {
			GLint rootNodeOffset;
			GLint rootNodeSize;
			GLint renderResolution;
			GLint bootstrap;
			GLint finalize;
			GLint subPassOffset;
			GLint subPassSize;
			GLint readPassSize;
			GLint depthImage;
			GLint outImage;
		} _location;
	};
}