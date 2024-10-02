#include "TriangleMesh.h"

// split the string
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

void findAndAddVertexIndices(
    std::unordered_map<VertexPTN, unsigned int>& uniqueVertices,
    std::vector<VertexPTN>& vertices, std::vector<unsigned int>& vertexIndices,
    VertexPTN vertex) {
  if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
    uniqueVertices[vertex] = vertices.size();
    vertices.push_back(vertex);
    vertexIndices.push_back(vertices.size() - 1);
  } else {
    vertexIndices.push_back(uniqueVertices[vertex]);
  }
}

void polygonSubdivision(
    std::vector<glm::vec3>& points, std::vector<glm::vec2>& texs,
    std::vector<glm::vec3>& normals, std::vector<std::string>& parts,
    std::unordered_map<VertexPTN, unsigned int>& uniqueVertices,
    std::vector<VertexPTN>& vertices,
    std::vector<unsigned int>& vertexIndices) {
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

    findAndAddVertexIndices(uniqueVertices, vertices, vertexIndices,
                            firstVertex);
    findAndAddVertexIndices(uniqueVertices, vertices, vertexIndices,
                            secondVertex);
    findAndAddVertexIndices(uniqueVertices, vertices, vertexIndices,
                            thirdVertex);
  }
}

// process the line
void TriangleMesh::processLine(std::vector<glm::vec3>& points,
                               std::vector<glm::vec2>& texs,
                               std ::vector<glm::vec3>& normals,
                               std::vector<std::string> parts) {
  for (int i = 0; i < parts.size(); i++) {
    std::string part = parts[i];
    if (part == "#") return;
    if (part == "mtllib") return;
    if (part == "v") {
      glm::vec3 point;
      point.x = std::stof(parts[i + 1]);
      point.y = std::stof(parts[i + 2]);
      point.z = std::stof(parts[i + 3]);
      points.push_back(point);
      return;
    } else if (part == "vt") {
      glm::vec2 tex;
      tex.x = std::stof(parts[i + 1]);
      tex.y = std::stof(parts[i + 2]);
      texs.push_back(tex);
      return;
    } else if (part == "vn") {
      glm::vec3 normal;
      normal.x = std::stof(parts[i + 1]);
      normal.y = std::stof(parts[i + 2]);
      normal.z = std::stof(parts[i + 3]);
      normals.push_back(normal);
      return;
    } else if (part == "f") {
      if (parts.size() - 1 > 3) {
        polygonSubdivision(points, texs, normals, parts, uniqueVertices,
                           vertices, vertexIndices);
        return;
      }
      for (int j = i + 1; j < parts.size(); j++) {
        std::vector<std::string> indices = splitString(parts[j], '/');

        VertexPTN vertex;
        vertex.position = points[std::stoi(indices[0]) - 1];
        vertex.texcoord = texs[std::stoi(indices[1]) - 1];
        vertex.normal = normals[std::stoi(indices[2]) - 1];

        findAndAddVertexIndices(uniqueVertices, vertices, vertexIndices,
                                vertex);
      }
      return;
    }
  }
}

// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh() {
  numVertices = 0;
  numTriangles = 0;
  objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
  vboId = 0;
  iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh() {
  vertices.clear();
  vertexIndices.clear();
  glDeleteBuffers(1, &vboId);
  glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string& filePath,
                                const bool normalized) {
  // Add your code here.
  // ...

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
  numTriangles = vertexIndices.size() / 3;

  PrintMeshInfo();
  return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers() {
  // Add your code here.
  // ...
  glGenBuffers(1, &vboId);
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPTN),
               vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &iboId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vertexIndices.size() * sizeof(unsigned int),
               vertexIndices.data(), GL_STATIC_DRAW);
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4& mvpMatrix) {
  for (int i = 0; i < numVertices; ++i) {
    glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
    if (p.w != 0.0f) {
      float inv = 1.0f / p.w;
      vertices[i].position.x = p.x * inv;
      vertices[i].position.y = p.y * inv;
      vertices[i].position.z = p.z * inv;
    }
  }
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const {
  std::cout << "[*] Mesh Information: " << std::endl;
  std::cout << "# Vertices: " << numVertices << std::endl;
  std::cout << "# Triangles: " << numTriangles << std::endl;
  std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , "
            << objCenter.z << ")" << std::endl;
}
