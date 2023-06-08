// clang-format off
#include <glad/glad.h>
#include "camera.h"
#include "controls.h"
#include <GLFW/glfw3.h>
#include <resource.h>
#include <glm/ext.hpp>
#include <stdint.h>
#include <vector>


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // multi sampling
    glfwWindowHint(GLFW_SAMPLES, 16);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    // set callback function
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
        }, nullptr);

    ResourceManager resourceManager;
    Model model = resourceManager.loadModel("assets/sponza/Sponza.gltf");

    std::vector<uint32_t> gpuGeometries;
    gpuGeometries.resize(model.geometryCount);
    for(int i = 0; i < model.geometryCount; i++) {
        gpuGeometries[i] = resourceManager.uploadGeometry(model.geometryStart + i);
    }

    Camera camera(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 50.0f, 50.0f));

    float distance = glm::length(camera.getPosition() - camera.getTarget());
    
    OrbitControl orbitControl(&camera, window, distance);


    
    uint32_t programIndex = resourceManager.loadProgram("assets/shaders/forward.vert.glsl", "assets/shaders/forward.frag.glsl");
    glm::mat4 projection = camera.getProjectionMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    auto& program=  resourceManager.getProgram(programIndex); 
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        program.use();

        orbitControl.update(0.1f);
        glm::mat4 modelMatrix = model.transform.getMatrix();
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 mvp = projection * view * modelMatrix;

        program.setUniform("MVP", mvp);
        program.setUniform("NormalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
        program.setUniform("ModelMatrix", modelMatrix);

        for(int i = 0; i < model.geometryCount; i++) {
            auto& gpuGeometry = resourceManager.getGPUGeometry(gpuGeometries[i]);
            gpuGeometry.draw();
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    resourceManager.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}
