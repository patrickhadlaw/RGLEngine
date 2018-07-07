#pragma once

#include "Window.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>

namespace cppogl {

	enum CameraType {
		PERSPECTIVE_PROJECTION,
		ORTHOGONAL_PROJECTION,
	};

	class Camera {
	public:
		Camera();
		Camera(CameraType type, sWindow window, sShaderProgram shader);
		~Camera();

		virtual void update(float deltaT);

		void generate(CameraType type);

		void translate(float x, float y, float z);
		void rotate(float x, float y, float z);
		void lookAt(glm::vec3 direction);

	protected:
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 up;
		glm::vec3 right;
		glm::mat4 view;
		glm::mat4 projection;
		GLint viewLocation;
		GLint projectionLocation;

		sWindow window;
	};

	class NoClipCamera : public Camera {
	public:
		NoClipCamera();
		NoClipCamera(CameraType type, sWindow window, sShaderProgram program);
		~NoClipCamera();

		virtual void update(float deltaT);

		void moveRelative(float forward, float horizontal, float vertical);

	protected:
		bool isGrabbed;
	};
}
