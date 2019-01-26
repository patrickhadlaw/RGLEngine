#include "Interface.h"
#include "Font.h"

cppogl::UI::BoundingBox::BoundingBox()
{
}

cppogl::UI::BoundingBox::BoundingBox(UnitVector2D dimensions, UnitVector2D topleft)
{
}

cppogl::UI::BoundingBox::~BoundingBox()
{
}

void cppogl::UI::BoundingBox::onBoxUpdate()
{
}

cppogl::UnitVector2D cppogl::UI::BoundingBox::getTopLeft()
{
	return _topLeft;
}

cppogl::UnitVector2D cppogl::UI::BoundingBox::getDimensions()
{
	return _dimensions;
}

void cppogl::UI::BoundingBox::changeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void cppogl::UI::BoundingBox::changeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void cppogl::UI::BoundingBox::change(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	LayoutChangeMessage* message = new LayoutChangeMessage();
	message->box.dimensions = _dimensions;
	message->box.topLeft = _topLeft;
	this->broadcastEvent("bounding-box", message);
	this->onBoxUpdate();
}

void cppogl::UI::BoundingBox::quietChangeDimensions(UnitVector2D dimensions)
{
	this->_dimensions = dimensions;
	this->onBoxUpdate();
}

void cppogl::UI::BoundingBox::quietChangeTopLeft(UnitVector2D topleft)
{
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

void cppogl::UI::BoundingBox::quietChange(UnitVector2D dimensions, UnitVector2D topleft)
{
	this->_dimensions = dimensions;
	this->_topLeft = topleft;
	this->onBoxUpdate();
}

cppogl::UI::LayoutChangeMessage::LayoutChangeMessage()
{
}

cppogl::UI::LayoutChangeMessage::~LayoutChangeMessage()
{
}

cppogl::UI::RelativeAligner::RelativeAligner()
{
}

cppogl::UI::RelativeAligner::RelativeAligner(sBoundingBox element, sBoundingBox align, RelativeAlignerAttributes attributes)
{
	this->_element = element;
	this->_align = align;
	this->_attributes = attributes;
	this->_element->registerListener("bounding-box", this);
	this->realign();
}

cppogl::UI::RelativeAligner::~RelativeAligner()
{
}

void cppogl::UI::RelativeAligner::realign()
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

cppogl::UI::LinearAligner::LinearAligner()
{
}

cppogl::UI::LinearAligner::LinearAligner(std::vector<sBoundingBox> elements, LinearAlignerAttributes attributes)
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

cppogl::UI::LinearAligner::~LinearAligner()
{
}

void cppogl::UI::LinearAligner::push(sBoundingBox element)
{
}

cppogl::UI::sBoundingBox cppogl::UI::LinearAligner::pop()
{
	_aligners.pop_back();
	sBoundingBox bbox = _elements.back();
	_elements.pop_back();
	return bbox;
}

void cppogl::UI::LinearAligner::realign()
{
	for (int i = 0; i < _aligners.size(); i++) {
		_aligners[i].realign();
	}
}

cppogl::UI::Element::Element()
{
}

cppogl::UI::Element::Element(const Context & context, ElementAttributes attributes) : Renderable(context)
{
	this->_elementAttributes = attributes;
}

cppogl::UI::Element::~Element()
{
}

void cppogl::UI::Element::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "resize") {
		this->onBoxUpdate();
	}
}

bool cppogl::UI::Element::raycast(Ray ray)
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

cppogl::UI::DelegateMouseState cppogl::UI::Element::delegateMouseState(Ray clickray, bool inside, MouseState state)
{
	return DelegateMouseState::UNCHANGED;
}

glm::mat4 cppogl::UI::Element::transform()
{
	glm::mat4 transform = glm::mat4(1.0f);
	transform[3][0] = _topLeft.x.resolve(_context.window, Window::X);
	transform[3][1] = -_topLeft.y.resolve(_context.window, Window::Y);
	transform[3][2] = _elementAttributes.zIndex;
	return transform;
}

cppogl::UI::ElementAttributes cppogl::UI::Element::getElementAttribs()
{
	return _elementAttributes;
}

std::string & cppogl::UI::Element::typeName()
{
	return std::string("cppogl::UI::Element");
}

cppogl::UI::Aligner::Aligner()
{
}

cppogl::UI::Aligner::~Aligner()
{
}

void cppogl::UI::Aligner::onMessage(std::string eventname, EventMessage * message)
{
	if (eventname == "bounding-box") {
		this->realign();
	}
}

void cppogl::UI::Aligner::realign()
{
}

