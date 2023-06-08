#pragma once
#include <glm/ext.hpp>
#include <glm/glm.hpp>

class Camera {
public:
  Camera(float fovInDegree, float aspect, float near, float far) {
    projection = glm::perspective(glm::radians(fovInDegree), aspect, near, far);
    view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
  }

  const glm::mat4 &getProjectionMatrix() const { return projection; }
  const glm::mat4 &getViewMatrix() const { return view; }

  const glm::vec3 &getPosition() const { return position; }

  const glm::vec3 &getTarget() const { return target; }

  void setPosition(const glm::vec3 &position) {
    this->position = position;
    updateViewMatrix();
  }
  void setTarget(const glm::vec3 &target) {
    this->target = target;
    updateViewMatrix();
  }
  void setPositionAndTarget(const glm::vec3 &position,
                            const glm::vec3 &target) {
    this->position = position;
    this->target = target;
    updateViewMatrix();
  }

  void updateViewMatrix() {
    view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
  }

private:
  glm::mat4 projection;
  glm::mat4 view;
  glm::vec3 position{0.0f, 0.0f, 5.0f};
  glm::vec3 target{0.0f, 0.0f, 0.0f};
};