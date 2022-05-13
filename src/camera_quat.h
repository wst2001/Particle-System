#include "gl_env.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	ANTICLOCKWISE,
	CLOCKWISE
};

float lastX = 800 / 2.0f;
float lastY = 800 / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


class Camera {
public:
	// camera matrix
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::mat4 viewMatrix;
	// camera options
	float Yaw = 90.0f;
	float Pitch = 0.0f;
	float MovementSpeed = 5.0f;
	float MouseSensitivity = 0.05f;
	float RotationSensitivity = 10.0f;
	float Zoom = 45.0f;

	Camera() {
		Position = glm::vec3(0.0f, 0.0f, -10.0f);
		Front = glm::vec3(0.0f, 0.0f, 1.0f);
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		Right = glm::vec3(1.0f, 0.0f, 0.0f);
		WorldUp = Up;
		updateCameraVectors();
	}
	Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
		Position = position;
		Up = up;
		Yaw = yaw;
		Pitch = pitch;
		WorldUp = up;
		updateCameraVectors();
	}
	glm::mat4 GetViewMatrix(){
		return glm::lookAt(Position, Position + Front, Up);
	}
	void Keyboard(Camera_Movement direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position += Right * velocity;
		if (direction == RIGHT)
			Position -= Right * velocity;
		updateCameraVectors();
	}

	void Mouse(float xoffset, float yoffset) {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;
		float theta_x = glm::radians(-xoffset / 2.0f), theta_y = glm::radians(-yoffset / 2.0f);
		glm::quat Rotation_Axis_x = glm::quat(
			glm::cos(theta_x), 
			glm::sin(theta_x) * Up.x,
			glm::sin(theta_x) * Up.y,
			glm::sin(theta_x) * Up.z
		);
		glm::quat front = glm::quat(0.0f, Front);
		glm::quat right = glm::quat(0.0f, Right);
		glm::quat up = glm::quat(0.0f, Up);
		front = Rotation_Axis_x * front * glm::inverse(Rotation_Axis_x);
		
		glm::quat Rotation_Axis_y = glm::quat(
			glm::cos(theta_y),
			glm::sin(theta_y) * right.x,
			glm::sin(theta_y) * right.y,
			glm::sin(theta_y) * right.z
		);
		front = Rotation_Axis_y * front * glm::inverse(Rotation_Axis_y);
		
		Front = glm::vec3(front.x, front.y, front.z);
		Right = glm::cross(WorldUp, Front);
		Up = glm::cross(Front, Right);

		updateCameraVectors();
	}
	void viewRoll(Camera_Movement clockwise, float deltaTime) {
		float theta_z = RotationSensitivity * deltaTime;
		
		
		if (clockwise == ANTICLOCKWISE) {
			theta_z = glm::radians(-theta_z / 2.0f);
		}
		if (clockwise == CLOCKWISE) {
			theta_z = glm::radians(theta_z / 2.0f);
		}
		glm::quat Rotation_Axis_z = glm::quat(
			glm::cos(theta_z),
			glm::sin(theta_z) * Front.x,
			glm::sin(theta_z) * Front.y,
			glm::sin(theta_z) * Front.z
		);
		glm::quat right = glm::quat(0.0f, Right);
		glm::quat up = glm::quat(0.0f, Up);
		right = Rotation_Axis_z * right * glm::inverse(Rotation_Axis_z);
		up = Rotation_Axis_z * up * glm::inverse(Rotation_Axis_z);
		
		Right = glm::vec3(right.x, right.y, right.z);
		Up = glm::vec3(up.x, up.y, up.z);
		WorldUp = Up;
	}
	void MouseScroll(float yoffset) {
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}
private:
	void updateCameraVectors() {
		
	}
}camera;

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.Keyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.Keyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.Keyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.Keyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.viewRoll(ANTICLOCKWISE, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.viewRoll(CLOCKWISE, deltaTime);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.MouseScroll(static_cast<float> (yoffset));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.Mouse(xoffset, yoffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void Quaternion_Slerp(glm::vec3 Point_A, glm::vec3 Direction_A, glm::vec3 Point_B, glm::vec3 Direction_B, float t) {
	glm::vec3 Position = Point_A * (1.0f - t) + Point_B * t;
	glm::quat DA_quat = glm::quat(0.0f, Direction_A);
	glm::quat DB_quat = glm::quat(0.0f, Direction_B);
	glm::quat Front = glm::slerp(DA_quat, DB_quat, t);
	camera.Position = Position;
	camera.Front = glm::normalize(glm::vec3(Front.x, Front.y, Front.z));
	//std::cout << glm::to_string(Front) << std::endl;
	camera.Up = glm::normalize(glm::cross(camera.Front, camera.Right));
	camera.Right = glm::normalize(glm::cross(camera.Up, camera.Front));

}