cppogl::UI::Layer::Layer(Context context, std::string id, clock_t ticktime) : RenderLayer(id), _tickTime(ticktime), _lastTick(0), _raycastCheck(false), _castHit(false)
{
	this->_tickTime = ticktime;
	this->_lastTick = 0;
	this->_context = context;
	this->_context.window->registerListener("mousemove", this);
	this->_context.window->registerListener("mouseclick", this);
}

cppogl::UI::Layer::~Layer()
{
}

bool cppogl::UI::Layer::tick()
{
	return (((float)clock() - (float)_lastTick) / CLOCKS_PER_SEC) >= _tickTime;
}

bool cppogl::UI::Layer::raycastHit()
{
	return _castHit;
}

void cppogl::UI::Layer::onMessage(std::string eventname, EventMessage * message)
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

void cppogl::UI::Layer::update()
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

void cppogl::UI::Layer::render()
{
	GLuint currentShader = 0;
	for (int i = 0; i < _elements.size(); i++) {
		if (_elements[i]->shader == nullptr) {
			throw RenderException("failed to render ui element, shader is null", EXCEPT_DETAIL_IDENTIFIER(_elements[i]->id));
		}
		if (_elements[i]->shader->programId() != currentShader) {
			currentShader = _elements[i]->shader->programId();
			_elements[i]->shader->use();
		}
		_elements[i]->render();
	}
}

void cppogl::UI::Layer::addLogicNode(sLogicNode node)
{
	this->_logicNodes.push_back(node);
}

void cppogl::UI::Layer::addElement(sElement element)
{
	for (int i = 0; i < this->_elements.size(); i++) {
		if (element->getElementAttribs().zIndex < this->_elements[i]->getElementAttribs().zIndex) {
			_elements.insert(_elements.begin() + i, element);
			return;
		}
	}
	this->_elements.push_back(element);
}

cppogl::UI::Button::Button()
{

}

cppogl::UI::Button::~Button()
{
}

void cppogl::UI::Button::render()
{
	
}

void cppogl::UI::Button::update()
{

}

cppogl::UI::DelegateMouseState cppogl::UI::Button::delegateMouseState(Ray clickray, bool inside, MouseState state)
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

void cppogl::UI::Button::onStateChange(State state, bool inside, MouseState mouseState)
{
	if (this->_currentState == ACTIVE && inside) {
		this->broadcastEvent("onclick", new MouseStateMessage(mouseState));
	}
}

cppogl::UI::BasicButton::BasicButton()
{
}

cppogl::UI::BasicButton::BasicButton(Context context, std::string shader, std::string fontfamily, std::string text, BasicButtonAttributes attribs)
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

cppogl::UI::BasicButton::~BasicButton()
{
}

void cppogl::UI::BasicButton::onBoxUpdate()
{
	glm::mat4 transform = this->transform();
	this->_rect.model.matrix = transform;
	glm::vec2 dim = this->_dimensions.resolve(this->_context.window);
	this->_rect.changeDimensions(dim.x, dim.y);
	this->_text->changeTopLeft(UnitVector2D(this->_topLeft.x + this->_basicButtonAttributes.paddingHorizontal.x, this->_topLeft.y));
}

void cppogl::UI::BasicButton::onStateChange(Button::State state, bool inside, MouseState mouseState)
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

void cppogl::UI::BasicButton::render()
{
	this->_rect.render();
	this->_text->shader->use(); // NOTE: rendering should be ordered by shader...
	this->_text->render();
	this->shader->use();
}

cppogl::UI::RectElement::RectElement()
{
}

cppogl::UI::RectElement::RectElement(Context& context, std::string& shader, RectAttributes& attribs)
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
		throw Exception("failed to locate shader uniform: model matrix", EXCEPT_DETAIL_DEFAULT);
	}
	this->color.location = glGetAttribLocation(this->shader->programId(), "vertex_color");
	if (this->color.location < 0) {
		throw Exception("failed to locate shader attribute: vertex color", EXCEPT_DETAIL_DEFAULT);
	}
	this->vertex.location = glGetAttribLocation(this->shader->programId(), "vertex_position");
	if (this->vertex.location < 0) {
		throw Exception("failed to locate shader attribute: vertex position", EXCEPT_DETAIL_DEFAULT);
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

cppogl::UI::RectElement::~RectElement()
{
}

void cppogl::UI::RectElement::onMessage(std::string eventname, EventMessage * message)
{
	Element::onMessage(eventname, message);
	if (eventname == "resize") {
		this->updateGeometry();
	}
}

void cppogl::UI::RectElement::onBoxUpdate()
{
	this->model.matrix = this->transform();
}

void cppogl::UI::RectElement::render()
{
	this->standardRender(this->shader);
}

void cppogl::UI::RectElement::updateGeometry()
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

void cppogl::UI::RectElement::changeColor(Fill color)
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