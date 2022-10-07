#include "rgle/gfx/Spatial.h"

const size_t rgle::gfx::SparseVoxelNodePayload::SIZE = rgle::gfx::aligned_std430_size(8 * sizeof(GLfloat) + sizeof(GLuint) + sizeof(GLint), 4 * sizeof(GLfloat));
const size_t rgle::gfx::SparseVoxelRayPayload::SIZE = rgle::gfx::aligned_std430_size(sizeof(GLuint) + sizeof(GLint), sizeof(GLint));
const size_t rgle::gfx::SparseVoxelOctree::BLOCK_SIZE = 8 * rgle::gfx::SparseVoxelNodePayload::SIZE;

const int rgle::gfx::SparseVoxelRenderer::RAY_BUFFER = 0;
const int rgle::gfx::SparseVoxelRenderer::OCTREE_BUFFER = 1;
const int rgle::gfx::SparseVoxelRenderer::PASS_READ_BUFFER = 2;
const int rgle::gfx::SparseVoxelRenderer::PASS_WRITE_BUFFER = 3;
const int rgle::gfx::SparseVoxelRenderer::PASS_WRTIE_COUNTER = 0;

rgle::gfx::SparseVoxelRenderer::SparseVoxelRenderer(
	std::string id,
	std::shared_ptr<SparseVoxelOctree> octree,
	std::string computeShaderId,
	std::string realizeShaderId,
	unsigned int width,
	unsigned int height,
	std::shared_ptr<SparseVoxelCamera> camera) :
	_octree(octree),
	_resolution(glm::ivec2(width, height)),
	_camera(camera),
	_maxBufferDepth(static_cast<size_t>(std::ceil(std::log2(_resolution.x)))),
	_lastTime(std::chrono::system_clock::now()),
	RenderLayer(id)
{
	this->shader() = this->context().manager.shader.lock()->getStrict(computeShaderId);
	this->_realizeShader = this->context().manager.shader.lock()->getStrict(realizeShaderId);
	auto shader = this->shaderLocked();
	this->_location.bootstrap = shader->uniformStrict("bootstrap");
	this->_location.finalize = shader->uniformStrict("finalize");
	this->_location.subPassOffset = shader->uniformStrict("subpass_offset");
	this->_location.subPassSize = shader->uniformStrict("subpass_size");
	this->_location.renderResolution = shader->uniformStrict("render_resolution");
	this->_location.rootNodeOffset = shader->uniformStrict("root_node_offset");
	this->_location.rootNodeSize = shader->uniformStrict("root_node_size");
	this->_location.readPassSize = shader->uniformStrict("read_pass_size");
	this->_location.depthImage = shader->uniformStrict("depth_image");
	this->_location.outImage = shader->uniformStrict("out_image");
	this->transformer() = this->_camera;
	auto depthImage = std::make_shared<Image>(this->_resolution.x, this->_resolution.y, 1, 1, sizeof(GLuint));
	auto outImage = std::make_shared<Image>(this->_resolution.x, this->_resolution.y, 1, 1, sizeof(GLint));
	const GLuint uintMax = std::numeric_limits<GLuint>::max();
	const GLint startIndex = -1;
	for (int i = 0; i < this->_resolution.x; i++) {
		for (int j = 0; j < this->_resolution.y; j++) {
			depthImage->set(i, j, (unsigned char*)&uintMax, sizeof(GLuint));
			outImage->set(i, j, (unsigned char*)&startIndex, sizeof(GLint));
		}
	}
	this->_depthTexture = std::make_shared<PersistentTexture2D>(
		depthImage,
		1,
		PersistentTexture2D::Format{ GL_R32UI, GL_RED_INTEGER },
		GL_UNSIGNED_INT,
		GL_READ_WRITE
	);
	this->_outTexture = std::make_shared<PersistentTexture2D>(
		outImage,
		0,
		PersistentTexture2D::Format{ GL_R32I, GL_RED_INTEGER },
		GL_INT,
		GL_WRITE_ONLY
	);

	this->_imageRect = ImageRect(Sampler2D(this->_realizeShader, this->_outTexture), 2.0f, 2.0f);
	this->_imageRect.model.matrix[3][2] = 0.0f;

	this->_passBuffers = std::make_unique<GLuint[]>(this->_maxBufferDepth);
	this->_bufferSizes = std::make_unique<unsigned int[]>(this->_maxBufferDepth);
	this->_counterBuffers = std::make_unique<GLuint[]>(this->_maxBufferDepth - 1);
	this->_subPassStack = std::make_unique<SubPass[]>(this->_maxBufferDepth);

	glGenBuffers(1, &this->_rayBuffer);
	glGenBuffers(static_cast<GLsizei>(this->_maxBufferDepth), &this->_passBuffers[0]);
	glGenBuffers(static_cast<GLsizei>(this->_maxBufferDepth - 1), &this->_counterBuffers[0]);

	std::vector<unsigned char> bootstrapData(this->_resolution.x * this->_resolution.y * SparseVoxelRayPayload::SIZE);
	std::vector<glm::vec4> rayData(this->_resolution.x * this->_resolution.y);
	SparseVoxelRayPayload payload;
	glm::vec2 pixelAngle(this->_camera->fieldOfView() / this->_resolution.x, this->_camera->fieldOfView() / this->_resolution.y);
	glm::ivec2 center(this->_resolution.x / 2, this->_resolution.y / 2);
	unsigned char* bootstrapPtr = bootstrapData.data();
	for (int j = 0; j < this->_resolution.y; j++) {
		for (int i = 0; i < this->_resolution.x; i++) {
			payload.pixel = i + j * this->_resolution.x;
			payload.offset = 0;
			payload.mapToBuffer(bootstrapPtr);
			bootstrapPtr += SparseVoxelRayPayload::SIZE;
			// Generate ray for pixel (i, j)
			glm::vec2 delta = glm::ivec2(i, j) - center;
			glm::vec2 theta(delta.x * pixelAngle.x, delta.y * pixelAngle.y);
			rayData[payload.pixel] = glm::normalize(glm::vec4(std::tanf(theta.x), std::tanf(theta.y), 1.0f, 0.0f));
		}
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bootstrapData.size(), bootstrapData.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_rayBuffer);
	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		this->_resolution.x * this->_resolution.y * 4 * sizeof(GLfloat),
		rayData.data(),
		GL_STATIC_DRAW
	);
	size_t subPassSize = this->_resolution.x * this->_resolution.y * 8;
	for (size_t i = 1; i < this->_maxBufferDepth; i++) {
		this->_bufferSizes[i] = static_cast<GLuint>(subPassSize * i);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passBuffers[i]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, this->_bufferSizes[i] * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, this->_counterBuffers[i - 1]);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	}
}

