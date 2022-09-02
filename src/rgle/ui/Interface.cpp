#include "rgle/ui/Interface.h"
#include "rgle/ui/Text.h"

rgle::ui::BoundingBox::BoundingBox()
{
}

rgle::ui::BoundingBox::BoundingBox(UnitVector2D dimensions, UnitVector2D topleft)
{
}

rgle::ui::BoundingBox::~BoundingBox()
{
}

void rgle::ui::BoundingBox::onBoxUpdate()
{
}

rgle::UnitVector2D rgle::ui::BoundingBox::getTopLeft()
{
	return _topLeft;
}

rgle::UnitVector2D rgle::ui::BoundingBox::getDimensions()
{
	return _dimensions;
}

void rgle::ui::BoundingBox::changeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::ui::BoundingBox::changeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::ui::BoundingBox::change(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::ui::BoundingBox::quietChangeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	this->onBoxUpdate();
}

void rgle::ui::BoundingBox::quietChangeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

void rgle::ui::BoundingBox::quietChange(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

rgle::ui::LayoutChangeMessage::LayoutChangeMessage()
{
}

rgle::ui::LayoutChangeMessage::~LayoutChangeMessage()
{
}

rgle::ui::RelativeAligner::RelativeAligner()
{
}

rgle::ui::RelativeAligner::RelativeAligner(std::shared_ptr<BoundingBox> element, std::shared_ptr<BoundingBox> align, RelativeAlignerAttributes attributes)
{
	this->_element = element;
	this->_align = align;
	this->_attributes = attributes;
	this->_element->registerListener("bounding-box", this);
	this->realign();
}

rgle::ui::RelativeAligner::~RelativeAligner()
{
}

void rgle::ui::RelativeAligner::realign()
{
	UnitVector2D newTopLeft = UnitVector2D{};
	switch (_attributes.direction) {
	case Aligner::Direction::BOTTOM:
		newTopLeft.x = _element->getTopLeft().x;
		newTopLeft.y = _element->getTopLeft().y + _element->getDimensions().y + _attributes.spacing;
		break;
	case Aligner::Direction::LEFT:
		newTopLeft.y = _element->getTopLeft().y;
		newTopLeft.x = _element->getTopLeft().x + _element->getDimensions().x + _attributes.spacing;
		break;
	case Aligner::Direction::RIGHT:
		newTopLeft.y = _element->getTopLeft().y;
		newTopLeft.x = _element->getTopLeft().x - _align->getDimensions().x - _attributes.spacing;
		break;
	case Aligner::Direction::TOP:
		newTopLeft.x = _element->getTopLeft().x;
		newTopLeft.y = _element->getTopLeft().y - _align->getDimensions().y - _attributes.spacing;
		break;
	}
	_align->changeTopLeft(newTopLeft);
}

rgle::ui::LinearAligner::LinearAligner()
{
}

rgle::ui::LinearAligner::LinearAligner(std::vector<std::shared_ptr<BoundingBox>> elements, LinearAlignerAttributes attributes)
{
	this->_elements = elements;
	this->_attributes = attributes;
	if (!this->_elements.empty()) {
		UnitVector2D newTopLeft = this->_attributes.topLeft;
		this->_elements.front()->changeTopLeft(newTopLeft);
	}
	RelativeAlignerAttributes attribs;
	attribs.center = this->_attributes.center;
	attribs.direction = this->_attributes.direction;
	attribs.spacing = this->_attributes.spacing;
	for (size_t i = 1; i < this->_elements.size(); i++) {
		this->_aligners.push_back(RelativeAligner(this->_elements[i - 1], this->_elements[i], attribs));
	}
}

rgle::ui::LinearAligner::~LinearAligner()
{
}

void rgle::ui::LinearAligner::push(std::shared_ptr<BoundingBox> element)
{
}

std::shared_ptr<rgle::ui::BoundingBox> rgle::ui::LinearAligner::pop()
{
	this->_aligners.pop_back();
	std::shared_ptr<BoundingBox> bbox = this->_elements.back();
	this->_elements.pop_back();
	return bbox;
}

void rgle::ui::LinearAligner::realign()
{
	for (size_t i = 0; i < this->_aligners.size(); i++) {
		this->_aligners[i].realign();
	}
}

rgle::ui::Element::Element()
{
}

rgle::ui::Element::Element(ElementAttributes attributes)
{
	this->_elementAttributes = attributes;
}

rgle::ui::Element::~Element()
{
}

void rgle::ui::Element::onMessage(std::string eventname, EventMessage *)
{
	if (eventname == "resize") {
		this->onBoxUpdate();
	}
}

bool rgle::ui::Element::raycast(Ray ray)
{
	auto window = this->context().window.lock();
	glm::vec3 topleft = glm::vec3(_topLeft.resolve(window), 0.0);
	glm::vec2 dimensions = _dimensions.resolve(window);
	glm::vec3 topright = topleft;
	topright.x += dimensions.x;
	glm::vec3 bottomleft = topleft;
	bottomleft.y += dimensions.y;
	glm::vec3 bottomright = topleft;
	bottomright.x += dimensions.x;
	bottomright.y += dimensions.y;

	return ray.intersect(topright, topleft, bottomleft) || ray.intersect(bottomleft, bottomright, topright);
}

rgle::ui::DelegateMouseState rgle::ui::Element::delegateMouseState(Ray clickray, bool, MouseState)
{
	return DelegateMouseState::UNCHANGED;
}

glm::mat4 rgle::ui::Element::transform()
{
	glm::mat4 transform = glm::mat4(1.0f);
	auto window = this->context().window.lock();
	transform[3][0] = _topLeft.x.resolve(window, Window::X);
	transform[3][1] = -_topLeft.y.resolve(window, Window::Y);
	transform[3][2] = _elementAttributes.zIndex;
	return transform;
}

rgle::ui::ElementAttributes rgle::ui::Element::getElementAttribs()
{
	return _elementAttributes;
}

const char * rgle::ui::Element::typeName() const
{
	return "rgle::ui::Element";
}

rgle::ui::Aligner::Aligner()
{
}

rgle::ui::Aligner::~Aligner()
{
}

void rgle::ui::Aligner::onMessage(std::string eventname, EventMessage *)
{
	if (eventname == "bounding-box") {
		this->realign();
	}
}

void rgle::ui::Aligner::realign()
{
}

rgle::ui::Layer::Layer(std::string id, float ticktime) : 
	RenderLayer(id),
	_tickTime(ticktime),
	_lastTick(0),
	_raycastCheck(false),
	_castHit(false)
{
	this->transformer() = std::make_shared<gfx::Camera>(gfx::CameraType::ORTHOGONAL_PROJECTION, this->context().window.lock());
	if (ticktime <= 0.0f || !std::isnormal(ticktime)) {
		throw IllegalArgumentException("failed to create UI layer, invalid tick time", LOGGER_DETAIL_IDENTIFIER(id));
	}
	auto window = this->context().window.lock();
	window->registerListener("mousemove", this);
	window->registerListener("mouseclick", this);
}

rgle::ui::Layer::~Layer()
{
}

bool rgle::ui::Layer::tick()
{
	return (((float)clock() - (float)_lastTick) / CLOCKS_PER_SEC) >= _tickTime;
}

bool rgle::ui::Layer::raycastHit()
{
	return _castHit;
}

void rgle::ui::Layer::onMessage(std::string eventname, EventMessage * message)
{
	bool click = eventname == "mouseclick";
	auto window = this->context().window.lock();
	if ((eventname == "mousemove" || click) && !window->grabbed()) {
		this->_raycastCheck = true;
		glm::vec2 cursor = window->getCursorPosition();
		this->_mouseState.x = cursor.x;
		this->_mouseState.y = cursor.y;
		if (click) {
			MouseClickMessage* clickMessage = message->cast<MouseClickMessage>();
			this->_mouseState.action = clickMessage->mouse.action;
			this->_mouseState.button = clickMessage->mouse.button;
			this->_mouseState.modifier = clickMessage->mouse.modifier;
		}
	}
}

void rgle::ui::Layer::update()
{
	clock_t currentTime = clock();
	float deltaTime = ((float)currentTime - (float)_lastTick) / CLOCKS_PER_SEC;
	auto window = this->context().window.lock();
	if (deltaTime >= _tickTime) {
		if (_raycastCheck) {
			if (!window->grabbed()) {
				this->_castHit = false;
				glm::vec2 cursor = window->getCursorPosition();
				Ray mouseray = Ray(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3((cursor.x / window->width()) * 2, (cursor.y / window->height()) * 2, 0.0f));
				sElement closest = nullptr;
				for (size_t i = 0; i < this->_elements.size(); i++) {
					if (this->_elements[i]->raycast(mouseray)) {
						if (closest == nullptr || this->_elements[i]->getElementAttribs().zIndex < closest->getElementAttribs().zIndex) {
							if (closest != nullptr) {
								closest->delegateMouseState(mouseray, false, this->_mouseState);
							}
							closest = this->_elements[i];
						}
						else {
							this->_elements[i]->delegateMouseState(mouseray, false, this->_mouseState);
						}
						this->_castHit = true;
					}
					else {
						this->_elements[i]->delegateMouseState(mouseray, false, this->_mouseState);
					}
				}
				if (this->_castHit) {
					DelegateMouseState state = closest->delegateMouseState(mouseray, true, this->_mouseState);
					switch (state) {
					case DelegateMouseState::CURSOR_ARROW:
						window->setCursor(GLFW_ARROW_CURSOR);
						break;
					case DelegateMouseState::CURSOR_HAND:
						window->setCursor(GLFW_HAND_CURSOR);
						break;
					default:
						break;
					}
				}
				else {
					window->setCursor(GLFW_ARROW_CURSOR);
				}
			}
			this->_raycastCheck = false;
		}
		for (size_t i = 0; i < this->_elements.size(); i++) {
			this->_elements[i]->update();
		}
		for (size_t i = 0; i < this->_logicNodes.size(); i++) {
			this->_logicNodes[i]->update();
		}
		_lastTick = currentTime;
	}
}

void rgle::ui::Layer::render()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	this->viewport()->use();
	GLuint currentShader = 0;
	for (size_t i = 0; i < this->_elements.size(); i++) {
		if (this->_elements[i]->shader().expired()) {
			throw gfx::RenderException("failed to render ui element, shader is null", LOGGER_DETAIL_IDENTIFIER(this->_elements[i]->id));
		}
		auto shader = this->_elements[i]->shader().lock();
		if (shader->programId() != currentShader) {
			currentShader = shader->programId();
			shader->use();
		}
		this->transformer()->bind(shader);
		this->_elements[i]->render();
	}
}

