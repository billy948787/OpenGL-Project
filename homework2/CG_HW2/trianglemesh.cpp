#include "trianglemesh.h"

std::vector<std::string> getFilesInDirectory(const std::string& directoryPath) {
  std::vector<std::string> files;
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(directoryPath)) {
    if (entry.is_regular_file()) {
      std::string filePath = entry.path().string();

      if (filePath.find(".obj") != std::string::npos) {
		// 因為在 Windows 環境下路徑分隔符號是 '\' ，而在 Unix 環境下是 '/' 而如果將路徑中的 '\' 換成 '/' ，可以避免一些問題
        std::replace(filePath.begin(), filePath.end(), '\\' , '/');
        files.push_back(filePath);
      }
    }
  }

  return files;
}

std::vector<std::string> splitString(std::string s, char delimiter) {
  std::vector<std::string> parts;
  std::string temp;
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == delimiter || s[i] == '\n' || s[i] == '\r') {
      if (!temp.empty()) parts.push_back(temp);
      temp.clear();
    } else if (i == s.size() - 1) {
      temp.push_back(s[i]);
      parts.push_back(temp);
    } else {
      temp.push_back(s[i]);
    }
  }

  return parts;
}

void TriangleMesh::findAndAddVertexIndices(const VertexPTN vertex,
                                           SubMesh& subMesh) {
  if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
    uniqueVertices[vertex] = vertices.size();
    vertices.push_back(vertex);
    subMesh.vertexIndices.push_back(vertices.size() - 1);
  } else {
    subMesh.vertexIndices.push_back(uniqueVertices[vertex]);
  }
}

void TriangleMesh::polygonSubdivision(const std::vector<glm::vec3>& points,
                                      const std::vector<glm::vec2>& texs,
                                      const std::vector<glm::vec3>& normals,
                                      const std::vector<std::string> parts,
                                      SubMesh& subMesh) {
  VertexPTN firstVertex;
  for (int j = 1; j < parts.size() - 1; j++) {
    std::vector<std::string> indices = splitString(parts[j], '/');

    if (j == 1) {
      firstVertex.position = points[std::stoi(indices[0]) - 1];
      firstVertex.texcoord = texs[std::stoi(indices[1]) - 1];
      firstVertex.normal = normals[std::stoi(indices[2]) - 1];

      continue;
    }

    VertexPTN secondVertex;
    VertexPTN thirdVertex;

    secondVertex.position = points[std::stoi(indices[0]) - 1];
    secondVertex.texcoord = texs[std::stoi(indices[1]) - 1];
    secondVertex.normal = normals[std::stoi(indices[2]) - 1];

    indices = splitString(parts[j + 1], '/');

    thirdVertex.position = points[std::stoi(indices[0]) - 1];
    thirdVertex.texcoord = texs[std::stoi(indices[1]) - 1];
    thirdVertex.normal = normals[std::stoi(indices[2]) - 1];

    findAndAddVertexIndices(firstVertex, subMesh);
    findAndAddVertexIndices(secondVertex, subMesh);
    findAndAddVertexIndices(thirdVertex, subMesh);
    numTriangles++;
  }
}

void TriangleMesh::processLine(std::vector<glm::vec3>& points,
                               std::vector<glm::vec2>& texs,
                               std ::vector<glm::vec3>& normals,
                               std::vector<std::string> parts) {
  if (parts.empty()) return;

  const std::string& command = parts[0];
  if (command == "#") return;

  // 處理主要命令
  if (command == "mtllib") {
    processMaterialLib(parts[1]);
  } else if (command == "usemtl") {
    processUseMaterial(parts[1]);
  } else if (command == "v") {
    processVertex(parts, points);
  } else if (command == "vt") {
    processTexCoord(parts, texs);
  } else if (command == "vn") {
    processNormal(parts, normals);
  } else if (command == "f") {
    processFace(parts, points, texs, normals);
  }
}

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh() {
  // -------------------------------------------------------
  // Add your initialization code here.
  // -------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh() {
  subMeshes.clear();
  // Destructor of a triangle mesh.

  for (auto& subMesh : subMeshes) {
    delete subMesh.material;
    glDeleteBuffers(1, &subMesh.iboId);
    subMesh.vertexIndices.clear();
  }
  subMeshes.clear();

  for (auto& pair : materials) {
    delete pair.second;
  }

  materials.clear();
  vertices.clear();
  uniqueVertices.clear();

  glDeleteBuffers(1, &vboId);
}

void TriangleMesh::processMaterialLib(const std::string& mtlFile) {
  // 因為 mtl 檔案與 obj 檔案放在一起，所以根據 objFilePath 找到 mtl 檔案
  mtlFilePath =
      objFilePath.substr(0, objFilePath.find_last_of('/')) + "/" + mtlFile;
  std::ifstream file(mtlFilePath);
  if (!file.is_open()) {
    std::cerr << "Error: Failed to open file " << mtlFilePath << std::endl;
    return;
  }

  PhongMaterial* currentMaterial = nullptr;
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::vector<std::string> parts = splitString(line, ' ');
    if (parts.empty()) continue;

    const std::string& cmd = parts[0];
    if (cmd == "newmtl") {
      currentMaterial = new PhongMaterial();
      currentMaterial->SetName(parts[1]);
      materials[parts[1]] = currentMaterial;
    } else if (currentMaterial) {
      processMateriakProperty(cmd, parts, currentMaterial);
    }
  }
  file.close();
}

