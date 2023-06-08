#pragma once
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  glm::vec3 tangent;
};

struct Geometry {
  std::vector<Vertex> vertices{};
  std::vector<unsigned int> indices{};

  void calcTangents();
};

struct GPUGeometry {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  unsigned int numIndices;
  void draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  }
};

GPUGeometry uploadGeometryToGPU(const Geometry &geometry);

struct Material {
  glm::vec4 baseColorFactor;
  int32_t baseColorTexture;
  int32_t normalTexture;
  int32_t metallicRoughnessTexture;
  float alphaCutoff;
  bool doubleSided;
};

struct Transform {
  glm::vec3 position{0.0f};
  glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f};

  glm::mat4 getMatrix() const {
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, position);
    matrix *= glm::mat4_cast(rotation);
    matrix = glm::scale(matrix, scale);
    return matrix;
  }
};

struct Model {
  uint32_t geometryStart;
  uint32_t geometryCount;
  std::vector<Material> materials;
  Transform transform;
};

typedef std::vector<unsigned char> LDR;
typedef std::vector<float> HDR;

typedef std::variant<LDR, HDR> PixelDataArray;

struct Sampler {
  enum class Filter {
    Nearest,
    Linear,
  };
  enum class Wrap { Repeat, MirroredRepeat, ClampToEdge, ClampToBorder };

  static Filter filter_from_int(int i) {
    switch (i) {
    case 9728:
      return Filter::Nearest;
    case 9729:
      return Filter::Linear;
    default:
      return Filter::Nearest;
    }
  }

  static Wrap wrap_from_int(int i) {
    switch (i) {
    case 10497:
      return Wrap::Repeat;
    case 33648:
      return Wrap::MirroredRepeat;
    case 33071:
      return Wrap::ClampToEdge;
    case 33069:
      return Wrap::ClampToBorder;
    default:
      return Wrap::Repeat;
    }
  }

  Filter min_filter{Filter::Nearest};
  Filter mag_filter{Filter::Nearest};
  Wrap wrap_s{Wrap::Repeat};
  Wrap wrap_t{Wrap::Repeat};
  Wrap wrap_r{Wrap::Repeat};
};

struct Texture {
  uint32_t width;
  uint32_t height;
  uint32_t component;
  PixelDataArray data;
  Sampler sampler;

  void loadFromFile(const std::string &path, bool flip = false);
};

typedef std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4,
                     glm::mat3, glm::ivec2>
    GLUniformValue;

struct Program {
  GLuint program;

  std::unordered_map<std::string, GLuint> uniforms;

  void use() const { glUseProgram(program); }

  void setUniform(const std::string &name, GLUniformValue value) const;

  void loadFromFile(const std::string &vertexPath,
                    const std::string &fragmentPath);

  void populateUniforms();
};

class ResourceManager {
public:
  Model loadModel(const std::string &path);

  uint32_t loadProgram(const std::string &vertexPath,
                       const std::string &fragmentPath);

  uint32_t uploadGeometry(uint32_t i);

  void addTexture(const Texture &texture) { textures.push_back(texture); }

  const GPUGeometry &getGPUGeometry(uint32_t i) const {
    return gpuGeometries[i];
  }

  const Program &getProgram(uint32_t i) const { return programs[i]; }

  void cleanup();

private:
  std::vector<Geometry> geometries;
  std::vector<Texture> textures;
  std::vector<Program> programs;
  std::vector<GPUGeometry> gpuGeometries;
};

GLuint createShader(const std::string &path, GLenum type);

GLuint createProgram(const std::string &vertexPath,
                     const std::string &fragmentPath);
