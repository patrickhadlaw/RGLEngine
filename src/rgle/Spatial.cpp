#include "rgle/Spatial.h"

const size_t rgle::SparseVoxelNodePayload::SIZE = rgle::aligned_std430_size(8 * sizeof(GLfloat) + sizeof(GLuint) + sizeof(GLint), 4 * sizeof(GLfloat));
const size_t rgle::SparseVoxelRayPayload::SIZE = rgle::aligned_std430_size(3 * sizeof(GLint) + 4 * sizeof(GLfloat), 4 * sizeof(GLfloat));
const size_t rgle::SparseVoxelOctree::BLOCK_SIZE = 8 * rgle::SparseVoxelNodePayload::SIZE;

const int rgle::SparseVoxelRenderer::OCTREE_BUFFER = 1;
const int rgle::SparseVoxelRenderer::PASS_READ_BUFFER = 2;
const int rgle::SparseVoxelRenderer::PASS_WRITE_BUFFER = 3;
const int rgle::SparseVoxelRenderer::PASS_WRTIE_COUNTER = 0;

rgle::SparseVoxelRenderer::SparseVoxelRenderer(
	std::string id,
	std::shared_ptr<SparseVoxelOctree> octree,
	std::string computeShaderId,
	std::string realizeShaderId,
	unsigned int width,
	unsigned int height,
	std::shared_ptr<SparseVoxelCamera> camera,
	size_t maxPassesPerFrame) :
	_octree(octree),
	_resolution(glm::ivec2(width, height)),
	_camera(camera),
	_maxPassesPerFrame(maxPassesPerFrame),
	_lastTime(std::chrono::system_clock::now()),
	RenderLayer(id)
{
	this->shader() = this->context().manager.shader.lock()->getStrict(computeShaderId);
	this->_realizeShader = this->context().manager.shader.lock()->getStrict(realizeShaderId);
	auto shader = this->shaderLocked();
	this->_location.bootstrap = shader->uniformStrict("bootstrap");
	this->_location.finalize = shader->uniformStrict("finalize");
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

	glGenBuffers(1, &this->_passReadBuffer);
	glGenBuffers(1, &this->_passWriteBuffer);
	glGenBuffers(1, &this->_writeCounterBuffer);
	this->_passAllocatedSize = this->_resolution.x * this->_resolution.y * 8;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->_passAllocatedSize * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passWriteBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->_passAllocatedSize * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, this->_writeCounterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	this->_bootstrapData = std::malloc(this->_resolution.x * this->_resolution.y * SparseVoxelRayPayload::SIZE);
	SparseVoxelRayPayload payload;
	glm::vec2 pixelAngle(this->_camera->fieldOfView() / this->_resolution.x, this->_camera->fieldOfView() / this->_resolution.y);
	glm::ivec2 center(this->_resolution.x / 2, this->_resolution.y / 2);
	unsigned char* ptr = (unsigned char*)this->_bootstrapData;
	for (int i = 0; i < this->_resolution.x; i++) {
		for (int j = 0; j < this->_resolution.y; j++) {
			payload.pixel = glm::ivec2(i, j);
			payload.offset = 0;
			glm::vec2 delta = payload.pixel - center;
			glm::vec2 theta(delta.x * pixelAngle.x, delta.y * pixelAngle.y);
			payload.ray = glm::vec3(std::tanf(theta.x), std::tanf(theta.y), 1.0f);
			payload.ray = glm::normalize(payload.ray);
			payload.mapToBuffer(ptr);
			ptr += SparseVoxelRayPayload::SIZE;
		}
	}
}

rgle::SparseVoxelRenderer::~SparseVoxelRenderer()
{
	glDeleteBuffers(1, &this->_passReadBuffer);
	glDeleteBuffers(1, &this->_passWriteBuffer);
	glDeleteBuffers(1, &this->_writeCounterBuffer);
	std::free(this->_bootstrapData);
}

size_t rgle::SparseVoxelRenderer::swapBuffers()
{
	GLuint nextReadSize;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, this->_writeCounterBuffer);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextReadSize);
	const GLuint zero = 0;
	glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
	GLuint temp = this->_passReadBuffer;
	this->_passReadBuffer = this->_passWriteBuffer;
	this->_passWriteBuffer = temp;
	if (nextReadSize * 8 >= this->_passAllocatedSize) {
		this->reallocPassBuffers(8.0f);
	}
	return nextReadSize;
}

