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
		newTopLeft.y = _element->getTopLeft().y + _element->getDimensions().y + _attributes.spacing;
		break;
	case Aligner::Direction::LEFT:
		newTopLeft.x = _element->getTopLeft().x + _element->getDimensions().x + _attributes.spacing;
		break;
	case Aligner::Direction::RIGHT:
		newTopLeft.x = _element->getTopLeft().x - _align->getDimensions().x - _attributes.spacing;
		break;
	case Aligner::Direction::TOP:
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
	if (eventname == "mousemove" && !this->_context.window->grabbed()) {
		glm::vec2 cursor = this->_context.window->getCursorPosition();
		Ray mouseray = Ray(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3((cursor.x / this->_context.window->width())*2, (cursor.y / this->_context.window->height())*2, 0.0f));
		if (this->raycast(mouseray)) {
			this->delegateMouseState(mouseray, true);
		}
		else {
			this->delegateMouseState(mouseray, false);
		}
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

void cppogl::UI::Element::delegateMouseState(Ray clickray, bool inside)
{
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

cppogl::UI::Layer::Layer(std::string id, clock_t ticktime) : RenderLayer(id)
{
	this->_tickTime = ticktime;
	this->_lastTick = 0;
}

cppogl::UI::Layer::~Layer()
{
}

bool cppogl::UI::Layer::tick()
{
	return (((float)clock() - (float)_lastTick) / CLOCKS_PER_SEC) >= _tickTime;
}

void cppogl::UI::Layer::update()
{
	clock_t currentTime = clock();
	float deltaTime = ((float)currentTime - (float)_lastTick) / CLOCKS_PER_SEC;
	if (deltaTime >= _tickTime) {
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
	this->renderState(this->_currentState);
}

void cppogl::UI::Button::renderState(State state)
{

}

void cppogl::UI::Button::delegateMouseState(Ray clickray, bool inside)
{
	int key = this->_context.window->getMouseButton(GLFW_MOUSE_BUTTON_LEFT);
	State next = DEFAULT;
	if (!inside) {
		next = DEFAULT;
	}
	else if (key == GLFW_PRESS) {
		next = ACTIVE;
	}
	else {
		next = HOVER;
	}
	if (next != this->_currentState) {
		this->onStateChange(next);
		this->_currentState = next;
	}
}

void cppogl::UI::Button::onStateChange(State state)
{

}

cppogl::UI::BasicButton::BasicButton()
{
}

cppogl::UI::BasicButton::BasicButton(Context context, std::string shader, std::string text)
{
	this->_context = context;
	this->shader = this->_context.manager.shader->operator[](shader);
	this->_default = Rect(this->_context, shader, 0.5f, 0.25f, glm::vec4(1.0, 0.0, 0.0, 1.0));
	this->_active = Rect(this->_context, shader, 0.5f, 0.25f, glm::vec4(0.0, 1.0, 0.0, 1.0));
	this->_dimensions = UnitVector2D(0.5f, 0.25f);
	this->_context.window->registerListener("mousemove", this);
}

cppogl::UI::BasicButton::~BasicButton()
{
}

void cppogl::UI::BasicButton::renderState(Button::State state)
{
	switch (state) {
	default:
		this->_default.model.matrix = this->transform();
		this->_default.render();
		break;
	case Button::HOVER:
		this->_active.model.matrix = this->transform();
		this->_active.render();
		break;
	case Button::ACTIVE:
		this->_active.model.matrix = this->transform();
		this->_active.render();
		break;
	}
}
