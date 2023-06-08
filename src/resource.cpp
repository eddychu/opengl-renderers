
#include <fstream>
#include <iostream>
#include <istream>
#include <resource.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <spdlog/spdlog.h>
#include <tiny_gltf.h>

#include <utils/calc_tangent.h>
void Geometry::calcTangents() {
  CalcTangents calcTangents;
  calcTangents.calc(this);
}

GPUGeometry uploadGeometryToGPU(const Geometry &geometry) {
  GLuint vao, vbo, ebo;
  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);
  glCreateBuffers(1, &ebo);

  glNamedBufferStorage(vbo, geometry.vertices.size() * sizeof(Vertex),
                       geometry.vertices.data(), 0);

  glNamedBufferStorage(ebo, geometry.indices.size() * sizeof(unsigned int),
                       geometry.indices.data(), 0);

  glVertexArrayElementBuffer(vao, ebo);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, normal));
  glVertexArrayAttribBinding(vao, 1, 0);

  glEnableVertexArrayAttrib(vao, 2);
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, texCoords));
  glVertexArrayAttribBinding(vao, 2, 0);

  glEnableVertexArrayAttrib(vao, 3);
  glVertexArrayAttribFormat(vao, 3, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, tangent));
  glVertexArrayAttribBinding(vao, 3, 0);

  unsigned int numIndices = geometry.indices.size();
  return {vao, vbo, ebo, numIndices};
}

Texture createTexture(const tinygltf::Model &model, const int texture_index) {
  const auto &gltfTexture = model.textures[texture_index];
  const auto &gltfImage = model.images[gltfTexture.source];
  const auto &gltfSampler = model.samplers[gltfTexture.sampler];
  Texture texture;
  texture.width = gltfImage.width;
  texture.height = gltfImage.height;
  texture.sampler.wrap_s = Sampler::wrap_from_int(gltfSampler.wrapS);
  texture.sampler.wrap_t = Sampler::wrap_from_int(gltfSampler.wrapT);
  texture.sampler.min_filter = Sampler::filter_from_int(gltfSampler.minFilter);
  texture.sampler.mag_filter = Sampler::filter_from_int(gltfSampler.magFilter);
  texture.data = std::move(gltfImage.image);
  texture.component = gltfImage.component;
  return texture;
}

static GLenum get_format_from_component(int component) {
  switch (component) {
  case 1:
    return GL_RED;
  case 2:
    return GL_RG;
  case 3:
    return GL_RGB;
  case 4:
    return GL_RGBA;
  default:
    spdlog::error("Invalid texture component {}", component);
    throw std::runtime_error("Invalid texture component");
  }
}

static GLenum get_internal_format_from_component(int component) {
  switch (component) {
  case 1:
    return GL_R8;
  case 2:
    return GL_RG8;
  case 3:
    return GL_RGB8;
  case 4:
    return GL_RGBA8;
  default:
    spdlog::error("Invalid texture component: {}", component);
    throw std::runtime_error("Invalid texture component");
  }
}

static GLenum get_type_from_texture(const PixelDataArray &data) {
  if (std::holds_alternative<LDR>(data)) {
    return GL_UNSIGNED_BYTE;
  } else {
    return GL_FLOAT;
  }
}

static GLenum get_filter(Sampler::Filter filter) {
  switch (filter) {
  case Sampler::Filter::Nearest:
    return GL_NEAREST;
  case Sampler::Filter::Linear:
    return GL_LINEAR;
  default:
    spdlog::error("Invalid filter ");
    throw std::runtime_error("Invalid filter");
  }
}

static GLenum get_wrap(Sampler::Wrap wrap) {
  switch (wrap) {
  case Sampler::Wrap::Repeat:
    return GL_REPEAT;
  case Sampler::Wrap::MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  case Sampler::Wrap::ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case Sampler::Wrap::ClampToBorder:
    return GL_CLAMP_TO_BORDER;
  default:
    spdlog::error("Invalid wrap ");
    throw std::runtime_error("Invalid wrap");
  }
}