rgle::gfx::SparseVoxelRenderer::~SparseVoxelRenderer()
{
	glDeleteBuffers(1, &this->_rayBuffer);
	glDeleteBuffers(static_cast<GLsizei>(this->_maxBufferDepth), &this->_passBuffers[0]);
	glDeleteBuffers(static_cast<GLsizei>(this->_maxBufferDepth - 1), &this->_counterBuffers[0]);
}

std::shared_ptr<rgle::gfx::SparseVoxelCamera>& rgle::gfx::SparseVoxelRenderer::camera()
{
	return this->_camera;
}

const std::shared_ptr<rgle::gfx::SparseVoxelCamera>& rgle::gfx::SparseVoxelRenderer::camera() const
{
	return this->_camera;
}

void rgle::gfx::SparseVoxelRenderer::update()
{
	auto currentTime = std::chrono::system_clock::now();
	auto diff = currentTime - this->_lastTime;
	this->_octree->flush();
	this->_camera->update(diff.count() / 1000000.0f);
	this->_lastTime = currentTime;
}

void rgle::gfx::SparseVoxelRenderer::render()
{
	auto shader = this->shaderLocked();
	shader->use();
	size_t top = 0;
	GLuint subPassSize = this->_resolution.x * this->_resolution.y;
	this->_bootstrap(top);
	bool first = true;
	while (top > 0 || this->_subPassStack[top].offset < this->_subPassStack[top].count) {
		glUniform1i(this->_location.bootstrap, first);
		glUniform1i(this->_location.finalize, this->_lastSubPass(top));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, RAY_BUFFER, this->_rayBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_READ_BUFFER, this->_passBuffers[top]);
		if (this->_lastSubPass(top)) {
			subPassSize = this->_subPassStack[top].count - this->_subPassStack[top].offset;
			glUniform1i(this->_location.finalize, true);
			glUniform1ui(this->_location.subPassOffset, this->_subPassStack[top].offset);
			glUniform1ui(this->_location.subPassSize, subPassSize);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, PASS_WRTIE_COUNTER, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_WRITE_BUFFER, 0);
		}
		else {
			subPassSize = this->_subPassSize(top);
			glUniform1i(this->_location.finalize, false);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, PASS_WRTIE_COUNTER, this->_counterBuffers[top]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_WRITE_BUFFER, this->_passBuffers[top + 1]);
		}

		glUniform1ui(this->_location.readPassSize, this->_subPassStack[top].count);
		glUniform1ui(this->_location.subPassOffset, this->_subPassStack[top].offset);
		glUniform1ui(this->_location.subPassSize, subPassSize);

		glDispatchCompute(this->_numWorkGroups(subPassSize), 1, 1);
		this->_subPassStack[top].offset += subPassSize;

		if (this->_lastSubPass(top)) {
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			top = this->_unwindStack(top);
		}
		else {
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, this->_counterBuffers[top]);
			glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &this->_subPassStack[top + 1].count);
			if (this->_subPassStack[top + 1].count > 0) {
				top++;
			} else {
				top = this->_unwindStack(top);
			}
		}
		first = false;
	}
	this->_realizeShader->use();
	this->_octree->bind();
	this->_imageRect.render();
}

