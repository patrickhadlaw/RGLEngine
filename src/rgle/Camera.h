#pragma once

#include "rgle/Window.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>

namespace rgle {

	float z_index(float z);

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

	class ViewTransformer {
	public:
		ViewTransformer();
		virtual ~ViewTransformer();

		virtual void update(float deltaT);

		virtual void bind(std::shared_ptr<ShaderProgram> program);
	};

	class Camera : public ViewTransformer, public EventListener {
	public:
		Camera();
		Camera(CameraType type, std::shared_ptr<Window> window);
		~Camera();

		virtual void onMessage(std::string eventname, EventMessage* message);

		virtual void update(float deltaT);

		virtual void bind(std::shared_ptr<ShaderProgram> program);

		void generate(CameraType type);

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);
		void lookAt(glm::vec3 direction);

		void relocate(glm::vec3 position);

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
		CameraType _type;

		std::shared_ptr<Window> _window;
	};

	class NoClipCamera : public Camera {
	public:
		NoClipCamera();
		NoClipCamera(CameraType type, std::shared_ptr<Window> window);
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