void rgle::ui::Layer::addLogicNode(sLogicNode node)
{
	this->_logicNodes.push_back(node);
}

void rgle::ui::Layer::addElement(sElement element)
{
	for (size_t i = 0; i < this->_elements.size(); i++) {
		if (element->getElementAttribs().zIndex < this->_elements[i]->getElementAttribs().zIndex) {
			this->_elements.insert(_elements.begin() + i, element);
			return;
		}
	}
	this->_elements.push_back(element);
}

rgle::ui::Button::Button()
{

}

rgle::ui::Button::~Button()
{
}

void rgle::ui::Button::render()
{
	
}

void rgle::ui::Button::update()
{

}

rgle::ui::DelegateMouseState rgle::ui::Button::delegateMouseState(Ray clickray, bool inside, MouseState state)
{
	int key = this->context().window.lock()->getMouseButton(GLFW_MOUSE_BUTTON_LEFT);
	State next = DEFAULT;
	DelegateMouseState delegateState = DelegateMouseState::UNCHANGED;
	if (!inside) {
		next = DEFAULT;
		delegateState = DelegateMouseState::CURSOR_ARROW;
	}
	else if (key == GLFW_PRESS) {
		next = ACTIVE;
		delegateState = DelegateMouseState::CURSOR_HAND;
	}
	else {
		next = HOVER;
		delegateState = DelegateMouseState::CURSOR_HAND;
	}
	if (next != this->_currentState) {
		this->onStateChange(next, inside, state);
		this->_currentState = next;
	}
	return delegateState;
}

