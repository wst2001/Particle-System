// Hand Example
// Author: Yi Kangrui <yikangrui@pku.edu.cn>

//#define DIFFUSE_TEXTURE_MAPPING

#include "gl_env.h"
#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <config.h>
#include <map>
#include <string>
#include <cstring>
#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif

#include <iostream>

#include "skeletal_mesh.h"
#include "camera_quat.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace SkeletalAnimation {
	const char *vertex_shader_330 =
		"#version 330 core\n"
		"const int MAX_BONES = 100;\n"
		"uniform mat4 u_bone_transf[MAX_BONES];\n"
		"uniform mat4 u_mvp;\n"
		"layout(location = 0) in vec3 in_position;\n"
		"layout(location = 1) in vec2 in_texcoord;\n"
		"layout(location = 2) in vec3 in_normal;\n"
		"layout(location = 3) in ivec4 in_bone_index;\n"
		"layout(location = 4) in vec4 in_bone_weight;\n"
		"out vec2 pass_texcoord;\n"
		"void main() {\n"
		"    float adjust_factor = 0.0;\n"
		"    for (int i = 0; i < 4; i++) adjust_factor += in_bone_weight[i] * 0.25;\n"
		"    mat4 bone_transform = mat4(1.0);\n"
		"    if (adjust_factor > 1e-3) {\n"
		"        bone_transform -= bone_transform;\n"
		"        for (int i = 0; i < 4; i++)\n"
		"            bone_transform += u_bone_transf[in_bone_index[i]] * in_bone_weight[i] / adjust_factor;\n"
		"	 }\n"
		"    gl_Position = u_mvp * bone_transform * vec4(in_position, 1.0);\n"
		"    pass_texcoord = in_texcoord;\n"
		"}\n";

	const char *fragment_shader_330 =
		"#version 330 core\n"
		"uniform sampler2D u_diffuse;\n"
		"in vec2 pass_texcoord;\n"
		"out vec4 out_color;\n"
		"void main() {\n"
#ifdef DIFFUSE_TEXTURE_MAPPING
		"    out_color = vec4(texture(u_diffuse, pass_texcoord).xyz, 1.0);\n"
#else
		"    out_color = vec4(pass_texcoord, 0.0, 1.0);\n"
#endif
		"}\n";
}

