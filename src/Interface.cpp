#include "Interface.h"
#include "Font.h"

rgle::UI::BoundingBox::BoundingBox()
{
}

rgle::UI::BoundingBox::BoundingBox(UnitVector2D dimensions, UnitVector2D topleft)
{
}

rgle::UI::BoundingBox::~BoundingBox()
{
}

void rgle::UI::BoundingBox::onBoxUpdate()
{
}

rgle::UnitVector2D rgle::UI::BoundingBox::getTopLeft()
{
	return _topLeft;
}

rgle::UnitVector2D rgle::UI::BoundingBox::getDimensions()
{
	return _dimensions;
}

void rgle::UI::BoundingBox::changeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::UI::BoundingBox::changeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::UI::BoundingBox::change(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void rgle::UI::BoundingBox::quietChangeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	this->onBoxUpdate();
}

void rgle::UI::BoundingBox::quietChangeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

void rgle::UI::BoundingBox::quietChange(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

rgle::UI::LayoutChangeMessage::LayoutChangeMessage()
{
}

rgle::UI::LayoutChangeMessage::~LayoutChangeMessage()
{
}

rgle::UI::RelativeAligner::RelativeAligner()
{
}

rgle::UI::RelativeAligner::RelativeAligner(sBoundingBox element, sBoundingBox align, RelativeAlignerAttributes attributes)
{
	this->_element = element;
	this->_align = align;
	this->_attributes = attributes;
	this->_element->registerListener("bounding-box", this);
	this->realign();
}

rgle::UI::RelativeAligner::~RelativeAligner()
{
}

void rgle::UI::RelativeAligner::realign()
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

rgle::UI::LinearAligner::LinearAligner()
{
}

rgle::UI::LinearAligner::LinearAligner(std::vector<sBoundingBox> elements, LinearAlignerAttributes attributes)
{
	this->_elements = elements;
	this->_attributes = attributes;
	if (!_elements.empty()) {
		UnitVector2D newTopLeft = _attributes.topLeft;
		_elements.front()->changeTopLeft(newTopLeft);
	}
	RelativeAlignerAttributes attribs;
	attribs.center = _attributes.center;
	attribs.direction = _attributes.direction;
	attribs.spacing = _attributes.spacing;
	for (int i = 1; i < _elements.size(); i++) {
		_aligners.push_back(RelativeAligner(_elements[i - 1], _elements[i], attribs));
	}
}

rgle::UI::LinearAligner::~LinearAligner()
{
}

void rgle::UI::LinearAligner::push(sBoundingBox element)
{
}

rgle::UI::sBoundingBox rgle::UI::LinearAligner::pop()
{
	_aligners.pop_back();
	sBoundingBox bbox = _elements.back();
	_elements.pop_back();
	return bbox;
}

void rgle::UI::LinearAligner::realign()
{
	for (int i = 0; i < _aligners.size(); i++) {
		_aligners[i].realign();
	}
}

rgle::UI::Element::Element()
{
}

rgle::UI::Element::Element(const Context & context, ElementAttributes attributes) : Renderable(context)
{
	this->_elementAttributes = attributes;
}

rgle::UI::Element::~Element()
{
}

void rgle::UI::Element::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->onBoxUpdate();
	}
}

bool rgle::UI::Element::raycast(Ray ray)
{
	glm::vec3 topleft = glm::vec3(_topLeft.resolve(_context.window), 0.0);
	glm::vec2 dimensions = _dimensions.resolve(_context.window);
	glm::vec3 topright = topleft;
	topright.x += dimensions.x;
	glm::vec3 bottomleft = topleft;
	bottomleft.y += dimensions.y;
	glm::vec3 bottomright = topleft;
	bottomright.x += dimensions.x;
	bottomright.y += dimensions.y;

	return ray.intersect(topright, topleft, bottomleft) || ray.intersect(bottomleft, bottomright, topright);
}

rgle::UI::DelegateMouseState rgle::UI::Element::delegateMouseState(Ray clickray, bool inside, MouseState state)
{
	return DelegateMouseState::UNCHANGED;
}

glm::mat4 rgle::UI::Element::transform()
{
	glm::mat4 transform = glm::mat4(1.0f);
	transform[3][0] = _topLeft.x.resolve(_context.window, Window::X);
	transform[3][1] = -_topLeft.y.resolve(_context.window, Window::Y);
	transform[3][2] = _elementAttributes.zIndex;
	return transform;
}

