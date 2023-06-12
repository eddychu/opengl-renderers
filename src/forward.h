#pragma once

#include "camera.h"
#include "resource.h"
#include <random>
#include <stdint.h>

class ForwardRenderer {

public:
  void initResource(ResourceManager &resourceManager) {
    uint32_t programIndex = resourceManager.loadProgram(
        "assets/shaders/forward.vert.glsl", "assets/shaders/forward.frag.glsl");
    program = &(resourceManager.getProgram(programIndex));

    lights.resize(1024);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-10.0f, 10.0f);
    std::uniform_real_distribution<float> disY(0.0f, 10.0f);
    std::uniform_real_distribution<float> disZ(-10.0f, 10.0f);
    // generate

    for (int i = 0; i < lights.size(); i++) {
      lights[i].position = glm::vec4(disX(gen), disY(gen), disZ(gen), 1.0f);
      lights[i].color =
          glm::vec4(glm::linearRand(glm::vec3(0.6f), glm::vec3(1.0f)), 1.0f);
      lights[i].intensity =
          glm::vec4(glm::linearRand(glm::vec3(0.3f), glm::vec3(2.0f)), 1.0f);
    }
    glCreateBuffers(1, &lightBuffer);
    glNamedBufferData(lightBuffer, sizeof(PointLight) * lights.size(),
                      lights.data(), GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, lightBuffer, 0,
                      sizeof(PointLight) * lights.size());
  }

  void render(Model &model, Camera &camera, ResourceManager &resourceManager) {
    program->use();
    for (int i = 0; i < lights.size(); i++) {
      if (lights[i].position.x > 5.0f) {
        movingSpeedX *= -1.0f;
      } else if (lights[i].position.x < -5.0f) {
        movingSpeedX *= -1.0f;
      }
      if (lights[i].position.z > 5.0f) {
        movingSpeedZ *= -1.0f;
      } else if (lights[i].position.z < -5.0f) {
        movingSpeedZ *= -1.0f;
      }
      lights[i].position.x += movingSpeedX;
      lights[i].position.z += movingSpeedZ;
    }
    glNamedBufferSubData(lightBuffer, 0, sizeof(PointLight) * lights.size(),
                         lights.data());

    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 modelMatrix = model.transform.getMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 mvp = projection * view * modelMatrix;

    program->setUniform("MVP", mvp);
    program->setUniform("NormalMatrix",
                        glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    program->setUniform("ModelMatrix", modelMatrix);

    for (int i = 0; i < model.geometryCount; i++) {
      auto &baseColorTexture =
          resourceManager.getGPUTexture(model.materials[i].baseColorTexture);
      glBindTextureUnit(0, baseColorTexture.texture);
      auto &gpuGeometry =
          resourceManager.getGPUGeometry(i + model.geometryStart);
      gpuGeometry.draw();
    }
  }

private:
  const Program *program;
  GLuint lightBuffer;

  std::vector<PointLight> lights;
  float movingSpeedX = 0.1f;
  float movingSpeedZ = 0.1f;
};