#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "headers.h"
#include "material.h"

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

  bool operator==(const VertexPTN &other) const {
    return position == other.position && normal == other.normal &&
           texcoord == other.texcoord;
  }
};

namespace std {
template <>
struct hash<VertexPTN> {
  size_t operator()(const VertexPTN &vertex) const {
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

// SubMesh Declarations.
struct SubMesh {
  SubMesh() {
    material = nullptr;
    iboId = 0;
  }

  ~SubMesh() { vertexIndices.clear(); }

  void draw() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN),
                          (void *)offsetof(VertexPTN, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN),
                          (void *)offsetof(VertexPTN, normal));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 vertexIndices.size() * sizeof(unsigned int),
                 vertexIndices.data(), GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, (GLsizei)vertexIndices.size(), GL_UNSIGNED_INT,
                   0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void createBuffer() { glGenBuffers(1, &iboId); }

  PhongMaterial *material;
  GLuint iboId;
  std::vector<unsigned int> vertexIndices;
};

// TriangleMesh Declarations.
class TriangleMesh {
 public:
  // TriangleMesh Public Methods.
  TriangleMesh();
  ~TriangleMesh();

  // Load the model from an *.OBJ file.
  bool LoadFromFile(const std::string &filePath, const bool normalized = true);

  // Show model information.
  void ShowInfo();

  // -------------------------------------------------------
  // Feel free to add your methods or data here.
  // -------------------------------------------------------

  std::vector<SubMesh> &getSubMeshes() { return subMeshes; }

  void createBuffer();
  void bindBuffer();

  int GetNumVertices() const { return numVertices; }
  int GetNumTriangles() const { return numTriangles; }
  int GetNumSubMeshes() const { return (int)subMeshes.size(); }

  glm::vec3 GetObjCenter() const { return objCenter; }
  glm::vec3 GetObjExtent() const { return objExtent; }

 private:
  // -------------------------------------------------------
  // Feel free to add your methods or data here.
  // -------------------------------------------------------

  void findAndAddVertexIndices(const VertexPTN vertex, SubMesh &subMesh);
  void polygonSubdivision(const std::vector<glm::vec3> &points,
                          const std::vector<glm::vec2> &texs,
                          const std::vector<glm::vec3> &normals,
                          const std::vector<std::string> parts,
                          SubMesh &subMesh);
  void processLine(std::vector<glm::vec3> &points, std::vector<glm::vec2> &texs,
                   std::vector<glm::vec3> &normals,
                   std::vector<std::string> parts);
  void processMaterialLib(const std::string &mtlFile);

  // 處理材質屬性
  void processMateriakProperty(const std::string &cmd,
                               const std::vector<std::string> &parts,
                               PhongMaterial *material);

  // 使用材質
  void processUseMaterial(const std::string &matName);

  // 處理頂點位置
  void processVertex(const std::vector<std::string> &parts,
                     std::vector<glm::vec3> &points);

  // 處理紋理座標
  void processTexCoord(const std::vector<std::string> &parts,
                       std::vector<glm::vec2> &texs);

  // 處理法線
  void processNormal(const std::vector<std::string> &parts,
                     std::vector<glm::vec3> &normals);
  void processFace(const std::vector<std::string> &parts,
                   const std::vector<glm::vec3> &points,
                   const std::vector<glm::vec2> &texs,
                   const std::vector<glm::vec3> &normals);

  // TriangleMesh Private Data.
  GLuint vboId;

  std::vector<VertexPTN> vertices;
  // For supporting multiple materials per object, move to SubMesh.
  // GLuint iboId;
  // std::vector<unsigned int> vertexIndices;
  std::unordered_map<VertexPTN, unsigned int> uniqueVertices;
  std::unordered_map<std::string, PhongMaterial *> materials;
  std::vector<SubMesh> subMeshes;

  // material file path
  std::string mtlFilePath;
  std::string objFilePath;

  int numVertices;
  int numTriangles;
  glm::vec3 objCenter;
  glm::vec3 objExtent;
};

// helper functions prototypes
std::vector<std::string> getFilesInDirectory(const std::string &directoryPath);
std::vector<std::string> splitString(std::string s, char delimiter);

#endif