const char * rgle::gfx::SparseVoxelRenderer::typeName() const
{
	return "rgle::gfx::SparseVoxelRenderer";
}

void rgle::gfx::SparseVoxelRenderer::_bootstrap(const size_t& index)
{
	auto shader = this->shaderLocked();
	// Restore the depth image
	this->_depthTexture->update();
	// Restore the output image
	this->_outTexture->update();
	glUniform1i(this->_location.rootNodeOffset, static_cast<GLint>(this->_octree->root()->index()));
	glUniform1f(this->_location.rootNodeSize, this->_octree->root()->size());
	this->_clearCounter(this->_counterBuffers[index]);
	this->transformer()->bind(shader);
	glUniform2ui(
		this->_location.renderResolution,
		static_cast<GLuint>(this->_resolution.x),
		static_cast<GLuint>(this->_resolution.y)
	);
	this->_octree->bind();
	glUniform1i(this->_location.depthImage, this->_depthTexture->index());
	this->_depthTexture->bindImage2D();
	glUniform1i(this->_location.outImage, this->_outTexture->index());
	this->_outTexture->bindImage2D();
	glUniform1i(this->_location.finalize, false);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	for (size_t i = 0; i < this->_maxBufferDepth - 1; i++) {
		this->_subPassStack[index].offset = 0;
		this->_subPassStack[index].count = 0;
	}
	this->_subPassStack[index].offset = 0;
	this->_subPassStack[index].count = this->_resolution.x * this->_resolution.y;
}

void rgle::gfx::SparseVoxelRenderer::_clearCounter(const GLuint& buffer)
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer);
	const GLuint zero = 0;
	glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
}

void rgle::gfx::SparseVoxelRenderer::_setCounter(const GLuint& buffer, const GLuint& value)
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &value);
}

size_t rgle::gfx::SparseVoxelRenderer::_unwindStack(const size_t& top)
{
	size_t result = top;
	while (result > 0 && this->_subPassStack[result].offset >= this->_subPassStack[result].count) {
		this->_subPassStack[result].offset = 0;
		this->_subPassStack[result].count = 0;
		result--;
		this->_clearCounter(this->_counterBuffers[result]);
	}
	return result;
}

GLuint rgle::gfx::SparseVoxelRenderer::_numWorkGroups(const GLuint& subPassSize) const
{
	return static_cast<GLuint>(std::ceil((float)subPassSize / 1024.0f));
}

