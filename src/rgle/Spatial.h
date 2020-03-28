#pragma once

#include "rgle/Graphics.h"

#include <deque>

namespace rgle {

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

		void insertChildren(glm::vec4 colors[8]);

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
		CollectingQueue<size_t> _modifiedBlocks;

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
		GLint offset;
		glm::ivec2 pixel;
		glm::vec3 ray;

		void mapToBuffer(unsigned char* buffer) const;

		static const size_t SIZE;
	};

	class SparseVoxelCamera : public ViewTransformer {
	public:
		SparseVoxelCamera(float near, float far, float fieldOfView);
		virtual ~SparseVoxelCamera();

		virtual void bind(std::shared_ptr<ShaderProgram> program);

		void translate(const glm::vec3& by);

	private:
		glm::vec3 _position;
		glm::vec3 _direction;
		glm::vec3 _up;
		float _fieldOfView;
		float _near;
		float _far;
		glm::mat3 _viewMatrix;
	};

	class SparseVoxelRenderer : public RenderLayer {
	public:
		static const int OCTREE_BUFFER;
		static const int PASS_READ_BUFFER;
		static const int PASS_WRITE_BUFFER;
		static const int META_BUFFER;

		SparseVoxelRenderer(
			std::string id,
			std::shared_ptr<SparseVoxelOctree> octree,
			std::string computeShaderId,
			std::string realizeShaderId,
			unsigned int width,
			unsigned int height,
			float fieldOfView
		);
		SparseVoxelRenderer(const SparseVoxelRenderer&) = delete;
		virtual ~SparseVoxelRenderer();

		void operator=(const SparseVoxelRenderer&) = delete;

		void reallocPassBuffers(float factor);

		void bootstrap();
		void finalize();

		size_t swapBuffers();

		std::shared_ptr<SparseVoxelCamera>& camera();
		const std::shared_ptr<SparseVoxelCamera>& camera() const;

		virtual void update();
		virtual void render();

		virtual const char* typeName() const;

	private:
		GLuint _passReadBuffer;
		GLuint _passWriteBuffer;
		GLuint _metaBuffer;
		void* _bootstrapData;
		glm::ivec2 _numWorkGroups;
		size_t _readPassSize;
		size_t _writePassSize;
		size_t _passAllocatedSize;
		uint32_t _index;
		glm::ivec2 _resolution;
		float _fieldOfView;
		struct {
			GLint rootNodeOffset;
			GLint rootNodeSize;
			GLint bootstrap;
			GLint finalize;
		} _location;
		Sampler2D _depthImage;
		Sampler2D _outImage;
		ImageRect _imageRect;
		std::shared_ptr<Camera> _orthoCamera;
		std::shared_ptr<ShaderProgram> _realizeShader;
		std::shared_ptr<SparseVoxelCamera> _sparseVoxelCamera;
		std::shared_ptr<SparseVoxelOctree> _octree;
	};
}