GPUTexture uploadTextureToGPU(const Texture &texture) {
  GLenum target = GL_TEXTURE_2D;
  GLenum internal_format =
      get_internal_format_from_component(texture.component);
  GLenum format = get_format_from_component(texture.component);
  GLenum type = get_type_from_texture(texture.data);
  void *data = nullptr;
  if (std::holds_alternative<LDR>(texture.data)) {
    data = (void *)std::get<LDR>(texture.data).data();
  } else {
    data = (void *)std::get<HDR>(texture.data).data();
  }

  GLenum min_filter = get_filter(texture.sampler.min_filter);
  GLenum mag_filter = get_filter(texture.sampler.mag_filter);
  GLenum wrap_s = get_wrap(texture.sampler.wrap_s);
  GLenum wrap_t = get_wrap(texture.sampler.wrap_t);
  GLuint mHandle;
  glCreateTextures(GL_TEXTURE_2D, 1, &mHandle);
  glTextureStorage2D(mHandle, 1, internal_format, texture.width,
                     texture.height);
  glTextureSubImage2D(mHandle, 0, 0, 0, texture.width, texture.height, format,
                      type, data);
  glTextureParameteri(mHandle, GL_TEXTURE_MIN_FILTER, min_filter);
  glTextureParameteri(mHandle, GL_TEXTURE_MAG_FILTER, mag_filter);
  glTextureParameteri(mHandle, GL_TEXTURE_WRAP_S, wrap_s);
  glTextureParameteri(mHandle, GL_TEXTURE_WRAP_T, wrap_t);
  return {mHandle};
}