std::shared_ptr<rgle::SparseVoxelCamera>& rgle::SparseVoxelRenderer::camera()
{
	return this->_camera;
}

const std::shared_ptr<rgle::SparseVoxelCamera>& rgle::SparseVoxelRenderer::camera() const
{
	return this->_camera;
}

void rgle::SparseVoxelRenderer::update()
{
	auto currentTime = std::chrono::system_clock::now();
	auto diff = currentTime - this->_lastTime;
	this->_octree->flush();
	this->_camera->update(diff.count() / 1000000.0f);
	this->_lastTime = currentTime;
}

void rgle::SparseVoxelRenderer::render()
{
	auto shader = this->shaderLocked();
	shader->use();
	this->bootstrap();
	this->transformer()->bind(shader);
	glUniform2ui(this->_location.renderResolution, this->_resolution.x, this->_resolution.y);
	this->_octree->bind();
	glUniform1i(this->_location.depthImage, this->_depthTexture->index());
	this->_depthTexture->bindImage2D();
	glUniform1i(this->_location.outImage, this->_outTexture->index());
	this->_outTexture->bindImage2D();
	glUniform1i(this->_location.finalize, false);
	bool first = true;
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	int i = 0;
	while (i < this->_maxPassesPerFrame && this->_currentPassSize > 0) {
		glUniform1i(this->_location.bootstrap, first);
		glUniform1ui(this->_location.readPassSize, this->_currentPassSize);
		first = false;
		if (i == this->_maxPassesPerFrame - 1) {
			this->finalize();
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_READ_BUFFER, this->_passReadBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_WRITE_BUFFER, this->_passWriteBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, PASS_WRTIE_COUNTER, this->_writeCounterBuffer);
		size_t dim = std::ceil((float)this->_currentPassSize / 1024.0f);
		glDispatchCompute(dim, 1, 1);
		if (i != this->_maxPassesPerFrame - 1) {
			this->_currentPassSize = this->swapBuffers();
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
		i++;
	}
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	this->_realizeShader->use();
	this->_octree->bind();
	this->_imageRect.render();
}

const char * rgle::SparseVoxelRenderer::typeName() const
{
	return "rgle::SparseVoxelRenderer";
}

void rgle::SparseVoxelRenderer::reallocPassBuffers(float factor)
{
	if (factor <= 0.0f) {
		throw GraphicsException("invalid sparse voxel allocation factor: " + std::to_string(factor) + " expected factor greater than zero", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else if (this->_passAllocatedSize * factor < this->_currentPassSize) {
		throw GraphicsException("failed to reallocate sparse voxel pass buffers, reallocation too small", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else {
		size_t size = this->_passAllocatedSize * factor;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passWriteBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_COPY_READ_BUFFER, this->_passReadBuffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, this->_passWriteBuffer);
		glCopyBufferSubData(
			GL_COPY_READ_BUFFER,
			GL_COPY_WRITE_BUFFER,
			0, 0,
			this->_currentPassSize * SparseVoxelNodePayload::SIZE
		);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
		this->_passAllocatedSize = size;
		GLuint temp = this->_passReadBuffer;
		this->_passReadBuffer = this->_passWriteBuffer;
		this->_passWriteBuffer = temp;
	}
}

void rgle::SparseVoxelRenderer::bootstrap()
{
	// Restore the depth image
	this->_depthTexture->update();
	// Restore the output image
	this->_outTexture->update();
	glUniform1i(this->_location.rootNodeOffset, this->_octree->root()->index());
	glUniform1f(this->_location.rootNodeSize, this->_octree->root()->size());
	// Reset the read/write pass state
	this->_currentPassSize = this->_resolution.x * this->_resolution.y;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, this->_writeCounterBuffer);
	GLuint zero = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_currentPassSize * SparseVoxelRayPayload::SIZE, this->_bootstrapData);
}

void rgle::SparseVoxelRenderer::finalize()
{
	glUniform1i(this->_location.finalize, true);
}

rgle::SparseVoxelCamera::SparseVoxelCamera(float near, float far, float fieldOfView) :
	_near(near),
	_far(far),
	_fieldOfView(fieldOfView),
	_position(0.0f, 0.0f, 0.0f),
	_direction(0.0f, 0.0f, 1.0f),
	_up(glm::vec3(0.0f, 1.0f, 0.0f)),
	_right(glm::cross(_up, _direction)),
	_view(_updatedView())
{
}

rgle::SparseVoxelCamera::~SparseVoxelCamera()
{
}

void rgle::SparseVoxelCamera::update(float deltaT)
{
	this->_view = this->_updatedView();
}

void rgle::SparseVoxelCamera::bind(std::shared_ptr<ShaderProgram> program)
{
	glUniform3f(glGetUniformLocation(program->programId(), "camera_position"), this->_position.x, this->_position.y, this->_position.z);
	glUniform3f(glGetUniformLocation(program->programId(), "camera_direction"), this->_direction.x, this->_direction.y, this->_direction.z);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_near"), this->_near);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_far"), this->_far);
	glUniform1f(glGetUniformLocation(program->programId(), "field_of_view"), this->_fieldOfView);
	glUniformMatrix3fv(glGetUniformLocation(program->programId(), "view"), 1, GL_FALSE, &this->_view[0][0]);
}