void rgle::ui::Button::onStateChange(State, bool inside, MouseState mouseState)
{
	if (this->_currentState == ACTIVE && inside) {
		this->broadcastEvent("onclick", new MouseStateMessage(mouseState));
	}
}

rgle::ui::BasicButton::BasicButton(std::string shaderid, std::string fontfamily, std::string text, BasicButtonAttributes attribs)
{
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	auto window = this->context().window.lock();
	this->_basicButtonAttributes = attribs;
	
	TextAttributes textattribs{};
	textattribs.zIndex = 1.0f;
	textattribs.topLeft = UnitVector2D(this->_topLeft.x + attribs.paddingHorizontal.x, this->_topLeft.y);
	this->_text = std::unique_ptr<Text>(new Text("text", fontfamily, text, textattribs));
	this->_dimensions = UnitVector2D(
		(attribs.paddingHorizontal.x + attribs.paddingHorizontal.y) + this->_text->getDimensions().x,
		(attribs.paddingVertical.x + attribs.paddingVertical.y) + this->_text->getDimensions().y
	);

	this->_rect = gfx::Rect(shaderid, this->_dimensions.x.resolve(window, Window::X), this->_dimensions.x.resolve(window, Window::X));
	this->_rect.standardFill(attribs.defaultColor);
	this->_rect.updateColorBuffer();
	window->registerListener("mousemove", this);
	window->registerListener("mouseclick", this);
	window->registerListener("resize", this);
}

