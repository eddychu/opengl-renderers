#pragma once
#include "camera.h"
#include "resource.h"
#include "shape.h"
#include <iostream>
class DeferredRenderer {
public:
  void initResource(ResourceManager &resourceManager) {
    uint32_t programIndex =
        resourceManager.loadProgram("assets/shaders/deferred-g-pass.vert.glsl",
                                    "assets/shaders/deferred-g-pass.frag.glsl");
    gPassProgram = &(resourceManager.getProgram(programIndex));

    programIndex = resourceManager.loadProgram(
        "assets/shaders/deferred-shading.vert.glsl",
        "assets/shaders/deferred-shading.frag.glsl");

    shadingProgram = &(resourceManager.getProgram(programIndex));

    glCreateFramebuffers(1, &gBuffer);
    glCreateTextures(GL_TEXTURE_2D, 1, &gPosition);
    glTextureStorage2D(gPosition, 1, GL_RGBA16F, 800, 600);
    glTextureSubImage2D(gPosition, 0, 0, 0, 800, 600, GL_RGBA, GL_FLOAT, NULL);
    glTextureParameteri(gPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, gPosition, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &gNormal);
    glTextureStorage2D(gNormal, 1, GL_RGBA16F, 800, 600);
    glTextureSubImage2D(gNormal, 0, 0, 0, 800, 600, GL_RGBA, GL_FLOAT, NULL);
    glTextureParameteri(gNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, gNormal, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &gAlbedo);
    glTextureStorage2D(gAlbedo, 1, GL_RGBA, 800, 600);
    glTextureSubImage2D(gAlbedo, 0, 0, 0, 800, 600, GL_RGBA, GL_FLOAT, NULL);
    glTextureParameteri(gAlbedo, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(gAlbedo, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT2, gAlbedo, 0);

    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2};

    glNamedFramebufferDrawBuffers(gBuffer, 3, attachments);

    if (glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "Framebuffer not complete!" << std::endl;
    }

    quad = uploadGeometryToGPU(make_quad());
  }

  void gPass(Model &model, Camera &camera, ResourceManager &resourceManager) {
    glViewport(0, 0, 800, 600);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gPassProgram->use();
    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 modelMatrix = model.transform.getMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 mvp = projection * view * modelMatrix;

    gPassProgram->setUniform("MVP", mvp);
    gPassProgram->setUniform(
        "NormalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    gPassProgram->setUniform("ModelMatrix", modelMatrix);

    for (int i = 0; i < model.geometryCount; i++) {
      auto &baseColorTexture =
          resourceManager.getGPUTexture(model.materials[i].baseColorTexture);
      glBindTextureUnit(0, baseColorTexture.texture);
      auto &gpuGeometry =
          resourceManager.getGPUGeometry(i + model.geometryStart);
      gpuGeometry.draw();
    }
    glUseProgram(0);
  }

  void render(Model &model, Camera &camera, ResourceManager &resourceManager) {
    gPass(model, camera, resourceManager);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadingProgram->use();
    // shadingProgram->setUniform("gPosition", 0);
    // shadingProgram->setUniform("gNormal", 1);
    // shadingProgram->setUniform("gAlbedo", 2);
    glBindTextureUnit(0, gPosition);
    glBindTextureUnit(1, gNormal);
    glBindTextureUnit(2, gAlbedo);
    quad.draw();
    glUseProgram(0);
  }

private:
  const Program *gPassProgram;
  const Program *shadingProgram;
  GLuint gBuffer;
  GLuint gPosition, gNormal, gAlbedo;
  GPUGeometry quad;
};