void rgle::SparseVoxelCamera::translate(const float& x, const float& y, const float& z)
{
	this->translate(glm::vec3(x, y, z));
}

void rgle::SparseVoxelCamera::translate(const glm::vec3 & by)
{
	this->_position += by;
}

void rgle::SparseVoxelCamera::rotate(const float& yaw, const float& pitch, const float& roll)
{
	glm::vec3 direction = this->_direction;
	this->_direction = glm::normalize(
		glm::angleAxis(yaw, this->_up) *
		glm::angleAxis(pitch, this->_right) *
		glm::angleAxis(roll, direction)
	) * this->_direction;
	this->_up = glm::normalize(
		glm::angleAxis(yaw, this->_up) *
		glm::angleAxis(pitch, this->_right) *
		glm::angleAxis(roll, direction)
	) * this->_up;
	this->_right = glm::cross(this->_up, this->_direction);
}

float & rgle::SparseVoxelCamera::fieldOfView()
{
	return this->_fieldOfView;
}

const float & rgle::SparseVoxelCamera::fieldOfView() const
{
	return this->_fieldOfView;
}

const glm::vec3 & rgle::SparseVoxelCamera::position() const
{
	return this->_position;
}

const glm::vec3 & rgle::SparseVoxelCamera::direction() const
{
	return this->_direction;
}

const glm::vec3 & rgle::SparseVoxelCamera::up() const
{
	return this->_up;
}

const glm::vec3 & rgle::SparseVoxelCamera::right() const
{
	return this->_right;
}

const glm::mat3 rgle::SparseVoxelCamera::_updatedView() const
{
	return glm::lookAt(glm::vec3(0.0f), -this->_direction, -this->_up);
}

rgle::SparseVoxelOctree::SparseVoxelOctree() : _top(0), _size(MIN_ALLOCATED)
{
	glGenBuffers(1, &this->_octreeBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_octreeBuffer);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		this->_size * BLOCK_SIZE,
		nullptr,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT
	);
	this->_octreeData = (unsigned char*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		this->_size * BLOCK_SIZE,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT//| GL_MAP_FLUSH_EXPLICIT_BIT
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

rgle::SparseVoxelOctree::~SparseVoxelOctree()
{
	glDeleteBuffers(1, &this->_octreeBuffer);
}

void rgle::SparseVoxelOctree::bind() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SparseVoxelRenderer::OCTREE_BUFFER, this->_octreeBuffer);
}

rgle::SparseVoxelNode * rgle::SparseVoxelOctree::root() const
{
	return this->_root;
}

void rgle::SparseVoxelOctree::flush()
{
	while (!this->_modifiedBlocks.empty()) {
		Range<size_t> flushRange = this->_modifiedBlocks.pop();
		//glFlushMappedNamedBufferRange(this->_octreeBuffer, flushRange.lower * BLOCK_SIZE, flushRange.length() * BLOCK_SIZE);
	}
}

const char * rgle::SparseVoxelOctree::typeName() const
{
	return "rgle::SparseVoxelOctree";
}

size_t rgle::SparseVoxelOctree::_aquireBlock()
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