rgle::ui::BasicButton::~BasicButton()
{
}

void rgle::ui::BasicButton::onBoxUpdate()
{
	glm::mat4 transform = this->transform();
	this->_rect.model.matrix = transform;
	glm::vec2 dim = this->_dimensions.resolve(this->context().window.lock());
	this->_rect.changeDimensions(dim.x, dim.y);
	this->_text->changeTopLeft(UnitVector2D(this->_topLeft.x + this->_basicButtonAttributes.paddingHorizontal.x, this->_topLeft.y));
}

void rgle::ui::BasicButton::onStateChange(Button::State state, bool inside, MouseState mouseState)
{
	Button::onStateChange(state, inside, mouseState);
	switch (state) {
	default:
		this->_rect.standardFill(this->_basicButtonAttributes.defaultColor);
		this->_rect.updateColorBuffer();
		break;
	case Button::HOVER:
		this->_rect.standardFill(this->_basicButtonAttributes.hoverColor);
		this->_rect.updateColorBuffer();
		break;
	case Button::ACTIVE:
		this->_rect.standardFill(this->_basicButtonAttributes.activeColor);
		this->_rect.updateColorBuffer();
	}
}

void rgle::ui::BasicButton::render()
{
	this->_rect.render();
	this->_text->shaderLocked()->use(); // NOTE: rendering should be ordered by shader...
	this->_text->render();
	this->shaderLocked()->use();
}

rgle::ui::RectElement::RectElement()
{
}

rgle::ui::RectElement::RectElement(std::string& shaderid, RectAttributes& attribs)
{
	auto shader = (*this->context().manager.shader.lock())[shaderid];
	this->shader() = shader;
	auto window = this->context().window.lock();
	window->registerListener("resize", this);
	this->_topLeft = attribs.topLeft;
	this->_dimensions = attribs.dimensions;
	this->_attributes = attribs;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(shader->programId(), "model");
	if (this->model.location < 0) {
		throw Exception("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	glm::vec2 dimensions = this->_dimensions.resolve(window);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, -dimensions.y, 0.0),
		glm::vec3(dimensions.x, 0.0, 0.0),
		glm::vec3(dimensions.x, -dimensions.y, 0.0)
	};
	this->index.list = {
		0,
		1,
		2,
		1,
		2,
		3
	};
	this->color.list = {
		this->_attributes.color.evaluate(0.0, 0.0),
		this->_attributes.color.evaluate(0.0, 1.0),
		this->_attributes.color.evaluate(1.0, 0.0),
		this->_attributes.color.evaluate(1.0, 1.0),
	};
	this->generate();
}

rgle::ui::RectElement::~RectElement()
{
}

void rgle::ui::RectElement::onMessage(std::string eventname, EventMessage * message)
{
	Element::onMessage(eventname, message);
	if (eventname == "resize") {
		this->updateGeometry();
	}
}

void rgle::ui::RectElement::onBoxUpdate()
{
	this->model.matrix = this->transform();
}

void rgle::ui::RectElement::render()
{
	this->standardRender(this->shaderLocked());
}

void rgle::ui::RectElement::updateGeometry()
{
	this->model.matrix = this->transform();
	glm::vec2 dimensions = this->_dimensions.resolve(this->context().window.lock());
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, -dimensions.y, 0.0),
		glm::vec3(dimensions.x, 0.0, 0.0),
		glm::vec3(dimensions.x, -dimensions.y, 0.0)
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void rgle::ui::RectElement::changeColor(util::Fill fill)
{
	this->_attributes.color = fill;
	this->color.list = {
		this->_attributes.color.evaluate(0.0, 0.0),
		this->_attributes.color.evaluate(0.0, 1.0),
		this->_attributes.color.evaluate(1.0, 0.0),
		this->_attributes.color.evaluate(1.0, 1.0),
	};
	glBindBuffer(GL_ARRAY_BUFFER, this->color.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * this->color.list.size() * 4, vertex.list.data());
}