#pragma once

#include "Renderable.h"
#include "Event.h"

namespace cppogl {
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

		typedef std::shared_ptr<BoundingBox> sBoundingBox;

		struct ElementAttributes {
			float zIndex = 0.0f;
		};

		class Element : public Renderable, public BoundingBox {
		public:
			Element();
			Element(const Context& context, ElementAttributes attributes);
			virtual ~Element();

			virtual bool hover(glm::vec2 cursor);

			glm::mat4 transform();

			ElementAttributes getElementAttribs();

			virtual std::string& typeName();
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
			RelativeAligner(sBoundingBox element, sBoundingBox align, RelativeAlignerAttributes attributes = {});
			virtual ~RelativeAligner();

			void realign();

		protected:
			sBoundingBox _element;
			sBoundingBox _align;
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
			LinearAligner(std::vector<sBoundingBox> elements, LinearAlignerAttributes attributes = LinearAlignerAttributes{});
			virtual ~LinearAligner();

			void push(sBoundingBox element);
			sBoundingBox pop();

			void realign();

		private:
			LinearAlignerAttributes _attributes;
			std::vector<sBoundingBox> _elements;
			std::vector<RelativeAligner> _aligners;
		};

		typedef std::shared_ptr<LinearAligner> sLinearAligner;

		class Layer : public RenderLayer {
		public:
			Layer(std::string id, clock_t ticktime = 30 / 1000);
			virtual ~Layer();

			virtual bool tick();

			virtual void update();
			virtual void render();

			void addLogicNode(sLogicNode node);
			void addElement(sElement element);

		protected:
			clock_t _tickTime;
			clock_t _lastTick;
			std::vector<sElement> _elements;
			std::vector<sLogicNode> _logicNodes;
		};
		typedef std::shared_ptr<Layer> sLayer;
	}
}