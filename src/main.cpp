// clang-format off
#include <glad/glad.h>
#include "camera.h"
#include "controls.h"
#include "deferred.h"
#include "forward.h"
#include <GLFW/glfw3.h>
#include <random>
#include <resource.h>
#include <glm/ext.hpp>
#include <stdint.h>
#include <vector>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glfwSwapInterval(0);



    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");


    // set callback function
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
        }, nullptr);

    ResourceManager resourceManager;
    Model model = resourceManager.loadModel("assets/sponza/Sponza.gltf");

    Camera camera(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(-4.0f, 2.0f, 0.0f));

    float distance = glm::length(camera.getPosition() - camera.getTarget());
    
    OrbitControl orbitControl(&camera, window, distance);

    // ForwardRenderer renderer;
    DeferredRenderer renderer;

    renderer.initResource(resourceManager);


    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        orbitControl.update(0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // renderer.gPass(model, camera, resourceManager);
        
        // renderer.useShadingProgram(resourceManager);
        // glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, sizeof(PointLight) * lights.size()); 
        renderer.render(model, camera, resourceManager);
     
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        double fps = 1.0 / deltaTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
          ImGui::Begin("FPS");

          ImGui::Text("fps %d", (int)fps);

          ImGui::End();
        }

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    resourceManager.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}