Model ResourceManager::loadModel(const std::string &path) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string err;
  std::string warn;

  std::string ext = path.substr(path.find_last_of(".") + 1);

  bool ret = false;

  if (ext.compare("glb") == 0) {
    ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.c_str());
  } else {
    ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.c_str());
  }

  if (!warn.empty()) {
    spdlog::warn("{}", warn);
  }

  if (!err.empty()) {
    spdlog::error("{}", err);

    throw std::runtime_error("Failed to load scene");
  }

  //   const auto &gltfScene =
  //       model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];

  const auto &defaultScene = model.scenes[0];

  const auto &node = model.nodes[defaultScene.nodes[0]];

  Transform transform;

  if (node.translation.size() == 3) {
    transform.position = glm::make_vec3(node.translation.data());
  }
  if (node.rotation.size() == 4) {
    transform.rotation = glm::make_quat(node.rotation.data());
  }
  if (node.scale.size() == 3) {
    transform.scale = glm::make_vec3(node.scale.data());
  }

  const auto &gltfMesh = model.meshes[node.mesh];

  uint32_t start = gpuGeometries.size();
  uint32_t count = gltfMesh.primitives.size();
  gpuGeometries.resize(start + count);
  std::vector<Material> materials(count);
  for (int i = 0; i < count; i++) {
    Geometry currentGeometry;

    const auto &primitive = gltfMesh.primitives[i];
    const float *positionData = nullptr;
    const float *normalData = nullptr;
    const float *texCoordData = nullptr;

    uint32_t posByteStride = 0;
    uint32_t normalByteStride = 0;
    uint32_t texCoordByteStride = 0;

    const tinygltf::Accessor &posAccessor =
        model.accessors[primitive.attributes.find("POSITION")->second];
    const tinygltf::BufferView &posBufferView =
        model.bufferViews[posAccessor.bufferView];
    const tinygltf::Accessor &normalAccessor =
        model.accessors[primitive.attributes.find("NORMAL")->second];
    const tinygltf::BufferView &normalBufferView =
        model.bufferViews[normalAccessor.bufferView];
    const tinygltf::Accessor &texCoordAccessor =
        model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
    const tinygltf::BufferView &texCoordBufferView =
        model.bufferViews[texCoordAccessor.bufferView];

    positionData = reinterpret_cast<const float *>(
        &model.buffers[posBufferView.buffer]
             .data[posAccessor.byteOffset + posBufferView.byteOffset]);

    uint32_t vertexCount = static_cast<uint32_t>(posAccessor.count);

    posByteStride = posAccessor.ByteStride(posBufferView)
                        ? posAccessor.ByteStride(posBufferView) / sizeof(float)
                        : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

    normalData = reinterpret_cast<const float *>(
        &model.buffers[normalBufferView.buffer]
             .data[normalAccessor.byteOffset + normalBufferView.byteOffset]);

    normalByteStride =
        normalAccessor.ByteStride(normalBufferView)
            ? normalAccessor.ByteStride(normalBufferView) / sizeof(float)
            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

    texCoordData = reinterpret_cast<const float *>(
        &model.buffers[texCoordBufferView.buffer]
             .data[texCoordAccessor.byteOffset +
                   texCoordBufferView.byteOffset]);

    texCoordByteStride =
        texCoordAccessor.ByteStride(texCoordBufferView)
            ? texCoordAccessor.ByteStride(texCoordBufferView) / sizeof(float)
            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
    std::vector<Vertex> vertices(vertexCount);

    for (uint32_t v = 0; v < vertexCount; v++) {
      vertices[v].position = glm::make_vec3(&positionData[v * posByteStride]);
      vertices[v].normal = glm::make_vec3(&normalData[v * normalByteStride]);
      vertices[v].texCoords =
          glm::make_vec2(&texCoordData[v * texCoordByteStride]);
    }

    std::vector<uint32_t> indices;
    const tinygltf::Accessor &indexAccessor =
        model.accessors[primitive.indices];

    const tinygltf::BufferView &indexBufferView =
        model.bufferViews[indexAccessor.bufferView];

    const tinygltf::Buffer &buffer = model.buffers[indexBufferView.buffer];

    indices.resize(indexAccessor.count);

    const void *data_ptr =
        &buffer.data[indexAccessor.byteOffset + indexBufferView.byteOffset];

    switch (indexAccessor.componentType) {
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
      const uint32_t *buf = static_cast<const uint32_t *>(data_ptr);
      for (size_t index = 0; index < indexAccessor.count; index++) {
        indices[index] = buf[index];
      }
      break;
    }

    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
      const uint16_t *buf = static_cast<const uint16_t *>(data_ptr);
      for (size_t index = 0; index < indexAccessor.count; index++) {
        indices[index] = buf[index];
      }
      break;
    }

    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
      const uint8_t *buf = static_cast<const uint8_t *>(data_ptr);
      for (size_t index = 0; index < indexAccessor.count; index++) {
        indices[index] = buf[index];
      }
      break;
    }
    }
    currentGeometry.vertices = vertices;
    currentGeometry.indices = indices;
    currentGeometry.calcTangents();

    gpuGeometries[start + i] = uploadGeometryToGPU(currentGeometry);

    auto &finalMaterial = materials[i];

    const auto &material = model.materials[primitive.material];

    const auto &pbrMetallicRoughness = material.pbrMetallicRoughness;

    const auto &baseColorFactor = pbrMetallicRoughness.baseColorFactor;

    if (pbrMetallicRoughness.baseColorTexture.index > -1) {

      Texture texture =
          createTexture(model, pbrMetallicRoughness.baseColorTexture.index);
      gpuTextures.push_back(uploadTextureToGPU(texture));
      finalMaterial.baseColorTexture = gpuTextures.size() - 1;
      // finalMaterial.baseColorTexture = textures.size() - 1;
    }

    if (material.normalTexture.index > -1) {
      Texture texture = createTexture(model, material.normalTexture.index);
      gpuTextures.push_back(uploadTextureToGPU(texture));
      finalMaterial.normalTexture = gpuTextures.size() - 1;
    }

    // if (material.emissiveTexture.index > -1) {
    //   finalMaterial->emissiveTexture =
    //       createTexture(model, material.emissiveTexture.index);
    // }

    // if (material.occlusionTexture.index > -1) {
    //   finalMaterial->occlusionTexture =
    //       createTexture(model, material.occlusionTexture.index);
    // }

    if (pbrMetallicRoughness.metallicRoughnessTexture.index > -1) {
      Texture texture = createTexture(
          model, pbrMetallicRoughness.metallicRoughnessTexture.index);
      gpuTextures.push_back(uploadTextureToGPU(texture));
      finalMaterial.metallicRoughnessTexture = gpuTextures.size() - 1;
    }

    finalMaterial.baseColorFactor = glm::make_vec4(baseColorFactor.data());
    finalMaterial.alphaCutoff = material.alphaCutoff;
    finalMaterial.doubleSided = material.doubleSided;
  }
  return {0, count, materials, transform};
}

