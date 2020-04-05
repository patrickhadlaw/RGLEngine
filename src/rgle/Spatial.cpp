#include "rgle/Spatial.h"

const size_t rgle::SparseVoxelNodePayload::SIZE = rgle::aligned_std430_size(7 * sizeof(GLfloat) + sizeof(GLuint) + sizeof(GLint), 4 * sizeof(GLfloat));
const size_t rgle::SparseVoxelRayPayload::SIZE = rgle::aligned_std430_size(3 * sizeof(GLint) + 3 * sizeof(GLfloat), 3 * sizeof(GLfloat));
const size_t rgle::SparseVoxelOctree::BLOCK_SIZE = 8 * rgle::SparseVoxelNodePayload::SIZE;

const int rgle::SparseVoxelRenderer::OCTREE_BUFFER = 1;
const int rgle::SparseVoxelRenderer::PASS_READ_BUFFER = 2;
const int rgle::SparseVoxelRenderer::PASS_WRITE_BUFFER = 3;
const int rgle::SparseVoxelRenderer::META_BUFFER = 4;

rgle::SparseVoxelRenderer::SparseVoxelRenderer(
	std::string id,
	std::shared_ptr<SparseVoxelOctree> octree,
	std::string computeShaderId,
	std::string realizeShaderId,
	unsigned int width,
	unsigned int height,
	float fieldOfView) : _octree(octree), _resolution(glm::ivec2(width, height)), _fieldOfView(fieldOfView), RenderLayer(id)
{
	this->shader() = this->context().manager.shader.lock()->get(computeShaderId);
	if (this->shader().expired()) {
		throw NotFoundException("shader: '" + computeShaderId + "' not found", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_realizeShader = this->context().manager.shader.lock()->get(realizeShaderId);
	if (this->_realizeShader == nullptr) {
		throw NotFoundException("shader: '" + realizeShaderId + "' not found", LOGGER_DETAIL_IDENTIFIER(id));
	}
	auto shader = this->shaderLocked();
	this->_location.bootstrap = glGetUniformLocation(shader->programId(), "bootstrap");
	if (this->_location.bootstrap < 0) {
		throw GraphicsException("failed to locate shader uniform: 'bootstrap'", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_location.finalize = glGetUniformLocation(shader->programId(), "finalize");
	if (this->_location.finalize < 0) {
		throw GraphicsException("failed to locate shader uniform: 'finalize'", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_location.renderResolution = glGetUniformLocation(shader->programId(), "render_resolution");
	if (this->_location.renderResolution < 0) {
		throw GraphicsException("failed to locate shader uniform: 'render_resolution'", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_location.rootNodeOffset = glGetUniformLocation(shader->programId(), "root_node_offset");
	if (this->_location.rootNodeOffset < 0) {
		throw GraphicsException("failed to locate shader uniform: 'root_node_offset'", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_location.rootNodeSize = glGetUniformLocation(shader->programId(), "root_node_size");
	if (this->_location.rootNodeSize < 0) {
		throw GraphicsException("failed to locate shader uniform: 'root_node_size'", LOGGER_DETAIL_IDENTIFIER(id));
	}
	this->_orthoCamera = std::make_shared<Camera>(CameraType::ORTHOGONAL_PROJECTION, this->context().window.lock());
	this->_sparseVoxelCamera = std::make_shared<SparseVoxelCamera>(0.01f, 1000.0f, this->_fieldOfView);
	this->transformer() = this->_sparseVoxelCamera;
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
	this->_depthImage = Sampler2D(
		this->shader(),
		std::make_shared<PersistentTexture2D>(
			depthImage,
			0,
			PersistentTexture2D::Format{ GL_R32UI, GL_RED_INTEGER },
			GL_UNSIGNED_INT,
			GL_READ_WRITE
		),
		"depth_image"
	);
	this->_outImage = Sampler2D(
		this->shader(),
		std::make_shared<PersistentTexture2D>(
			outImage,
			1,
			PersistentTexture2D::Format{ GL_R32I, GL_RED_INTEGER },
			GL_INT,
			GL_WRITE_ONLY
		),
		"out_image"
	);

	this->_imageRect = ImageRect(Sampler2D(this->_realizeShader, this->_outImage.texture), 2.0f, 2.0f);

	glGenBuffers(1, &this->_passReadBuffer);
	glGenBuffers(1, &this->_passWriteBuffer);
	glGenBuffers(1, &this->_metaBuffer);
	this->_passAllocatedSize = this->_resolution.x * this->_resolution.y * 8;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->_passAllocatedSize * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passWriteBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->_passAllocatedSize * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
	this->_bootstrapData = std::malloc(this->_resolution.x * this->_resolution.y * SparseVoxelRayPayload::SIZE);
	SparseVoxelRayPayload payload;
	glm::vec2 pixelAngle(this->_fieldOfView / this->_resolution.x, this->_fieldOfView / this->_resolution.y);
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
	glDeleteBuffers(1, &this->_metaBuffer);
	std::free(this->_bootstrapData);
}

size_t rgle::SparseVoxelRenderer::swapBuffers()
{
	GLuint nextReadSize;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_metaBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), sizeof(GLuint), &nextReadSize);
	size_t payload[2] = { nextReadSize, 0 };
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLuint), payload);
	GLuint temp = this->_passReadBuffer;
	this->_passReadBuffer = this->_passWriteBuffer;
	return nextReadSize;
}

std::shared_ptr<rgle::SparseVoxelCamera>& rgle::SparseVoxelRenderer::camera()
{
	return this->_sparseVoxelCamera;
}

const std::shared_ptr<rgle::SparseVoxelCamera>& rgle::SparseVoxelRenderer::camera() const
{
	return this->_sparseVoxelCamera;
}

void rgle::SparseVoxelRenderer::update()
{
	this->_octree->flush();
}

void rgle::SparseVoxelRenderer::render()
{
	auto shader = this->shaderLocked();
	shader->use();
	this->bootstrap();
	this->transformer()->bind(shader);
	glUniform2ui(this->_location.renderResolution, this->_resolution.x, this->_resolution.y);
	this->_octree->bind();
	this->_depthImage.use();
	this->_outImage.use();
	glUniform1i(this->_location.finalize, false);
	size_t read_size = this->_resolution.x * this->_resolution.y;
	bool first = true;
	bool last = true;
	glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
	while (first) { // TODO: do more than first pass
		glUniform1i(this->_location.bootstrap, first);
		first = false;
		if (last) {
			this->finalize();
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_READ_BUFFER, this->_passReadBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PASS_WRITE_BUFFER, this->_passWriteBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, META_BUFFER, this->_metaBuffer);
		//size_t dim = std::ceil(std::sqrt((double) read_size / 1024.0));
		size_t dim = std::ceil((float)read_size / 1024.0f);
		glDispatchCompute(dim, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//read_size = this->swapBuffers();
		// TODO: check realloc
	}
	/*this->_realizeShader->use();
	this->_octree->bind();
	this->_orthoCamera->bind(this->_realizeShader);
	this->_imageRect.render();*/
	/*GLuint* tmp = new GLuint[this->_resolution.x * this->_resolution.y];
	Image8 img(this->_resolution.x, this->_resolution.y, 1);
	glGetTextureImage(this->_depthImage.texture->id(), 0, GL_RED, GL_UNSIGNED_INT, this->_resolution.x * this->_resolution.y * sizeof(GLuint), tmp);
	for (int i = 0; i < this->_resolution.x; i++) {
		for (int j = 0; j < this->_resolution.y; j++) {
			img.image[i + j * this->_resolution.x] = static_cast<unsigned char>((float)tmp[i + j * this->_resolution.x] / 100000.0f);
		}
	}
	std::free(tmp);
	img.write("tmp.png");*/
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
	else if (this->_passAllocatedSize * factor < std::max(this->_readPassSize, this->_writePassSize)) {
		throw GraphicsException("failed to reallocate sparse voxel pass buffers, reallocation too small", LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else {
		void* buffer = std::malloc(std::max(this->_readPassSize, this->_writePassSize) * SparseVoxelRayPayload::SIZE);
		size_t size = this->_passAllocatedSize * factor;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_readPassSize * SparseVoxelRayPayload::SIZE, buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_readPassSize * SparseVoxelRayPayload::SIZE, buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passWriteBuffer);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_writePassSize * SparseVoxelRayPayload::SIZE, buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * SparseVoxelRayPayload::SIZE, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_writePassSize * SparseVoxelRayPayload::SIZE, buffer);
		this->_passAllocatedSize = size;
		std::free(buffer);
	}
}

void rgle::SparseVoxelRenderer::bootstrap()
{
	// Restore the depth image
	this->_depthImage.texture->update();
	// Restore the output image
	this->_outImage.texture->update();
	glUniform1i(this->_location.rootNodeOffset, this->_octree->root()->index());
	glUniform1f(this->_location.rootNodeSize, this->_octree->root()->size());
	// Reset the read/write pass state
	// TODO: use a persistent map
	this->_readPassSize = this->_resolution.x * this->_resolution.y;
	this->_writePassSize = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_metaBuffer);
	GLuint meta[2] = { this->_readPassSize, this->_writePassSize };
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLuint), meta, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_passReadBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_readPassSize * SparseVoxelRayPayload::SIZE, this->_bootstrapData);
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
	_viewMatrix(1.0f)
{
}

rgle::SparseVoxelCamera::~SparseVoxelCamera()
{
}

void rgle::SparseVoxelCamera::bind(std::shared_ptr<ShaderProgram> program)
{
	glUniform3f(glGetUniformLocation(program->programId(), "camera_position"), this->_position.x, this->_position.y, this->_position.z);
	glUniform3f(glGetUniformLocation(program->programId(), "camera_direction"), this->_direction.x, this->_direction.y, this->_direction.z);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_near"), this->_near);
	glUniform1f(glGetUniformLocation(program->programId(), "camera_far"), this->_far);
	glUniform1f(glGetUniformLocation(program->programId(), "field_of_view"), this->_fieldOfView);
	glUniformMatrix3fv(glGetUniformLocation(program->programId(), "view"), 1, GL_FALSE, &this->_viewMatrix[0][0]);
}

void rgle::SparseVoxelCamera::translate(const glm::vec3 & by)
{
	this->_position += by;
}

rgle::SparseVoxelOctree::SparseVoxelOctree() : _top(0), _size(MIN_ALLOCATED)
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
	this->flush();
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
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_octreeBuffer);
	while (!this->_modifiedBlocks.empty()) {
		Range<size_t> flushRange = this->_modifiedBlocks.pop();
		glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, flushRange.lower * BLOCK_SIZE, flushRange.length() * BLOCK_SIZE);
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
	unsigned char* data = (unsigned char*)std::malloc(this->_size * BLOCK_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->_octreeBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->_size * BLOCK_SIZE, data);
	glInvalidateBufferData(GL_SHADER_STORAGE_BUFFER);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		newsize * BLOCK_SIZE,
		nullptr,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
	);
	this->_octreeData = (unsigned char*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_MAP_PERSISTENT_BIT);
	std::memcpy(this->_octreeData, data, this->_size * BLOCK_SIZE);
	glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, 0, this->_size * SparseVoxelNodePayload::SIZE);
	this->_size = newsize;
	this->_modifiedBlocks.clear();
	free(data);
}

unsigned char * rgle::SparseVoxelOctree::_buffer(const size_t & at)
{
	if (at >= this->_top) {
		throw OutOfBoundsException(LOGGER_DETAIL_DEFAULT);
	}
	return this->_octreeData + at * SparseVoxelNodePayload::SIZE;
}

void rgle::SparseVoxelRayPayload::mapToBuffer(unsigned char * buffer) const
{
	unsigned char* next = (unsigned char*)std::memcpy(buffer, &this->ray.x, 3 * sizeof(GLfloat));
	next = (unsigned char*)std::memcpy(next + 3 * sizeof(GLfloat), &this->pixel.x, 2 * sizeof(GLint));
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

void rgle::SparseVoxelNode::insertChildren(glm::vec4 colors[8])
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
	next = (unsigned char*)std::memcpy(next + 3 * sizeof(GLfloat), &this->depth, sizeof(GLuint));
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
