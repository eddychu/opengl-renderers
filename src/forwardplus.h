#pragma once

#include "shape.h"
#pragma once

#include "camera.h"
#include "resource.h"
#include <random>
#include <spdlog/spdlog.h>
#include <stdint.h>

struct VisibleIndex {
  uint32_t index;
};

class ForwardPlusRenderer {

public:
  void initResource(ResourceManager &resourceManager) {
    uint32_t programIndex = resourceManager.loadProgram(
        "assets/shaders/forward.vert.glsl", "assets/shaders/forward.frag.glsl");

    uint32_t depthProgramIndex = resourceManager.loadProgram(
        "assets/shaders/depth.vert.glsl", "assets/shaders/depth.frag.glsl");

    uint32_t lightCullingProgramIndex = resourceManager.loadComputeProgram(
        "assets/shaders/lightculling.comp.glsl");

    uint32_t depthDebugProgramIndex =
        resourceManager.loadProgram("assets/shaders/depth-debug.vert.glsl",
                                    "assets/shaders/depth-debug.frag.glsl");

    uint32_t lightAccumulationProgramIndex = resourceManager.loadProgram(
        "assets/shaders/light-accumulation.vert.glsl",
        "assets/shaders/light-accumulation.frag.glsl");

    lights.resize(1024);

    std::random_device rd;
    std::mt19937 gen(rd());

    const glm::vec3 LIGHT_MIN_BOUNDS = glm::vec3(-10.0f, 0.0f, -10.0f);
    const glm::vec3 LIGHT_MAX_BOUNDS = glm::vec3(10.f, 10.0f, 10.0f);
    // generate

    for (int i = 0; i < lights.size(); i++) {
      lights[i].position =
          glm::vec4(glm::linearRand(LIGHT_MIN_BOUNDS, LIGHT_MAX_BOUNDS), 1.0);
      lights[i].color =
          glm::vec4(glm::linearRand(glm::vec3(0.1f), glm::vec3(0.6f)), 1.0f);
      lights[i].intensity =
          glm::vec4(glm::linearRand(glm::vec3(0.3f), glm::vec3(2.0f)), 20.0f);
    }
    glCreateBuffers(1, &lightBuffer);
    glNamedBufferData(lightBuffer, sizeof(PointLight) * lights.size(),
                      lights.data(), GL_DYNAMIC_DRAW);
    // glBindBufferRange(GL_UNIFORM_BUFFER, 0, lightBuffer, 0,
    //                   sizeof(PointLight) * lights.size());

    workGroupX = (800 + (800 % 16)) / 16;
    workGroupY = (600 + (600 % 16)) / 16;

    size_t numTiles = workGroupX * workGroupY;
    glCreateBuffers(1, &visibleLightBuffer);
    glNamedBufferData(visibleLightBuffer,
                      sizeof(VisibleIndex) * numTiles * 1024, NULL,
                      GL_STATIC_DRAW);

    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 800, 600, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTextureParameteri(depthMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(depthMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      spdlog::error("Framebuffer not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    program = &(resourceManager.getProgram(programIndex));
    depthProgram = &(resourceManager.getProgram(depthProgramIndex));
    lightCullingProgram =
        &(resourceManager.getProgram(lightCullingProgramIndex));
    depthDebugProgram = &(resourceManager.getProgram(depthDebugProgramIndex));

    lightAccumulationProgram =
        &(resourceManager.getProgram(lightAccumulationProgramIndex));

    lightCullingProgram->use();
    lightCullingProgram->setUniform("lightCount", 1024);
    lightCullingProgram->setUniform("screenSize", glm::ivec2(800, 600));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightBuffer);

    lightAccumulationProgram->use();
    lightAccumulationProgram->setUniform("numberOfTilesX", workGroupX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightBuffer);

    glUseProgram(0);

    quad = uploadGeometryToGPU(make_quad());
  }

  void render(Model &model, Camera &camera, ResourceManager &resourceManager) {

    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 modelMatrix = model.transform.getMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 mvp = projection * view * modelMatrix;

    depthProgram->use();
    depthProgram->setUniform("MVP", mvp);
    // depthProgram->setUniform("ModelMatrix", modelMatrix);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glViewport(0, 0, 800, 600);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < model.geometryCount; i++) {
      auto &gpuGeometry =
          resourceManager.getGPUGeometry(i + model.geometryStart);
      gpuGeometry.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightCullingProgram->use();
    lightCullingProgram->setUniform("projection", projection);
    lightCullingProgram->setUniform("view", view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glDispatchCompute(workGroupX, workGroupY, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightAccumulationProgram->use();
    lightAccumulationProgram->setUniform("MVP", mvp);
    lightAccumulationProgram->setUniform(
        "NormalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    lightAccumulationProgram->setUniform("ModelMatrix", modelMatrix);
    lightAccumulationProgram->setUniform("cameraPosition",
                                         camera.getPosition());

    for (int i = 0; i < model.geometryCount; i++) {
      auto &baseColorTexture =
          resourceManager.getGPUTexture(model.materials[i].baseColorTexture);
      glBindTextureUnit(0, baseColorTexture.texture);
      auto &gpuGeometry =
          resourceManager.getGPUGeometry(i + model.geometryStart);
      gpuGeometry.draw();
    }

    // depthDebugProgram->use();
    // // depthDebugProgram->setUniform("nearPlane", camera.nearPlane);
    // // depthDebugProgram->setUniform("farPlane", camera.farPlane);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthMap);
    // glViewport(0, 0, 800, 600);
    // glClear(GL_COLOR_BUFFER_BIT);
    // quad.draw();

    // program->use();
    // program->setUniform("MVP", mvp);
    // program->setUniform("NormalMatrix",
    //                     glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    // program->setUniform("ModelMatrix", modelMatrix);

    // for (int i = 0; i < model.geometryCount; i++) {
    //   auto &baseColorTexture =
    //       resourceManager.getGPUTexture(model.materials[i].baseColorTexture);
    //   glBindTextureUnit(0, baseColorTexture.texture);
    //   auto &gpuGeometry =
    //       resourceManager.getGPUGeometry(i + model.geometryStart);
    //   gpuGeometry.draw();
    // }
  }

private:
  const Program *program;
  GLuint lightBuffer;
  GLuint visibleLightBuffer;
  GLuint depthMapFBO;
  GLuint depthMap;
  const Program *depthProgram;
  const Program *lightCullingProgram;
  const Program *depthDebugProgram;
  const Program *lightAccumulationProgram;

  GPUGeometry quad;

  std::vector<PointLight> lights;
  float movingSpeedX = 0.1f;
  float movingSpeedZ = 0.1f;
  int workGroupX{};
  int workGroupY{};
};