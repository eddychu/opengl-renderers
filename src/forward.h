#pragma once

#include "camera.h"
#include "resource.h"
#include <stdint.h>

class ForwardRenderer {

public:
  void initResource(ResourceManager &resourceManager) {
    uint32_t programIndex = resourceManager.loadProgram(
        "assets/shaders/forward.vert.glsl", "assets/shaders/forward.frag.glsl");
    program = &(resourceManager.getProgram(programIndex));
  }

  void render(Model &model, Camera &camera, ResourceManager &resourceManager) {

    // glBindBufferRange(GL_UNIFORM_BUFFER, 0, lightBuffer.buffer, 0,
    // sizeof(PointLight) * lights.size());

    program->use();
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
};