constexpr bool rgle::gfx::SparseVoxelRenderer::_lastSubPass(const size_t& top) const
{
	return top >= this->_maxBufferDepth - 2;
}

constexpr unsigned int rgle::gfx::SparseVoxelRenderer::_subPassSize(const size_t& top) const
{
	return std::min(
		this->_subPassStack[top].count - this->_subPassStack[top].offset,
		this->_bufferSizes[top + 1] / 8
	);
}

rgle::gfx::SparseVoxelCamera::SparseVoxelCamera(float near, float far, float fieldOfView) :
	_near(near),
	_far(far),
	_fieldOfView(fieldOfView),
	_position(0.0f, 0.0f, 0.0f),
	_direction(0.0f, 0.0f, 1.0f),
	_up(0.0f, 1.0f, 0.0f),
	_right(glm::cross(_up, _direction)),
	_rotation(1.0f, 0.0f, 0.0f, 0.0f)
{
}

rgle::gfx::SparseVoxelCamera::~SparseVoxelCamera()
{
}

void rgle::gfx::SparseVoxelCamera::update(float deltaT)
{
}

void rgle::gfx::SparseVoxelCamera::bind(std::shared_ptr<ShaderProgram> program)
{
	glUniform3f(glGetUniformLocation(program->programId(), "camera_position"), this->_position.x, this->_position.y, this->_position.z);
	glUniform3f(glGetUniformLocation(program->programId(), "camera_direction"), this->_direction.x, this->_direction.y, this->_direction.z);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_near"), this->_near);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_far"), this->_far);
	glUniform1f(glGetUniformLocation(program->programId(), "field_of_view"), this->_fieldOfView);
	glUniform4f(
		glGetUniformLocation(program->programId(), "rotation_quat"),
		this->_rotation.x,
		this->_rotation.y,
		this->_rotation.z,
		this->_rotation.w
	);
}

void rgle::gfx::SparseVoxelCamera::translate(const float& x, const float& y, const float& z)
{
	this->translate(glm::vec3(x, y, z));
}

void rgle::gfx::SparseVoxelCamera::translate(const glm::vec3 & by)
{
	this->_position += by;
}

void rgle::gfx::SparseVoxelCamera::rotate(const float& yaw, const float& pitch, const float& roll)
{
	this->_rotation = glm::normalize(
		glm::angleAxis(yaw, this->_up) *
		glm::angleAxis(pitch, this->_right) *
		glm::angleAxis(roll, this->_direction) *
		this->_rotation
	);
	this->_direction = glm::normalize(this->_rotation * glm::vec3(0.0f, 0.0f, 1.0f));
	this->_up = glm::normalize(this->_rotation * glm::vec3(0.0f, 1.0f, 0.0f));
	this->_right = glm::normalize(glm::cross(this->_up, this->_direction));
}

float & rgle::gfx::SparseVoxelCamera::fieldOfView()
{
	return this->_fieldOfView;
}

const float & rgle::gfx::SparseVoxelCamera::fieldOfView() const
{
	return this->_fieldOfView;
}

const glm::vec3 & rgle::gfx::SparseVoxelCamera::position() const
{
	return this->_position;
}

const glm::vec3 & rgle::gfx::SparseVoxelCamera::direction() const
{
	return this->_direction;
}

const glm::vec3 & rgle::gfx::SparseVoxelCamera::up() const
{
	return this->_up;
}

const glm::vec3 & rgle::gfx::SparseVoxelCamera::right() const
{
	return this->_right;
}

rgle::gfx::SparseVoxelOctree::SparseVoxelOctree() : _top(0), _size(MIN_ALLOCATED)
{
	glGenBuffers(1, &this->_octreeBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_octreeBuffer);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		this->_size * BLOCK_SIZE,
		nullptr,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
	);
	this->_octreeData = (unsigned char*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		this->_size * BLOCK_SIZE,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
	);
	if (this->_octreeData == nullptr) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	this->_top++;
	this->_root = new SparseVoxelNode(this);
	this->_root->_depth = 0;
	this->_root->_index = 0;
	this->_root->update();
}

rgle::gfx::SparseVoxelOctree::~SparseVoxelOctree()
{
	glDeleteBuffers(1, &this->_octreeBuffer);
}

