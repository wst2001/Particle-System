#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"
#include "mesh.h"

struct Particle {
	glm::vec3 Position, Velocity;
	glm::vec3 Color;
	float Life, Angle;
	float CameraDistance;
	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};
int MaxParticles = 10000;
int currentParticles = 10000;
int LastUnusedParticle = 0;
float MaxLife = 2.0f;
bool SelfLight = false;
glm::vec3 CurrentColor = glm::vec3(1.0f);
class ParticleGenerator {
private:
	Shader shader;
	Texture texture;
	unsigned int VAO;
	std::vector<Particle> particles;
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

		for (int i = 0; i < MaxParticles; i++) {
			this->particles.push_back(Particle());
		}
	}
	unsigned int firstUnusedParticle() {
		for (int i = LastUnusedParticle; i < currentParticles; i++) {
			if (particles[i].Life < 0) {
				LastUnusedParticle = i;
				return i;
			}
		}
		for (int i = 0; i < LastUnusedParticle; i++) {
			if (particles[i].Life < 0) {
				LastUnusedParticle = i;
				return i;
			}
		}
		LastUnusedParticle = 0;
		return MaxParticles + 10;
	}
	float rand1() {
		return (2 * float(rand()) / float(RAND_MAX)) - 1.0f;
	}
	void respawnParticle(Particle &particle) {
		
		particle.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		//particle.Color = glm::vec3(1.0f);// glm::vec3((rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f);
		particle.Color = CurrentColor;
		particle.Life = MaxLife;
		particle.Velocity = glm::vec3(rand1(), rand1() + 5.0f, rand1());
		//particle.Angle = rand1() * 180.0f;
	}
	void genCurrentColor() {
		CurrentColor = CurrentColor + glm::vec3(rand1(), rand1(), rand1()) * 0.2f;
		if (CurrentColor.x < 0.0f)
			CurrentColor.x = 0.0f;
		if (CurrentColor.x > 1.0f)
			CurrentColor.x = 1.0f;
		if (CurrentColor.y < 0.0f)
			CurrentColor.y = 0.0f;
		if (CurrentColor.y > 1.0f)
			CurrentColor.y = 1.0f;
		if (CurrentColor.z < 0.0f)
			CurrentColor.z = 0.0f;
		if (CurrentColor.z > 1.0f)
			CurrentColor.z = 1.0f;
	}
public:
	ParticleGenerator(Shader shader): shader(shader){
		this->init();
	}
	void Update(float deltaTime) {
		int newParticles = (int)(deltaTime * currentParticles);
		if (newParticles > (int)(0.016f * currentParticles))
			newParticles = (int)(0.016f * currentParticles);
		genCurrentColor();
		for (int i = 0; i < newParticles; i++) {
			int unusedParticle = this->firstUnusedParticle();
			if (unusedParticle > MaxParticles) break;
			this->respawnParticle(this->particles[unusedParticle]);
		}
		for (int i = 0; i < MaxParticles; i++) {
			Particle &p = this->particles[i];
			p.Life -= deltaTime;
			if (p.Life > 0.0f) {
				p.Velocity += deltaTime * glm::vec3(0.0f, -9.81f, 0.0f) * 0.5f;
				p.Position += p.Velocity * deltaTime;
			}
		}
	}
	void Draw() {
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		this->shader.use();
		for (Particle particle : this->particles) {
			if (particle.Life > 0.0f) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, particle.Position);
				model = glm::scale(model, glm::vec3(0.03f));
				model = glm::rotate(model, glm::radians(particle.Angle), glm::vec3(1.0f, 0.3f, 0.5f));
				this->shader.setMat4("model", model);
				this->shader.setVec3("Color", particle.Color);
				if (SelfLight)
					this->shader.setBool("SelfLight", true);
				else
					this->shader.setBool("SelfLight", false);

				//this->texture.Bind();
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			}
		}
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
	}
};