static void error_callback(int error, const char *description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
float get_angle(float angle) {
	float period = 2.4f;
	float passed_time = (float)glfwGetTime();
	float time_in_period = fmod(passed_time, period);

	return abs(time_in_period / (period * 0.5f) - 1.0f) * (M_PI / angle);
}
void Finger_Movement(GLFWwindow* window);
SkeletalMesh::SkeletonModifier modifier;
bool appear_mouse = true;
float lasttime_press = 0.0f;
int main(int argc, char *argv[]) {
	GLFWwindow *window;
	GLuint vertex_shader, fragment_shader, program;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__ // for macos
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(800, 800, "OpenGL output", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSwapInterval(0);

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &SkeletalAnimation::vertex_shader_330, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &SkeletalAnimation::fragment_shader_330, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	int linkStatus;
	if (glGetProgramiv(program, GL_LINK_STATUS, &linkStatus), linkStatus == GL_FALSE)
		std::cout << "Error occured in glLinkProgram()" << std::endl;

	SkeletalMesh::Scene &sr = SkeletalMesh::Scene::loadScene("Hand", DATA_DIR"/Hand.fbx");
	if (&sr == &SkeletalMesh::Scene::error)
		std::cout << "Error occured in loadMesh()" << std::endl;

	sr.setShaderInput(program, "in_position", "in_texcoord", "in_normal", "in_bone_index", "in_bone_weight");

	float passed_time;

	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glm::vec3 Point_A = glm::vec3(0.0f), Point_B = glm::vec3(0.0f);
	glm::vec3 Direction_A = glm::vec3(0.0f, 0.0f, 1.0f), Direction_B = glm::vec3(0.0f, 0.0f, 1.0f);
	float t = 0.0f;
	float SlerpSpeed = 0.0003f;
	bool slerping = false;
	while (!glfwWindowShouldClose(window)) {
		passed_time = (float)glfwGetTime();
		deltaTime = passed_time - lastFrame;
		lastFrame = passed_time;
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		


		Finger_Movement(window);
		processInput(window);

		// --- You may edit above ---

		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glClearColor(0.5, 0.5, 0.5, 1.0);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glm::fmat4 projection = glm::perspective(glm::radians(camera.Zoom), ratio, 0.1f, 150.0f);
		glm::fmat4 view = camera.GetViewMatrix();
		glm::fmat4 mvp = projection * view;

		glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE, (const GLfloat *)&mvp);
		glUniform1i(glGetUniformLocation(program, "u_diffuse"), SCENE_RESOURCE_SHADER_DIFFUSE_CHANNEL);
		SkeletalMesh::Scene::SkeletonTransf bonesTransf;
		sr.getSkeletonTransform(bonesTransf, modifier);
		if (!bonesTransf.empty())
			glUniformMatrix4fv(glGetUniformLocation(program, "u_bone_transf"), bonesTransf.size(), GL_FALSE,
			(float *)bonesTransf.data());
		sr.render();

		ImGui::Begin("Hand");
		float *p_a = &Point_A.x, *p_b = &Point_B.x;
		float *d_a = &Direction_A.x, *d_b = &Direction_B.x;
		ImGui::Text("Please input position and direction.");
		ImGui::Text("Press Alt to hide the mouse.");
		ImGui::InputFloat3("Point A", p_a);
		ImGui::InputFloat3("Direction A", d_a);
		ImGui::InputFloat3("Point B", p_b);
		ImGui::InputFloat3("Direction B", d_b);
		if (ImGui::Button("Start")) {
			slerping = true;
			t = 0.0f;
		}
		if (slerping) {
			Quaternion_Slerp(Point_A, Direction_A, Point_B, Direction_B, t);
			t += SlerpSpeed;
			if (t >= 1.0f) {
				slerping = false;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
			if (passed_time - lasttime_press > 0.5f) {
				appear_mouse ^= 1;
				if (appear_mouse)
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				
				lasttime_press = passed_time;
			}
			
		}
			
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	SkeletalMesh::Scene::unloadScene("Hand");

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
unsigned int FingerSelect = 0, JointSelect = 0;
float Finger_UD_Angle[5] = { 2.4f, 2.4f, 2.4f, 2.4f, 2.4f };
float Finger_LR_Angle[5] = { 2.4f, 2.4f, 2.4f, 2.4f, 2.4f };
void Finger_Movement(GLFWwindow* window) {
	
	std::string FingerName[5] = {
		"thumb_", "index_", "middle_", "ring_", "pinky_"
	};
	std::string JointName[4] = {
		"proximal_phalange", "intermediate_phalange", "distal_phalange", "fingertip"
	};

	
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		FingerSelect = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		FingerSelect = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		FingerSelect = 2;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		FingerSelect = 3;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		FingerSelect = 4;
	}

	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		JointSelect = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		JointSelect = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		JointSelect = 2;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		JointSelect = 3;
	}
	float period = 2.4f;
	
	float speed = 0.05f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		Finger_UD_Angle[FingerSelect] += speed;
		
		Sleep(10);
		if (Finger_UD_Angle[FingerSelect] < 0.0f)
			Finger_UD_Angle[FingerSelect] = 0.0f;
		if (Finger_UD_Angle[FingerSelect] > period)
			Finger_UD_Angle[FingerSelect] = period;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		Finger_UD_Angle[FingerSelect] -= speed;
		Sleep(10);
		if (Finger_UD_Angle[FingerSelect] < 0.0f)
			Finger_UD_Angle[FingerSelect] = 0.0f;
		if (Finger_UD_Angle[FingerSelect] > period)
			Finger_UD_Angle[FingerSelect] = period;

	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		Finger_LR_Angle[FingerSelect] += speed;
		Sleep(10);
		if (Finger_LR_Angle[FingerSelect] < 0.0f)
			Finger_LR_Angle[FingerSelect] = 0.0f;
		if (Finger_LR_Angle[FingerSelect] > period)
			Finger_LR_Angle[FingerSelect] = period;

	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		Finger_LR_Angle[FingerSelect] -= speed;
		Sleep(10);
		if (Finger_LR_Angle[FingerSelect] < 0.0f)
			Finger_LR_Angle[FingerSelect] = 0.0f;
		if (Finger_LR_Angle[FingerSelect] > period)
			Finger_LR_Angle[FingerSelect] = period;

	}

	float UD_angle = abs(Finger_UD_Angle[FingerSelect] / period - 1.0f) * (M_PI / 2.0f);
	float LR_angle = abs(Finger_LR_Angle[FingerSelect] / period - 1.0f) * (M_PI / 2.0f);
	modifier[FingerName[FingerSelect] + JointName[JointSelect]] =
		glm::rotate(glm::identity<glm::mat4>(), UD_angle, glm::fvec3(0.0, 0.0, 1.0)) *
		glm::rotate(glm::identity<glm::mat4>(), LR_angle, glm::fvec3(0.0, 1.0, 0.0));
}