rgle::UI::ElementAttributes rgle::UI::Element::getElementAttribs()
{
	return _elementAttributes;
}

std::string & rgle::UI::Element::typeName()
{
	return std::string("rgle::UI::Element");
}

rgle::UI::Aligner::Aligner()
{
}

rgle::UI::Aligner::~Aligner()
{
}

void rgle::UI::Aligner::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "bounding-box") {
		this->realign();
	}
}

void rgle::UI::Aligner::realign()
{
}

rgle::UI::Layer::Layer(Context context, std::string id, clock_t ticktime) : RenderLayer(id), _tickTime(ticktime), _lastTick(0), _raycastCheck(false), _castHit(false)
{
	this->_tickTime = ticktime;
	this->_lastTick = 0;
	this->_context = context;
	this->_context.window->registerListener("mousemove", this);
	this->_context.window->registerListener("mouseclick", this);
}

rgle::UI::Layer::~Layer()
{
}

bool rgle::UI::Layer::tick()
{
	return (((float)clock() - (float)_lastTick) / CLOCKS_PER_SEC) >= _tickTime;
}

bool rgle::UI::Layer::raycastHit()
{
	return _castHit;
}

void rgle::UI::Layer::onMessage(std::string eventname, EventMessage * message)
{
	bool click = eventname == "mouseclick";
	if (eventname == "mousemove" || click && !this->_context.window->grabbed()) {
		this->_raycastCheck = true;
		glm::vec2 cursor = this->_context.window->getCursorPosition();
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

void rgle::UI::Layer::update()
{
	clock_t currentTime = clock();
	float deltaTime = ((float)currentTime - (float)_lastTick) / CLOCKS_PER_SEC;
	if (deltaTime >= _tickTime) {
		if (_raycastCheck) {
			if (!_context.window->grabbed()) {
				this->_castHit = false;
				glm::vec2 cursor = this->_context.window->getCursorPosition();
				Ray mouseray = Ray(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3((cursor.x / this->_context.window->width()) * 2, (cursor.y / this->_context.window->height()) * 2, 0.0f));
				sElement closest = nullptr;
				for (int i = 0; i < this->_elements.size(); i++) {
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
						this->_context.window->setCursor(GLFW_ARROW_CURSOR);
						break;
					case DelegateMouseState::CURSOR_HAND:
						this->_context.window->setCursor(GLFW_HAND_CURSOR);
						break;
					default:
						break;
					}
				}
				else {
					this->_context.window->setCursor(GLFW_ARROW_CURSOR);
				}
			}
			this->_raycastCheck = false;
		}
		for (int i = 0; i < this->_elements.size(); i++) {
			this->_elements[i]->update();
		}
		for (int i = 0; i < this->_logicNodes.size(); i++) {
			this->_logicNodes[i]->update();
		}
		_lastTick = currentTime;
	}
}

void rgle::UI::Layer::render()
{
	GLuint currentShader = 0;
	for (int i = 0; i < _elements.size(); i++) {
		if (_elements[i]->shader == nullptr) {
			throw RenderException("failed to render ui element, shader is null", LOGGER_DETAIL_IDENTIFIER(_elements[i]->id));
		}
		if (_elements[i]->shader->programId() != currentShader) {
			currentShader = _elements[i]->shader->programId();
			_elements[i]->shader->use();
		}
		_elements[i]->render();
	}
}

void rgle::UI::Layer::addLogicNode(sLogicNode node)
{
	this->_logicNodes.push_back(node);
}

void rgle::UI::Layer::addElement(sElement element)
{
	for (int i = 0; i < this->_elements.size(); i++) {
		if (element->getElementAttribs().zIndex < this->_elements[i]->getElementAttribs().zIndex) {
			_elements.insert(_elements.begin() + i, element);
			return;
		}
	}
	this->_elements.push_back(element);
}

rgle::UI::Button::Button()
{

}

rgle::UI::Button::~Button()
{
}

void rgle::UI::Button::render()
{
	
}

void rgle::UI::Button::update()
{

}

rgle::UI::DelegateMouseState rgle::UI::Button::delegateMouseState(Ray clickray, bool inside, MouseState state)
{
	int key = this->_context.window->getMouseButton(GLFW_MOUSE_BUTTON_LEFT);
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

void rgle::UI::Button::onStateChange(State state, bool inside, MouseState mouseState)
{
	if (this->_currentState == ACTIVE && inside) {
		this->broadcastEvent("onclick", new MouseStateMessage(mouseState));
	}
}

rgle::UI::BasicButton::BasicButton()
{
}

rgle::UI::BasicButton::BasicButton(Context context, std::string shader, std::string fontfamily, std::string text, BasicButtonAttributes attribs)
{
	this->_context = context;
	this->shader = this->_context.manager.shader->operator[](shader);
	this->_basicButtonAttributes = attribs;
	
	TextAttributes textattribs{};
	textattribs.zIndex = -0.1f;
	textattribs.topLeft = UnitVector2D(this->_topLeft.x + attribs.paddingHorizontal.x, this->_topLeft.y);
	this->_text = std::unique_ptr<Text>(new Text(this->_context, "text", fontfamily, text, textattribs));
	this->_dimensions = UnitVector2D(
		(attribs.paddingHorizontal.x + attribs.paddingHorizontal.y) + this->_text->getDimensions().x,
		(attribs.paddingVertical.x + attribs.paddingVertical.y) + this->_text->getDimensions().y
	);

	this->_rect = Rect(this->_context, shader, this->_dimensions.x.resolve(this->_context.window), this->_dimensions.x.resolve(this->_context.window, Window::X));
	this->_rect.standardFill(attribs.defaultColor);
	this->_rect.updateColorBuffer();
	this->_context.window->registerListener("mousemove", this);
	this->_context.window->registerListener("mouseclick", this);
	this->_context.window->registerListener("resize", this);
}

rgle::UI::BasicButton::~BasicButton()
{
}

void rgle::UI::BasicButton::onBoxUpdate()
{
	glm::mat4 transform = this->transform();
	this->_rect.model.matrix = transform;
	glm::vec2 dim = this->_dimensions.resolve(this->_context.window);
	this->_rect.changeDimensions(dim.x, dim.y);
	this->_text->changeTopLeft(UnitVector2D(this->_topLeft.x + this->_basicButtonAttributes.paddingHorizontal.x, this->_topLeft.y));
}

void rgle::UI::BasicButton::onStateChange(Button::State state, bool inside, MouseState mouseState)
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

void rgle::UI::BasicButton::render()
{
	this->_rect.render();
	this->_text->shader->use(); // NOTE: rendering should be ordered by shader...
	this->_text->render();
	this->shader->use();
}

rgle::UI::RectElement::RectElement()
{
}

rgle::UI::RectElement::RectElement(Context& context, std::string& shader, RectAttributes& attribs)
{
	this->_context = context;
	this->shader = (*this->_context.manager.shader)[shader];
	this->_context.window->registerListener("resize", this);
	this->_topLeft = attribs.topLeft;
	this->_dimensions = attribs.dimensions;
	this->_attributes = attribs;
	this->model.matrix = glm::mat4(1.0f);
	this->model.location = glGetUniformLocation(this->shader->programId(), "model");
	if (this->model.location < 0) {
		throw Exception("failed to locate shader uniform: model matrix", LOGGER_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(this->shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", LOGGER_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", LOGGER_DETAIL_DEFAULT);
	}
	glm::vec2 dimensions = this->_dimensions.resolve(this->_context.window);
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

rgle::UI::RectElement::~RectElement()
{
}

void rgle::UI::RectElement::onMessage(std::string eventname, EventMessage * message)
{
	Element::onMessage(eventname, message);
	if (eventname == "resize") {
		this->updateGeometry();
	}
}

void rgle::UI::RectElement::onBoxUpdate()
{
	this->model.matrix = this->transform();
}

void rgle::UI::RectElement::render()
{
	this->standardRender(this->shader);
}

void rgle::UI::RectElement::updateGeometry()
{
	this->model.matrix = this->transform();
	glm::vec2 dimensions = this->_dimensions.resolve(this->_context.window);
	this->vertex.list = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, -dimensions.y, 0.0),
		glm::vec3(dimensions.x, 0.0, 0.0),
		glm::vec3(dimensions.x, -dimensions.y, 0.0)
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertex.list.size() * 3, vertex.list.data());
}

void rgle::UI::RectElement::changeColor(Fill color)
{
	this->_attributes.color = color;
	this->color.list = {
		this->_attributes.color.evaluate(0.0, 0.0),
		this->_attributes.color.evaluate(0.0, 1.0),
		this->_attributes.color.evaluate(1.0, 0.0),
		this->_attributes.color.evaluate(1.0, 1.0),
	};
	glBindBuffer(GL_ARRAY_BUFFER, this->color.buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * this->color.list.size() * 4, vertex.list.data());
}