void rgle::gfx::SparseVoxelOctree::bind() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SparseVoxelRenderer::OCTREE_BUFFER, this->_octreeBuffer);
}

rgle::gfx::SparseVoxelNode * rgle::gfx::SparseVoxelOctree::root() const
{
	return this->_root;
}

void rgle::gfx::SparseVoxelOctree::flush()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_octreeBuffer);
	while (!this->_modifiedBlocks.empty()) {
		util::Range<size_t> flushRange = this->_modifiedBlocks.pop();
		glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, flushRange.lower * BLOCK_SIZE, flushRange.length() * BLOCK_SIZE);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

const char * rgle::gfx::SparseVoxelOctree::typeName() const
{
	return "rgle::gfx::SparseVoxelOctree";
}

size_t rgle::gfx::SparseVoxelOctree::_aquireBlock()
{
	size_t result;
	if (this->_freeBlocks.empty()) {
		result = this->_top++;
		if (this->_top > this->_size) {
			this->_realloc(ALLOCATION_FACTOR);
		}
		return result;
	}
	else {
		result = this->_freeBlocks.back();
		this->_freeBlocks.pop_back();
		return result;
	}
}

void rgle::gfx::SparseVoxelOctree::_realloc(float factor)
{
	size_t newsize = std::max(MIN_ALLOCATED, (size_t) factor * this->_size);
	GLuint newbuffer;
	glGenBuffers(1, &newbuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, newbuffer);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		newsize * BLOCK_SIZE,
		nullptr,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
	);
	glBindBuffer(GL_COPY_READ_BUFFER, this->_octreeBuffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, newbuffer);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, this->_size * BLOCK_SIZE);
	glDeleteBuffers(1, &this->_octreeBuffer);
	this->_octreeBuffer = newbuffer;
	this->_octreeData = (unsigned char*)glMapNamedBufferRange(
		this->_octreeBuffer,
		0,
		newsize * BLOCK_SIZE,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
	);
	if (this->_octreeData == nullptr) {
		throw GraphicsException("failed to memory map octree buffer", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	this->_modifiedBlocks = util::CollectingQueue(util::Range<size_t>{ 0, this->_size });
	this->_size = newsize;
}

unsigned char * rgle::gfx::SparseVoxelOctree::_buffer(const size_t & at)
{
	if (at >= this->_top * 8) {
		throw OutOfBoundsException(LOGGER_DETAIL_DEFAULT);
	}
	return this->_octreeData + at * SparseVoxelNodePayload::SIZE;
}

void rgle::gfx::SparseVoxelRayPayload::mapToBuffer(unsigned char * buffer) const
{
	unsigned char* next = (unsigned char*)std::memcpy(buffer, &this->pixel, sizeof(GLuint));
	std::memcpy(next + sizeof(GLuint), &this->offset, sizeof(GLint));
}

rgle::gfx::SparseVoxelNode::SparseVoxelNode(SparseVoxelNode && rvalue)
{
	this->_children = rvalue._children;
	this->_color = rvalue._color;
	this->_depth = rvalue._depth;
	this->_index = rvalue._index;
	this->_octree = rvalue._octree;
	this->_parent = rvalue._parent;
	this->_position = rvalue._position;
	this->_size = rvalue._size;
}

rgle::gfx::SparseVoxelNode::~SparseVoxelNode()
{
}

void rgle::gfx::SparseVoxelNode::operator=(SparseVoxelNode && rvalue)
{
	std::swap(this->_children, rvalue._children);
	this->_color = rvalue._color;
	this->_depth = rvalue._depth;
	this->_index = rvalue._index;
	this->_octree = rvalue._octree;
	this->_parent = rvalue._parent;
	this->_position = rvalue._position;
	this->_size = rvalue._size;
}

rgle::gfx::SparseVoxelNode * rgle::gfx::SparseVoxelNode::child(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const
{
	if (this->_children == nullptr) {
		return nullptr;
	}
	return &this->_children[OctreeIndex::to_index(x, y, z)];
}

rgle::gfx::SparseVoxelNode * rgle::gfx::SparseVoxelNode::parent() const
{
	return this->_parent;
}

void rgle::gfx::SparseVoxelNode::insertChildren(std::array<glm::vec4, 8> colors)
{
	if (!this->leaf()) {
		throw InvalidStateException("failed to create octree children, they already exist", LOGGER_DETAIL_DEFAULT);
	}
	this->_children = new SparseVoxelNode[8];
	size_t block = this->_octree->_aquireBlock() * 8;
	OctreeIndex::X x;
	OctreeIndex::Y y;
	OctreeIndex::Z z;
	for (int i = 0; i < 8; i++) {
		this->_children[i] = SparseVoxelNode(this->_octree);
		this->_children[i]._color = colors[i];
		this->_children[i]._parent = this;
		this->_children[i]._depth = this->_depth + 1;
		this->_children[i]._size = this->_size / 2;
		OctreeIndex::from_index(i, x, y, z);
		this->_children[i]._position = this->_childPosition(x, y, z);
		this->_children[i]._index = block + i;
		this->_children[i].update();
	}
	this->_propagateChanges();
}

bool rgle::gfx::SparseVoxelNode::leaf() const
{
	return this->_children == nullptr;
}

bool rgle::gfx::SparseVoxelNode::root() const
{
	return this->_parent == nullptr;
}

size_t rgle::gfx::SparseVoxelNode::index() const
{
	return this->_index;
}

size_t rgle::gfx::SparseVoxelNode::depth() const
{
	return this->_depth;
}

float & rgle::gfx::SparseVoxelNode::size()
{
	return this->_size;
}

const float & rgle::gfx::SparseVoxelNode::size() const
{
	return this->_size;
}

glm::vec4 & rgle::gfx::SparseVoxelNode::color()
{
	return this->_color;
}

const glm::vec4 & rgle::gfx::SparseVoxelNode::color() const
{
	return this->_color;
}

void rgle::gfx::SparseVoxelNode::update()
{
	unsigned char* buffer = this->_octree->_buffer(this->_index);
	SparseVoxelNodePayload payload = this->toPayload();
	payload.mapToBuffer(buffer);
	this->_octree->_modifiedBlocks.push(this->_index / 8);
}

rgle::gfx::SparseVoxelNodePayload rgle::gfx::SparseVoxelNode::toPayload() const
{
	SparseVoxelNodePayload payload;
	payload.color = this->_color;
	payload.depth = static_cast<GLuint>(this->_depth);
	payload.next = this->leaf() ? -1 : static_cast<GLint>(this->_children[0]._index);
	payload.position = this->_position;
	return payload;
}

rgle::gfx::SparseVoxelNode::SparseVoxelNode() :
	_index(0),
	_depth(0),
	_color(0.0f, 0.0f, 0.0f, 0.0f),
	_position(0.0f, 0.0f, 0.0f),
	_size(0.0f),
	_children(nullptr),
	_octree(nullptr)
{
}

rgle::gfx::SparseVoxelNode::SparseVoxelNode(SparseVoxelOctree * octree) :
	_index(0),
	_depth(0),
	_color(0.0f, 0.0f, 0.0f, 0.0f),
	_position(0.0f, 0.0f, 0.0f),
	_size(0.0f),
	_children(nullptr),
	_parent(nullptr),
	_octree(octree)
{
}

glm::vec3 rgle::gfx::SparseVoxelNode::_childPosition(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const
{
	glm::vec3 result = this->_position;
	float quarter = this->_size / 4;
	result.x += x == OctreeIndex::RIGHT ? quarter : -quarter;
	result.y += y == OctreeIndex::TOP ? quarter : -quarter;
	result.z += z == OctreeIndex::FRONT ? quarter : -quarter;
	return result;
}

void rgle::gfx::SparseVoxelNode::_propagateChanges()
{
	if (!this->leaf()) {
		glm::vec3 hsv = glm::vec3();
		float divisor = 0.0f;
		for (size_t i = 0; i < 8; i++) {
			auto rgba = util::RGBA(this->_children[i]._color);
			hsv = hsv + util::Color(rgba).into<util::HSV>().vector * rgba.a;
			divisor += rgba.a;
		}
		this->_color = glm::vec4(util::Color(util::HSV(hsv)).into<util::RGB>().vector / divisor, divisor / 8);
		this->update();
	}
	if (!this->root()) {
		this->_parent->_propagateChanges();
	}
}

void rgle::gfx::SparseVoxelNodePayload::mapToBuffer(unsigned char * buffer) const
{
	unsigned char* next = (unsigned char*)std::memcpy(buffer, &this->color.x, 4 * sizeof(GLfloat));
	next = (unsigned char*)std::memcpy(next + 4 * sizeof(GLfloat), &this->position.x, 3 * sizeof(GLfloat));
	// NOTE: offset by vec4 to avoid using vec3 in SSBO (vec3's are difficult to work with in interface blocks)
	next = (unsigned char*)std::memcpy(next + 4 * sizeof(GLfloat), &this->depth, sizeof(GLuint));
	std::memcpy(next + sizeof(GLuint), &this->next, sizeof(GLint));
}

size_t rgle::gfx::OctreeIndex::to_index(X x, Y y, Z z)
{
	return x + 2 * y + 4 * z;
}

void rgle::gfx::OctreeIndex::from_index(const size_t & index, X & x, Y & y, Z & z)
{
	x = index % 2 == 0 ? X::LEFT : X::RIGHT;
	y = index % 4 < 2 ? Y::BOTTOM : Y::TOP;
	z = index < 4 ? Z::BACK : Z::FRONT;
}

rgle::gfx::NoClipSparseVoxelCamera::NoClipSparseVoxelCamera(float near, float far, float fieldOfView, std::shared_ptr<Window> window) :
	_window(window),
	SparseVoxelCamera(near, far, fieldOfView)
{
	if (this->_window.expired()) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	window->registerListener("mousemove", this);
}

void rgle::gfx::NoClipSparseVoxelCamera::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "mousemove") {
		MouseMoveMessage* mousemove = dynamic_cast<MouseMoveMessage*>(message);
		this->_mouse.deltaX = mousemove->mouse.x;
		this->_mouse.deltaY = mousemove->mouse.y;
	}
}

void rgle::gfx::NoClipSparseVoxelCamera::update(float deltaT)
{
	auto window = this->_window.lock();
	if (window->grabbed()) {
		int state = window->getKey(GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			this->unGrab();
		}
		else {
			if (this->_mouse.deltaX != 0.0 || this->_mouse.deltaY != 0.0) {
				this->rotate(deltaT * ((float)this->_mouse.deltaX / window->height()), deltaT * ((float)this->_mouse.deltaY / window->width()), 0.0);
				this->_mouse.deltaX = 0.0;
				this->_mouse.deltaY = 0.0;
			}
			float move = deltaT * this->_speed;
			if (window->getKey(GLFW_KEY_W) == GLFW_PRESS) {
				this->move(move, 0.0, 0.0);
			}
			if (window->getKey(GLFW_KEY_A) == GLFW_PRESS) {
				this->move(0.0, -move, 0.0);
			}
			if (window->getKey(GLFW_KEY_S) == GLFW_PRESS) {
				this->move(-move, 0.0, 0.0);
			}
			if (window->getKey(GLFW_KEY_D) == GLFW_PRESS) {
				this->move(0.0, move, 0.0);
			}
			if (window->getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
				this->move(0.0, 0.0, move);
			}
			if (window->getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
				this->move(0.0, 0.0, -move);
			}
			if (window->getKey(GLFW_KEY_E) == GLFW_PRESS) {
				this->rotate(0.0, 0.0, -move);
			}
			if (window->getKey(GLFW_KEY_Q) == GLFW_PRESS) {
				this->rotate(0.0, 0.0, move);
			}
		}
	}
	SparseVoxelCamera::update(deltaT);
}

void rgle::gfx::NoClipSparseVoxelCamera::grab()
{
	this->_window.lock()->grabCursor();
}

void rgle::gfx::NoClipSparseVoxelCamera::unGrab()
{
	this->_window.lock()->ungrabCursor();
}

void rgle::gfx::NoClipSparseVoxelCamera::move(float forward, float horizontal, float vertical)
{
	this->translate(this->direction() * forward + this->right() * horizontal + this->up() * vertical);
}
