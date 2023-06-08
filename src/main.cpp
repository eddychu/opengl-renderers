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
    
    // uint32_t forwardProgramIndex = resourceManager.loadProgram("assets/shaders/forward.vert.glsl", "assets/shaders/forward.frag.glsl");
    // glm::mat4 projection = camera.getProjectionMatrix();

    std::vector<PointLight> lights(200);


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-10.0f, 10.0f);
    std::uniform_real_distribution<float> disY(0.0f, 10.0f);
    std::uniform_real_distribution<float> disZ(-10.0f, 10.0f);
    // generate
    

    for(int i = 0; i < lights.size(); i++) {
        lights[i].position = glm::vec4(disX(gen), disY(gen), disZ(gen), 1.0f);
        lights[i].color = glm::vec4(glm::linearRand(glm::vec3(0.0f), glm::vec3(0.1f)), 1.0f);
        lights[i].intensity = glm::vec4(glm::linearRand(glm::vec3(0.0f), glm::vec3(0.1f)), 1.0f);
    }
    GLuint buffer;
    glCreateBuffers(1, &buffer);
    glNamedBufferData(buffer, sizeof(PointLight) * lights.size(), lights.data(), GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, sizeof(PointLight) * lights.size());



    double lastTime = glfwGetTime();
    float movingSpeedX = 0.2f;
    float movingSpeedZ = 0.2f;
    while (!glfwWindowShouldClose(window)) {
        orbitControl.update(0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(int i = 0; i < lights.size(); i++) {  
            if (lights[i].position.x > 10.0f) {
                movingSpeedX *= -1.0f;
            } else if(lights[i].position.x < -10.0f) {
                movingSpeedX *= -1.0f;
            }
            if (lights[i].position.z > 10.0f) {
                movingSpeedZ *= -1.0f;
            } else if(lights[i].position.z < -10.0f) {
                movingSpeedZ *= -1.0f;
            }
            lights[i].position.x += movingSpeedX;
            lights[i].position.z += movingSpeedZ;
        }
        glNamedBufferSubData(buffer, 0, sizeof(PointLight) * lights.size(), lights.data());

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

        // print camera position
        fprintf(stderr, "camera position: %s\n", glm::to_string(camera.getPosition()).c_str());
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
