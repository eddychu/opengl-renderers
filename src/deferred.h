#pragma once
#include "camera.h"
#include "resource.h"
#include "shape.h"
#include <iostream>
class DeferredRenderer {
public:
  void initResource(ResourceManager &resourceManager) {
    gPassProgramIndex =
        resourceManager.loadProgram("assets/shaders/deferred-g-pass.vert.glsl",
                                    "assets/shaders/deferred-g-pass.frag.glsl");

    shadingProgramIndex = resourceManager.loadProgram(
        "assets/shaders/deferred-shading.vert.glsl",
        "assets/shaders/deferred-shading.frag.glsl");

    const int SCR_WIDTH = 800;
    const int SCR_HEIGHT = 600;

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           gPosition, 0);

    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           gNormal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                           gAlbedo, 0);

    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                   GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH,
                          SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepth);

    if (glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "Framebuffer not complete!" << std::endl;
      throw std::runtime_error("Framebuffer not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    quad = uploadGeometryToGPU(make_quad());
  }

  void gPass(Model &model, Camera &camera, ResourceManager &resourceManager) {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto &program = resourceManager.getProgram(gPassProgramIndex);
    program.use();
    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 modelMatrix = model.transform.getMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 mvp = projection * view * modelMatrix;

    program.setUniform("MVP", mvp);
    program.setUniform("NormalMatrix",
                       glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    program.setUniform("ModelMatrix", modelMatrix);

    for (int i = 0; i < model.geometryCount; i++) {
      auto &baseColorTexture =
          resourceManager.getGPUTexture(model.materials[i].baseColorTexture);
      glBindTextureUnit(0, baseColorTexture.texture);
      auto &gpuGeometry =
          resourceManager.getGPUGeometry(i + model.geometryStart);
      gpuGeometry.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void useShadingProgram(ResourceManager &resourceManager) {
    auto &shadingProgram = resourceManager.getProgram(shadingProgramIndex);
    shadingProgram.use();
  }

  void render(Model &model, Camera &camera, ResourceManager &resourceManager) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto &shadingProgram = resourceManager.getProgram(shadingProgramIndex);
    // shadingProgram.use();
    // shadingProgram->setUniform("gPosition", 0);
    // shadingProgram->setUniform("gNormal", 1);
    // shadingProgram->setUniform("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    quad.draw();
    glUseProgram(0);
  }

private:
  uint32_t gPassProgramIndex;
  uint32_t shadingProgramIndex;
  GLuint gBuffer;
  GLuint gPosition, gNormal, gAlbedo;

  GLuint rboDepth;
  GPUGeometry quad;
};