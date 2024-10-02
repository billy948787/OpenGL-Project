#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#ifdef __APPLE__  // MacOS
// include glew first to avoid compile error
#include <GL/glew.h>
// include freeglut.h after glew.h
#include <GL/freeglut.h>

// GLM.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
// include glew first to avoid compile error
#include <glew.h>
// include freeglut.h after glew.h
#include <freeglut.h>

// GLM.
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#endif

// C++ STL headers.
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// VertexPTN Declarations.
struct VertexPTN {
  VertexPTN() {
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    normal = glm::vec3(0.0f, 1.0f, 0.0f);
    texcoord = glm::vec2(0.0f, 0.0f);
  }
  VertexPTN(glm::vec3 p, glm::vec3 n, glm::vec2 uv) {
    position = p;
    normal = n;
    texcoord = uv;
  }
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texcoord;

  // operator overloading
  bool operator==(const VertexPTN& other) const {
    return position == other.position && normal == other.normal &&
           texcoord == other.texcoord;
  }
};

namespace std {
template <>
struct hash<VertexPTN> {
  size_t operator()(const VertexPTN& vertex) const {
    size_t h1 = std::hash<float>()(vertex.position.x);
    size_t h2 = std::hash<float>()(vertex.position.y);
    size_t h3 = std::hash<float>()(vertex.position.z);
    size_t h4 = std::hash<float>()(vertex.normal.x);
    size_t h5 = std::hash<float>()(vertex.normal.y);
    size_t h6 = std::hash<float>()(vertex.normal.z);
    size_t h7 = std::hash<float>()(vertex.texcoord.x);
    size_t h8 = std::hash<float>()(vertex.texcoord.y);

    // 組合哈希值
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^
           (h7 << 6) ^ (h8 << 7);
  }
};
}  // namespace std

// TriangleMesh Declarations.
class TriangleMesh {
 public:
  // TriangleMesh Public Methods.
  TriangleMesh();
  ~TriangleMesh();

  // Load the model from an *.OBJ file.
  bool LoadFromFile(const std::string& filePath, const bool normalized = true);

  void processLine(std::vector<glm::vec3>& points, std::vector<glm::vec2>& texs,
                   std::vector<glm::vec3>& normals,
                   std::vector<std::string> parts);

  // Create vertex and index buffers.
  void CreateBuffers();

  // Apply transform on CPU.
  void ApplyTransformCPU(const glm::mat4x4& mvpMatrix);

  int GetNumVertices() const { return numVertices; }
  int GetNumTriangles() const { return numTriangles; }
  int GetNumIndices() const { return (int)vertexIndices.size(); }
  glm::vec3 GetObjCenter() const { return objCenter; }

 private:
  friend void RenderSceneCB();
  // TriangleMesh Private Methods.
  void PrintMeshInfo() const;

  // TriangleMesh Private Data.
  GLuint vboId;
  GLuint iboId;
  std::vector<VertexPTN> vertices;
  std::vector<unsigned int> vertexIndices;

  std::unordered_map<VertexPTN, unsigned int> uniqueVertices;

  int numVertices;
  int numTriangles;
  glm::vec3 objCenter;
};

#endif
