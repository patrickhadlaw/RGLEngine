#pragma once

#include "Window.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>

namespace rgle {

	enum CameraType {
		PERSPECTIVE_PROJECTION,
		ORTHOGONAL_PROJECTION,
	};

	class Viewport {
	public:
		Viewport();
		Viewport(int width, int height, glm::ivec2 position);
		virtual ~Viewport();

		virtual void use();

	private:
		int _width;
		int _height;
		glm::ivec2 _position;
	};

	typedef std::shared_ptr<Viewport> sViewport;

	class ViewTransformer {
	public:
		ViewTransformer();
		virtual ~ViewTransformer();

		virtual void update(float deltaT);

		virtual void bind(sShaderProgram program);
	};

	typedef std::shared_ptr<ViewTransformer> sViewTransformer;

	class Camera : public ViewTransformer, public EventListener {
	public:
		Camera();
		Camera(CameraType type, sWindow window);
		~Camera();

		virtual void onMessage(std::string eventname, EventMessage* message);

		virtual void update(float deltaT);

		virtual void bind(sShaderProgram program);

		void generate(CameraType type);

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);
		void lookAt(glm::vec3 direction);

		glm::vec3 position();
		glm::vec3 direction();
		glm::vec3 up();
		glm::mat4 view();
		glm::mat4 projection();

	protected:
		glm::vec3 _position;
		glm::vec3 _direction;
		glm::vec3 _up;
		glm::vec3 _right;
		glm::mat4 _view;
		glm::mat4 _projection;
		GLint _viewLocation;
		GLint _projectionLocation;
		CameraType _type;

		sWindow _window;
	};

	typedef std::shared_ptr<Camera> sCamera;

	class NoClipCamera : public Camera {
	public:
		NoClipCamera();
		NoClipCamera(CameraType type, sWindow window);
		~NoClipCamera();

		virtual void onMessage(std::string eventname, EventMessage* message);

		virtual void update(float deltaT);

		virtual void grab();
		virtual void unGrab();

		void moveRelative(float forward, float horizontal, float vertical);

	protected:
		struct {
			double deltaX = 0.0f;
			double deltaY = 0.0f;
		} _mouse;
		bool _isGrabbed;
	};
}
