#pragma once

#include "rgle/gfx/Graphics.h"
#include "rgle/Raycast.h"
#include "rgle/Event.h"

namespace rgle {

	class Text;

	namespace UI {

		class LayoutChangeMessage : public EventMessage {
		public:
			LayoutChangeMessage();
			virtual ~LayoutChangeMessage();

			struct {
				UnitVector2D topLeft;
				UnitVector2D dimensions;
			} box;
		};

		class BoundingBox : public EventHost {
		public:
			BoundingBox();
			BoundingBox(UnitVector2D dimensions, UnitVector2D topleft);
			virtual ~BoundingBox();

			virtual void onBoxUpdate();

			UnitVector2D getTopLeft();
			UnitVector2D getDimensions();

			void changeDimensions(UnitVector2D dimensions);
			void changeTopLeft(UnitVector2D topleft);
			void change(UnitVector2D dimensions, UnitVector2D topleft);

			void quietChangeDimensions(UnitVector2D dimensions);
			void quietChangeTopLeft(UnitVector2D topleft);
			void quietChange(UnitVector2D dimensions, UnitVector2D topleft);

		protected:
			UnitVector2D _topLeft;
			UnitVector2D _dimensions;
		};

		struct ElementAttributes {
			float zIndex = 0.0f;
		};

		enum class DelegateMouseState {
			UNCHANGED,
			CURSOR_ARROW,
			CURSOR_HAND
		};

		class Element : public Renderable, public Raycastable, public BoundingBox, public EventListener {
		public:
			Element();
			Element(ElementAttributes attributes);
			virtual ~Element();

			virtual void onMessage(std::string eventname, EventMessage* message);

			virtual bool raycast(Ray ray);

			virtual DelegateMouseState delegateMouseState(Ray clickray, bool inside, MouseState state);

			glm::mat4 transform();

			ElementAttributes getElementAttribs();

			virtual const char* typeName() const;
		protected:
			ElementAttributes _elementAttributes;
		};

		typedef std::shared_ptr<Element> sElement;

		class Aligner : public LogicNode, public EventListener {
		public:
			enum class Direction {
				TOP,
				BOTTOM,
				LEFT,
				RIGHT
			};
			Aligner();
			virtual ~Aligner();

			virtual void onMessage(std::string eventname, EventMessage* message);

			virtual void realign();
		};

		struct RelativeAlignerAttributes {
			Aligner::Direction direction = Aligner::Direction::BOTTOM;
			UnitValue spacing = UnitValue{ 0.0f, Unit::ND };
			bool center = false;
		};

		class RelativeAligner : public Aligner {
		public:
			RelativeAligner();
			RelativeAligner(std::shared_ptr<BoundingBox> element, std::shared_ptr<BoundingBox> align, RelativeAlignerAttributes attributes = {});
			virtual ~RelativeAligner();

			void realign();

		protected:
			std::shared_ptr<BoundingBox> _element;
			std::shared_ptr<BoundingBox> _align;
			RelativeAlignerAttributes _attributes;
		};

		struct LinearAlignerAttributes {
			Aligner::Direction direction = Aligner::Direction::BOTTOM;
			UnitValue spacing = UnitValue{ 0.0f, Unit::ND };
			UnitVector2D topLeft;
			UnitValue width;
			UnitValue height;
			bool center = false;
		};

		class LinearAligner : public Aligner {
		public:
			LinearAligner();
			LinearAligner(std::vector<std::shared_ptr<BoundingBox>> elements, LinearAlignerAttributes attributes = LinearAlignerAttributes{});
			virtual ~LinearAligner();

			void push(std::shared_ptr<BoundingBox> element);
			std::shared_ptr<BoundingBox> pop();

			void realign();

		private:
			LinearAlignerAttributes _attributes;
			std::vector<std::shared_ptr<BoundingBox>> _elements;
			std::vector<RelativeAligner> _aligners;
		};


		class Layer : public RenderLayer, public EventListener {
		public:
			Layer(std::string id, float ticktime = 1 / 30.0f);
			virtual ~Layer();

			virtual bool tick();
			virtual bool raycastHit();

			virtual void onMessage(std::string eventname, EventMessage* message);

			virtual void update();
			virtual void render();

			void addLogicNode(sLogicNode node);
			void addElement(sElement element);

		protected:
			bool _raycastCheck;
			MouseState _mouseState;
			bool _castHit;
			float _tickTime;
			clock_t _lastTick;
			std::vector<sElement> _elements;
			std::vector<sLogicNode> _logicNodes;
		};

		struct RectAttributes {
			UnitVector2D dimensions;
			UnitVector2D topLeft;
			float zIndex = 0.0f;
			Fill color;
		};

		class RectElement : public Element, public Geometry3D {
		public:
			RectElement();
			RectElement(std::string& shaderid, RectAttributes& attribs);
			virtual ~RectElement();

			virtual void onMessage(std::string eventname, EventMessage* message);

			virtual void onBoxUpdate();

			virtual void render();

			virtual void updateGeometry();

			void changeColor(Fill fill);

		protected:
			RectAttributes _attributes;
		};

		class Button : public Element {
		public:
			Button();
			virtual ~Button();

			enum State {
				DEFAULT,
				HOVER,
				ACTIVE
			};

			virtual void render();
			virtual void update();

			virtual DelegateMouseState delegateMouseState(Ray clickray, bool inside, MouseState state);

			virtual void onStateChange(State state, bool inside, MouseState mouseState);

		protected:
			State _currentState = DEFAULT;
		};

		struct BasicButtonAttributes {
			UnitVector2D paddingHorizontal = UnitVector2D(8.0, 8.0, Unit::PT);
			UnitVector2D paddingVertical = UnitVector2D(2.0, 2.0, Unit::PT);
			Fill defaultColor = Fill(glm::vec4(0.9, 0.9, 0.9, 1.0));
			Fill hoverColor = Fill(glm::vec4(0.75, 0.75, 0.75, 1.0));
			Fill activeColor = Fill(glm::vec4(0.5, 0.5, 0.5, 1.0));
			Fill fontColor;
		};

		class BasicButton : public Button {
		public:
			BasicButton(
				std::string shaderid,
				std::string fontfamily,
				std::string text,
				BasicButtonAttributes attribs = BasicButtonAttributes{}
			);
			virtual ~BasicButton();

			virtual void onBoxUpdate();

			virtual void onStateChange(Button::State state, bool inside, MouseState mouseState);

			virtual void render();

		protected:
			Rect _rect;
			std::unique_ptr<Text> _text;
			BasicButtonAttributes _basicButtonAttributes;
		};

		class TextField : public Element {
		public:
			TextField();
			virtual ~TextField();
		};
	}
}