void rgle::SparseVoxelOctree::_realloc(float factor)
{
	size_t newsize = std::max(MIN_ALLOCATED, (size_t) factor * this->_size);
	GLuint newbuffer;
	glGenBuffers(1, &newbuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, newbuffer);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		newsize * BLOCK_SIZE,
		nullptr,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT
	);
	glBindBuffer(GL_COPY_READ_BUFFER, this->_octreeBuffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, newbuffer);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, this->_size * BLOCK_SIZE);
	glDeleteBuffers(1, &this->_octreeBuffer);
	this->_octreeBuffer = newbuffer;
	this->_octreeData = (unsigned char*)glMapNamedBufferRange(
		this->_octreeBuffer,
		0,
		this->_size * BLOCK_SIZE,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT//| GL_MAP_FLUSH_EXPLICIT_BIT
	);
	if (this->_octreeData == nullptr) {
		throw GraphicsException("failed to memory map octree buffer", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	this->_modifiedBlocks = CollectingQueue(Range<size_t>{ 0, this->_size });
	this->_size = newsize;
}

unsigned char * rgle::SparseVoxelOctree::_buffer(const size_t & at)
{
	if (at >= this->_top * 8) {
		throw OutOfBoundsException(LOGGER_DETAIL_DEFAULT);
	}
	return this->_octreeData + at * SparseVoxelNodePayload::SIZE;
}

void rgle::SparseVoxelRayPayload::mapToBuffer(unsigned char * buffer) const
{
	unsigned char* next = (unsigned char*)std::memcpy(buffer, &this->ray.x, 3 * sizeof(GLfloat));
	// NOTE: offset by vec4 to avoid using vec3 in SSBO (vec3's are difficult to work with in interface blocks)
	next = (unsigned char*)std::memcpy(next + 4 * sizeof(GLfloat), &this->pixel.x, 2 * sizeof(GLint));
	std::memcpy(next + 2 * sizeof(GLint), &this->offset, sizeof(GLint));
}

rgle::SparseVoxelNode::SparseVoxelNode(SparseVoxelNode && rvalue)
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

rgle::SparseVoxelNode::~SparseVoxelNode()
{
}

void rgle::SparseVoxelNode::operator=(SparseVoxelNode && rvalue)
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

rgle::SparseVoxelNode * rgle::SparseVoxelNode::child(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const
{
	if (this->_children == nullptr) {
		return nullptr;
	}
	return &this->_children[OctreeIndex::to_index(x, y, z)];
}

rgle::SparseVoxelNode * rgle::SparseVoxelNode::parent() const
{
	return this->_parent;
}

void rgle::SparseVoxelNode::insertChildren(std::array<glm::vec4, 8> colors)
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

bool rgle::SparseVoxelNode::leaf() const
{
	return this->_children == nullptr;
}

bool rgle::SparseVoxelNode::root() const
{
	return this->_parent == nullptr;
}

size_t rgle::SparseVoxelNode::index() const
{
	return this->_index;
}

size_t rgle::SparseVoxelNode::depth() const
{
	return this->_depth;
}

float & rgle::SparseVoxelNode::size()
{
	return this->_size;
}

const float & rgle::SparseVoxelNode::size() const
{
	return this->_size;
}

glm::vec4 & rgle::SparseVoxelNode::color()
{
	return this->_color;
}

const glm::vec4 & rgle::SparseVoxelNode::color() const
{
	return this->_color;
}

void rgle::SparseVoxelNode::update()
{
	unsigned char* buffer = this->_octree->_buffer(this->_index);
	SparseVoxelNodePayload payload = this->toPayload();
	payload.mapToBuffer(buffer);
	this->_octree->_modifiedBlocks.push(this->_index / 8);
}

rgle::SparseVoxelNodePayload rgle::SparseVoxelNode::toPayload() const
{
	SparseVoxelNodePayload payload;
	payload.color = this->_color;
	payload.depth = this->_depth;
	payload.next = this->leaf() ? -1 : this->_children[0]._index;
	payload.position = this->_position;
	return payload;
}

rgle::SparseVoxelNode::SparseVoxelNode() :
	_index(0),
	_depth(0),
	_color(0.0f, 0.0f, 0.0f, 0.0f),
	_position(0.0f, 0.0f, 0.0f),
	_size(0.0f),
	_children(nullptr),
	_octree(nullptr)
{
}

rgle::SparseVoxelNode::SparseVoxelNode(SparseVoxelOctree * octree) :
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

glm::vec3 rgle::SparseVoxelNode::_childPosition(OctreeIndex::X x, OctreeIndex::Y y, OctreeIndex::Z z) const
{
	glm::vec3 result = this->_position;
	float quarter = this->_size / 4;
	result.x += x == OctreeIndex::RIGHT ? quarter : -quarter;
	result.y += y == OctreeIndex::TOP ? quarter : -quarter;
	result.z += z == OctreeIndex::FRONT ? quarter : -quarter;
	return result;
}

void rgle::SparseVoxelNode::_propagateChanges()
{
	if (!this->leaf()) {
		this->_color = Color::blend({
			this->_children[0]._color,
			this->_children[1]._color,
			this->_children[2]._color,
			this->_children[3]._color,
			this->_children[4]._color,
			this->_children[5]._color,
			this->_children[6]._color,
			this->_children[7]._color
		});
		this->update();
	}
	if (!this->root()) {
		this->_parent->_propagateChanges();
	}
}

void rgle::SparseVoxelNodePayload::mapToBuffer(unsigned char * buffer) const
{
	unsigned char* next = (unsigned char*)std::memcpy(buffer, &this->color.x, 4 * sizeof(GLfloat));
	next = (unsigned char*)std::memcpy(next + 4 * sizeof(GLfloat), &this->position.x, 3 * sizeof(GLfloat));
	// NOTE: offset by vec4 to avoid using vec3 in SSBO (vec3's are difficult to work with in interface blocks)
	next = (unsigned char*)std::memcpy(next + 4 * sizeof(GLfloat), &this->depth, sizeof(GLuint));
	std::memcpy(next + sizeof(GLuint), &this->next, sizeof(GLint));
}

size_t rgle::OctreeIndex::to_index(X x, Y y, Z z)
{
	return x + 2 * y + 4 * z;
}

void rgle::OctreeIndex::from_index(const size_t & index, X & x, Y & y, Z & z)
{
	x = index % 2 == 0 ? X::LEFT : X::RIGHT;
	y = index % 4 < 2 ? Y::BOTTOM : Y::TOP;
	z = index < 4 ? Z::BACK : Z::FRONT;
}

rgle::NoClipSparseVoxelCamera::NoClipSparseVoxelCamera(float near, float far, float fieldOfView, std::shared_ptr<Window> window) :
	_window(window),
	SparseVoxelCamera(near, far, fieldOfView)
{
	if (this->_window.expired()) {
		throw NullPointerException(LOGGER_DETAIL_DEFAULT);
	}
	window->registerListener("mousemove", this);
}

void rgle::NoClipSparseVoxelCamera::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "mousemove") {
		MouseMoveMessage* mousemove = dynamic_cast<MouseMoveMessage*>(message);
		this->_mouse.deltaX = mousemove->mouse.x;
		this->_mouse.deltaY = mousemove->mouse.y;
	}
}

void rgle::NoClipSparseVoxelCamera::update(float deltaT)
{
	auto window = this->_window.lock();
	if (window->grabbed()) {
		int state = window->getKey(GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS) {
			this->unGrab();
		}
		else {
			if (this->_mouse.deltaX != 0.0 || this->_mouse.deltaY != 0.0) {
				this->rotate(-deltaT * ((float)this->_mouse.deltaX / window->height()), -deltaT * ((float)this->_mouse.deltaY / window->width()), 0.0);
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
				this->rotate(0.0, 0.0, move);
			}
			if (window->getKey(GLFW_KEY_Q) == GLFW_PRESS) {
				this->rotate(0.0, 0.0, -move);
			}
		}
	}
	SparseVoxelCamera::update(deltaT);
}

void rgle::NoClipSparseVoxelCamera::grab()
{
	this->_window.lock()->grabCursor();
}

void rgle::NoClipSparseVoxelCamera::unGrab()
{
	this->_window.lock()->ungrabCursor();
}

void rgle::NoClipSparseVoxelCamera::move(float forward, float horizontal, float vertical)
{
	this->translate(this->direction() * forward + this->right() * horizontal + this->up() * vertical);
}