uint32_t ResourceManager::loadProgram(const std::string &vertexPath,
                                      const std::string &fragmentPath) {
  Program program;
  program.loadFromFile(vertexPath, fragmentPath);
  programs.push_back(program);
  return programs.size() - 1;
}

void ResourceManager::cleanup() {
  // geometries.clear();
  // textures.clear();
  for (auto &gpuGeometry : gpuGeometries) {
    glDeleteVertexArrays(1, &gpuGeometry.vao);
    glDeleteBuffers(1, &gpuGeometry.vbo);
    glDeleteBuffers(1, &gpuGeometry.ebo);
  }

  gpuGeometries.clear();
  for (auto &program : programs) {
    glDeleteProgram(program.program);
  }
  programs.clear();

  for (auto &gpuTexture : gpuTextures) {
    glDeleteTextures(1, &gpuTexture.texture);
  }
  gpuTextures.clear();
}

GLuint createShader(const std::string &path, GLenum type) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file: " + path);
  }
  std::stringstream ss;
  ss << file.rdbuf();
  std::string source = ss.str();
  const char *csource = source.c_str();
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &csource, NULL);
  glCompileShader(shader);
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    spdlog::error("Failed to compile shader: {}\n{}", path, infoLog);
    throw std::runtime_error("Failed to compile shader: " + path + "\n" +
                             infoLog);
  }
  return shader;
}

GLuint createProgram(const std::string &vertexPath,
                     const std::string &fragmentPath) {
  GLuint vertexShader = createShader(vertexPath, GL_VERTEX_SHADER);
  GLuint fragmentShader = createShader(fragmentPath, GL_FRAGMENT_SHADER);
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    spdlog::error("Failed to link program: {}, {}\n{}", vertexPath,
                  fragmentPath, infoLog);
    throw std::runtime_error("Failed to link program: " + vertexPath + ", " +
                             fragmentPath + "\n" + infoLog);
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}

void Program::loadFromFile(const std::string &vertexPath,
                           const std::string &fragmentPath) {
  program = createProgram(vertexPath, fragmentPath);
  populateUniforms();
}

void Program::populateUniforms() {
  glUseProgram(program);
  GLint numUniforms = 0;
  glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES,
                          &numUniforms);

  GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

  for (GLint i = 0; i < numUniforms; ++i) {
    GLint results[4];
    glGetProgramResourceiv(program, GL_UNIFORM, i, 4, properties, 4, NULL,
                           results);

    if (results[3] != -1)
      continue; // Skip uniforms in blocks
    GLint nameBufSize = results[0] + 1;
    char *name = new char[nameBufSize];
    glGetProgramResourceName(program, GL_UNIFORM, i, nameBufSize, NULL, name);
    uniforms[name] = results[2];
    delete[] name;
  }

  // log all uniforms
  for (auto &uniform : uniforms) {
    printf("Uniform %s at location %d\n", uniform.first.c_str(),
           uniform.second);
  }

  glUseProgram(0);
}

void Program::setUniform(const std::string &name, GLUniformValue value) const {
  if (uniforms.find(name) == uniforms.end()) {
    spdlog::error("Uniform {} not found", name);
    return;
  }

  glUseProgram(program);

  switch (value.index()) {
  case 0:
    glUniform1i(uniforms.at(name), std::get<int>(value));
    break;
  case 1:
    glUniform1f(uniforms.at(name), std::get<float>(value));
    break;
  case 2:
    glUniform2fv(uniforms.at(name), 1, &std::get<glm::vec2>(value)[0]);
    break;
  case 3:
    glUniform3fv(uniforms.at(name), 1, &std::get<glm::vec3>(value)[0]);
    break;
  case 4:
    glUniform4fv(uniforms.at(name), 1, &std::get<glm::vec4>(value)[0]);
    break;
  case 5:
    glUniformMatrix4fv(uniforms.at(name), 1, GL_FALSE,
                       &std::get<glm::mat4>(value)[0][0]);
    break;
  case 6:
    glUniformMatrix3fv(uniforms.at(name), 1, GL_FALSE,
                       &std::get<glm::mat3>(value)[0][0]);
    break;
  case 7:
    glUniform2iv(uniforms.at(name), 1, &std::get<glm::ivec2>(value)[0]);
    break;
  default:
    spdlog::error("Uniform type not supported");
    break;
  }
}