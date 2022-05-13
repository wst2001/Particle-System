#pragma once
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#define NUM_PARTICLES 125
#define PI_FLOAT 3.1415927410125732421875f
#define PARTICLE_RADIUS 0.5f
#define PARTICLE_RESTING_DENSITY 125
#define PARTICLE_MASS 1.0f
#define SMOOTHING_LENGTH (4 * PARTICLE_RADIUS)
#define PARTICLE_STIFFNESS 2000
#define PARTICLE_VISCOSITY 3000.0f
#define GRAVITY_FORCE glm::vec3(0.0f, -9806.65f, 0.0f)

struct Particle {
	glm::vec3 Position, Velocity, Force;
	glm::vec3 Color;
	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Force(0.0f) {}
	Particle(glm::vec3 pos): Position(pos), Velocity(0.0f), Color(1.0f), Force(0.0f) {}
};
float x_floor = 1.0f, y_floor = 1.0f, z_floor = 2.0f;
class ParticleGenerator {
private:
	Shader shader;
	Texture texture;
	unsigned int VAO;
	std::vector<Particle> particles;
	float density[NUM_PARTICLES];
	float pressure[NUM_PARTICLES];
	void init() {
		unsigned int VBO;
		float particle_quad[] = {
			// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f

		};
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		for (float i = 0.0f; i < x_floor; i += float(x_floor / 5.0f)) {
			for (float j = 0.0f; j < y_floor; j += float(y_floor / 5.0f)) {
				for (float k = 1.0f; k < z_floor; k += float((z_floor - 1.0f) / 5.0f)) {
					this->particles.push_back(Particle(glm::vec3(i, j, k)));
				}
			}
		}
	}
	float rand1() {
		return (2 * float(rand()) / float(RAND_MAX)) - 1.0f;
	}
	void Calculate_Density_Pressure() {
		for (int i = 0; i < NUM_PARTICLES; i++) {
			float density_sum = 0.0f;
			for (int j = 0; j < NUM_PARTICLES; j++) {
				if (i == j) continue;
				glm::vec3 delta = particles[i].Position - particles[j].Position;
				float r = glm::length(delta);
				density_sum += PARTICLE_MASS * 315.0f * pow(SMOOTHING_LENGTH * SMOOTHING_LENGTH - r * r, 3) /
					(64.0f * PI_FLOAT * pow(SMOOTHING_LENGTH, 9));
				

			}
			density[i] = density_sum;
			pressure[i] = PARTICLE_STIFFNESS * (density_sum - PARTICLE_RESTING_DENSITY);
			pressure[i] = pressure[i] > 0.0f ? pressure[i] : 0.0f;
		}
	}
	void Calculate_Force() {
		for (int i = 0; i < NUM_PARTICLES; i++) {
			glm::vec3 pressure_force = glm::vec3(0.0f);
			glm::vec3 viscosity_force = glm::vec3(0.0f);
			for (int j = 0; j < NUM_PARTICLES; j++) {
				if (i == j) continue;
				glm::vec3 delta = particles[i].Position - particles[j].Position;
				float r = glm::length(delta);
				
				pressure_force -= PARTICLE_MASS * (pressure[i] + pressure[j]) / (2.f * density[j]) *
					-45.f / (PI_FLOAT * (float) pow(SMOOTHING_LENGTH, 6)) * (float) pow(SMOOTHING_LENGTH - r, 2) * glm::normalize(delta);
				viscosity_force += PARTICLE_MASS * (particles[j].Velocity - particles[i].Velocity) / density[j] *
					45.f / (PI_FLOAT * pow(SMOOTHING_LENGTH, 6)) * (SMOOTHING_LENGTH - r);
				
			}
			viscosity_force *= PARTICLE_VISCOSITY;
			glm::vec3 external_force = density[i] * GRAVITY_FORCE;
			particles[i].Force = pressure_force + viscosity_force + external_force;			
		}
	}
public:
	ParticleGenerator(Shader shader) : shader(shader) {
		this->init();
	}

	void Update(float deltaTime) {
		Calculate_Density_Pressure();
		Calculate_Force();
		for (int i = 0; i < NUM_PARTICLES; i++) {
			glm::vec3 a = particles[i].Force / PARTICLE_MASS;
			particles[i].Velocity += deltaTime * a;
			particles[i].Position += particles[i].Velocity * deltaTime;
			if (particles[i].Position.x < 0.0f)
				particles[i].Position.x = 0.0f;
			if (particles[i].Position.x > x_floor)
				particles[i].Position.x = x_floor;
			if (particles[i].Position.y < 0.0f)
				particles[i].Position.y = 0.0f;
			if (particles[i].Position.y > y_floor)
				particles[i].Position.y = y_floor;
			if (particles[i].Position.z < 0.0f)
				particles[i].Position.z = 0.0f;
		}
	}
	void Draw() {
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		this->shader.use();
		for (Particle particle : this->particles) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, particle.Position);
			model = glm::scale(model, glm::vec3(0.03f));
			//model = glm::rotate(model, glm::radians(particle.Angle), glm::vec3(1.0f, 0.3f, 0.5f));
			this->shader.setMat4("model", model);
			this->shader.setVec3("Color", particle.Color);
			//this->texture.Bind();
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
	}
};