void TriangleMesh::processMateriakProperty(
    const std::string& cmd, const std::vector<std::string>& parts,
    PhongMaterial* material) {
  if (cmd == "Ka" || cmd == "Kd" || cmd == "Ks") {
    glm::vec3 color(std::stof(parts[1]), std::stof(parts[2]),
                    std::stof(parts[3]));
    if (cmd == "Ka")
      material->SetKa(color);
    else if (cmd == "Kd")
      material->SetKd(color);
    else
      material->SetKs(color);
  } else if (cmd == "Ns") {
    material->SetNs(std::stof(parts[1]));
  }
}

void TriangleMesh::processUseMaterial(const std::string& matName) {
  SubMesh subMesh;
  subMesh.material = materials[matName];
  subMeshes.push_back(subMesh);
}

void TriangleMesh::processVertex(const std::vector<std::string>& parts,
                                 std::vector<glm::vec3>& points) {
  points.push_back(
      glm::vec3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3])));
}

void TriangleMesh::processTexCoord(const std::vector<std::string>& parts,
                                   std::vector<glm::vec2>& texs) {
  texs.push_back(glm::vec2(std::stof(parts[1]), std::stof(parts[2])));
}

void TriangleMesh::processNormal(const std::vector<std::string>& parts,
                                 std::vector<glm::vec3>& normals) {
  normals.push_back(
      glm::vec3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3])));
}

void TriangleMesh::processFace(const std::vector<std::string>& parts,
                               const std::vector<glm::vec3>& points,
                               const std::vector<glm::vec2>& texs,
                               const std::vector<glm::vec3>& normals) {
  if (parts.size() - 1 > 3) {
    polygonSubdivision(points, texs, normals, parts, subMeshes.back());
    return;
  }

  for (int i = 1; i < parts.size(); i++) {
    std::vector<std::string> indices = splitString(parts[i], '/');
    VertexPTN vertex(points[std::stoi(indices[0]) - 1],
                     normals[std::stoi(indices[2]) - 1],
                     texs[std::stoi(indices[1]) - 1]);
    findAndAddVertexIndices(vertex, subMeshes.back());
  }
  numTriangles++;
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath,
                                const bool normalized) {
  objFilePath = filePath;
  // temp vertex data
  std::vector<glm::vec3> points;
  std::vector<glm::vec2> texs;
  std::vector<glm::vec3> normals;

  std::fstream file(filePath, std::ios::in);

  if (!file.is_open()) {
    std::cerr << "Error: Failed to open file " << filePath << std::endl;
    return false;
  }

  std::string line;

  while (getline(file, line)) {
    // Split the line into parts.
    std::vector<std::string> parts = splitString(line, ' ');
    // Process the data.
    processLine(points, texs, normals, parts);
  }

  file.close();

  // Clear temp data.
  points.clear();
  texs.clear();
  normals.clear();
  uniqueVertices.clear();

  // Normalize the vertices.
  if (normalized) {
    // Step 1: Calculate the bounding box.
    glm::vec3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 maxPoint(FLT_MIN, FLT_MIN, FLT_MIN);

    for (const auto& vertex : vertices) {
      minPoint = glm::min(minPoint, vertex.position);
      maxPoint = glm::max(maxPoint, vertex.position);
    }

    // Step 2: Calculate the maximum side length of the bounding box.
    glm::vec3 bboxSize = maxPoint - minPoint;
    float maxSideLength =
        glm::max(glm::max(bboxSize.x, bboxSize.y), bboxSize.z);

    // Step 3: Normalize the vertices.
    for (auto& vertex : vertices) {
      vertex.position = (vertex.position - minPoint) / maxSideLength;
    }

    // reset the minPoint and maxPoint
    minPoint = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPoint = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);

    for (const auto& vertex : vertices) {
      minPoint = glm::min(minPoint, vertex.position);
      maxPoint = glm::max(maxPoint, vertex.position);
    }

    // Step 4: Move the object to the origin.
    glm::vec3 center = (minPoint + maxPoint) * 0.5f;
    for (auto& vertex : vertices) {
      vertex.position -= center;
    }
  }

  // Calculate the number of vertices and triangles.
  numVertices = vertices.size();

  ShowInfo();
  return true;
}

void TriangleMesh::createBuffer() {
  glGenBuffers(1, &vboId);

  for (auto& subMesh : subMeshes) {
    subMesh.createBuffer();
  }

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPTN),
      vertices.data(), GL_STATIC_DRAW);
}

void TriangleMesh::bindBuffer() {
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
}

// Show model information.
void TriangleMesh::ShowInfo() {
  std::cout << "# Vertices: " << numVertices << std::endl;
  std::cout << "# Triangles: " << numTriangles << std::endl;
  std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
  for (unsigned int i = 0; i < subMeshes.size(); ++i) {
    const SubMesh& g = subMeshes[i];
    std::cout << "SubMesh " << i << " with material: " << g.material->GetName()
              << std::endl;
    std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3
              << std::endl;
  }
  std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", "
            << objCenter.z << std::endl;
  std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x "
            << objExtent.z << std::endl;
}
