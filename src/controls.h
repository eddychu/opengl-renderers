#pragma once

#pragma once
#include <GLFW/glfw3.h>
#include <camera.h>
#include <glm/glm.hpp>

class OrbitControl {
public:
  OrbitControl(Camera *camera, GLFWwindow *window, float radius = 5.0f)
      : mCamera(camera), mWindow(window), mRadius(radius) {
    // get theta and phi from current rotation

    mTheta = glm::atan(mCamera->getPosition().z, mCamera->getPosition().x);
    mPhi = glm::asin(mCamera->getPosition().y / mRadius);
  }
  void rotate(double dx, double dy) {
    mTheta += dx * 0.01f;
    mPhi += dy * 0.01f;
    mPhi = glm::clamp(mPhi, -glm::half_pi<float>() + 0.01f,
                      glm::half_pi<float>() - 0.01f);

    float x = mRadius * glm::cos(mPhi) * glm::cos(mTheta);
    float y = mRadius * glm::sin(mPhi);
    float z = mRadius * glm::cos(mPhi) * glm::sin(mTheta);

    mCamera->setPosition(glm::vec3(x, y, z));
  }

  void update(float dt) {
    if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      if (!enabled) {
        enabled = true;
        double x, y;
        glfwGetCursorPos(mWindow, &x, &y);
        lastCursorPos[0] = x;
        lastCursorPos[1] = y;
        return;
      }
    } else {
      if (enabled) {
        enabled = false;
        return;
      }
    }

    // use w to zoom in and s to zoom out
    if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
      // update camera position

      mRadius -= 0.1f;
      float x = mRadius * glm::cos(mPhi) * glm::cos(mTheta);
      float y = mRadius * glm::sin(mPhi);
      float z = mRadius * glm::cos(mPhi) * glm::sin(mTheta);

      mCamera->setPosition(glm::vec3(x, y, z));
    }

    if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
      mRadius += 0.1f;
      float x = mRadius * glm::cos(mPhi) * glm::cos(mTheta);
      float y = mRadius * glm::sin(mPhi);
      float z = mRadius * glm::cos(mPhi) * glm::sin(mTheta);

      mCamera->setPosition(glm::vec3(x, y, z));
    }

    if (enabled) {

      double x, y;
      glfwGetCursorPos(mWindow, &x, &y);
      double dx = x - lastCursorPos[0];
      double dy = y - lastCursorPos[1];
      // if (dx == 0.0 && dy == 0.0)
      //   return;
      rotate(dx, dy);
      lastCursorPos[0] = x;
      lastCursorPos[1] = y;
    }
  }

private:
  float mRadius = 0.0f;
  Camera *mCamera;
  GLFWwindow *mWindow;
  bool enabled = false;
  double lastCursorPos[2];

  float mTheta = 0.0f;
  float mPhi = 0.0f;
};