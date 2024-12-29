#include "trianglemesh.h"

#include "fbx_loader.h"

std::vector<std::string> Utils::getFilesInDirectory(
    const std::string &directoryPath, const std::string &fileNameExtension)
{
  std::vector<std::string> files;
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(directoryPath))
  {
    if (entry.is_regular_file())
    {
      std::string filePath = entry.path().string();

      std::string extension = entry.path().extension().string();

      if (extension == fileNameExtension)
      {
        // 因為在 Windows 環境下路徑分隔符號是 '\' ，而在 Unix 環境下是 '/'
        // 而如果將路徑中的 '\' 換成 '/' ，可以避免一些問題
        std::replace(filePath.begin(), filePath.end(), '\\', '/');
        files.push_back(filePath);
      }
    }
  }

  return files;
}

std::vector<std::string> Utils::splitString(std::string s, char delimiter)
{
  std::vector<std::string> parts;
  std::string temp;
  for (int i = 0; i < s.size(); i++)
  {
    if (s[i] == delimiter || s[i] == '\n' || s[i] == '\r')
    {
      if (!temp.empty())
        parts.push_back(temp);
      temp.clear();
    }
    else if (i == s.size() - 1)
    {
      temp.push_back(s[i]);
      parts.push_back(temp);
    }
    else
    {
      temp.push_back(s[i]);
    }
  }

  return parts;
}

std::string Utils::getExtension(const std::string &filePath)
{
  return std::filesystem::path(filePath).extension().string();
}

VertexPTN TriangleMesh::parseVertex(const std::string &vertexData,
                                    const std::vector<glm::vec3> &points,
                                    const std::vector<glm::vec2> &texs,
                                    const std::vector<glm::vec3> &normals)
{
  std::vector<std::string> indices = Utils::splitString(vertexData, '/');
  int vIndex = std::stoi(indices[0]) - 1;
  glm::vec3 position = points[vIndex];
  glm::vec2 texcoord(0.0f, 0.0f);
  glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
  // 判斷是否有紋理座標
  if (indices.size() == 3)
  { // 有紋理座標
    int tIndex = std::stoi(indices[1]) - 1;
    texcoord = texs[tIndex];
    int nIndex = std::stoi(indices[2]) - 1;
    normal = normals[nIndex];
  }
  else
  {
    int nIndex = std::stoi(indices[1]) - 1;
    normal = normals[nIndex];
  }

  return VertexPTN(position, normal, texcoord);
}

void TriangleMesh::findAndAddVertexIndices(const VertexPTN vertex,
                                           SubMesh &subMesh)
{
  if (uniqueVertices.find(vertex) == uniqueVertices.end())
  {
    uniqueVertices[vertex] = vertices.size();
    vertices.push_back(vertex);
    subMesh.vertexIndices.push_back(vertices.size() - 1);
  }
  else
  {
    subMesh.vertexIndices.push_back(uniqueVertices[vertex]);
  }
}

void TriangleMesh::polygonSubdivision(const std::vector<glm::vec3> &points,
                                      const std::vector<glm::vec2> &texs,
                                      const std::vector<glm::vec3> &normals,
                                      const std::vector<std::string> parts,
                                      SubMesh &subMesh)
{
  VertexPTN firstVertex = parseVertex(parts[1], points, texs, normals);
  for (int j = 2; j < parts.size() - 1; j++)
  {
    VertexPTN secondVertex = parseVertex(parts[j], points, texs, normals);
    VertexPTN thirdVertex = parseVertex(parts[j + 1], points, texs, normals);

    findAndAddVertexIndices(firstVertex, subMesh);
    findAndAddVertexIndices(secondVertex, subMesh);
    findAndAddVertexIndices(thirdVertex, subMesh);
    numTriangles++;
  }
}

void TriangleMesh::parseLine(std::vector<glm::vec3> &points,
                             std::vector<glm::vec2> &texs,
                             std::vector<glm::vec3> &normals,
                             std::vector<std::string> parts)
{
  if (parts.empty())
    return;

  const std::string &command = parts[0];
  if (command == "#")
    return;

  // 處理主要命令
  if (command == "mtllib")
  {
    processMaterialLib(parts[1]);
  }
  else if (command == "usemtl")
  {
    processUseMaterial(parts[1]);
  }
  else if (command == "v")
  {
    processVertex(parts, points);
  }
  else if (command == "vt")
  {
    processTexCoord(parts, texs);
  }
  else if (command == "vn")
  {
    processNormal(parts, normals);
  }
  else if (command == "f")
  {
    processFace(parts, points, texs, normals);
  }
}

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
  // -------------------------------------------------------
  // Add your initialization code here.
  // -------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
  subMeshes.clear();
  // Destructor of a triangle mesh.

  for (auto &subMesh : subMeshes)
  {
    delete subMesh.material;
    glDeleteBuffers(1, &subMesh.iboId);
    subMesh.vertexIndices.clear();
  }
  subMeshes.clear();

  for (auto &pair : materials)
  {
    delete pair.second;
  }

  materials.clear();
  vertices.clear();
  uniqueVertices.clear();

  glDeleteBuffers(1, &vboId);
}
void TriangleMesh::processMaterialLib(const std::string &mtlFile)
{
  // 因為 mtl 檔案與 obj 檔案放在一起，所以根據 objFilePath 找到 mtl 檔案
  mtlFilePath =
      objFilePath.substr(0, objFilePath.find_last_of('/')) + "/" + mtlFile;
  std::ifstream file(mtlFilePath);
  if (!file.is_open())
  {
    std::cerr << "Error: Failed to open file " << mtlFilePath << std::endl;
    return;
  }

  PhongMaterial *currentMaterial = nullptr;
  std::string line;
  while (std::getline(file, line))
  {
    if (line.empty())
      continue;

    std::vector<std::string> parts = Utils::splitString(line, ' ');
    if (parts.empty())
      continue;

    const std::string &cmd = parts[0];
    if (cmd == "newmtl")
    {
      currentMaterial = new PhongMaterial();
      currentMaterial->SetName(parts[1]);
      materials[parts[1]] = currentMaterial;
    }
    else if (currentMaterial)
    {
      processMateriakProperty(cmd, parts, currentMaterial);
    }
  }
  file.close();
}

void TriangleMesh::processMateriakProperty(
    const std::string &cmd, const std::vector<std::string> &parts,
    PhongMaterial *material)
{
  if (cmd == "Ka" || cmd == "Kd" || cmd == "Ks")
  {
    glm::vec3 color(std::stof(parts[1]), std::stof(parts[2]),
                    std::stof(parts[3]));
    if (cmd == "Ka")
      material->SetKa(color);
    else if (cmd == "Kd")
      material->SetKd(color);
    else
      material->SetKs(color);
  }
  else if (cmd == "Ns")
  {
    material->SetNs(std::stof(parts[1]));
  }
  else if (cmd == "map_Kd")
  {
    ImageTexture *texture = new ImageTexture(
        mtlFilePath.substr(0, mtlFilePath.find_last_of('/')) + "/" + parts[1]);
    material->SetMapKd(texture);
  }
}

void TriangleMesh::processUseMaterial(const std::string &matName)
{
  SubMesh subMesh;
  subMesh.material = materials[matName];
  subMeshes.push_back(subMesh);
}

void TriangleMesh::processVertex(const std::vector<std::string> &parts,
                                 std::vector<glm::vec3> &points)
{
  points.push_back(
      glm::vec3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3])));
}

void TriangleMesh::processTexCoord(const std::vector<std::string> &parts,
                                   std::vector<glm::vec2> &texs)
{
  texs.push_back(glm::vec2(std::stof(parts[1]), std::stof(parts[2])));
}

void TriangleMesh::processNormal(const std::vector<std::string> &parts,
                                 std::vector<glm::vec3> &normals)
{
  normals.push_back(
      glm::vec3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3])));
}

void TriangleMesh::processFace(const std::vector<std::string> &parts,
                               const std::vector<glm::vec3> &points,
                               const std::vector<glm::vec2> &texs,
                               const std::vector<glm::vec3> &normals)
{
  if (parts.size() == 4)
  {
    // 三角形面，直接處理
    for (int i = 1; i < parts.size(); i++)
    {
      VertexPTN vertex = parseVertex(parts[i], points, texs, normals);
      if (subMeshes.empty())
      {
        SubMesh subMesh;
        subMeshes.push_back(subMesh);
      }
      findAndAddVertexIndices(vertex, subMeshes.back());
    }
    numTriangles++;
  }
  else if (parts.size() > 4)
  {
    // 多邊形面，進行多邊形分割
    if (subMeshes.empty())
    {
      SubMesh subMesh;
      subMeshes.push_back(subMesh);
    }
    polygonSubdivision(points, texs, normals, parts, subMeshes.back());
  }
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string &filePath,
                                const bool normalized, Scene *scene)
{
  objFilePath = filePath;
  // temp vertex data
  std::vector<glm::vec3> points;
  std::vector<glm::vec2> texs;
  std::vector<glm::vec3> normals;

  std::string extension = Utils::getExtension(filePath);

  if (extension == ".fbx")
  {
    FbxModelLoader* loader = new FbxSdkLoader(this);

    loader->loadFbx(filePath, scene);

    delete loader;
  }
  else if (extension == ".obj")
  {
    std::ifstream file(filePath);
    if (!file.is_open())
    {
      std::cerr << "Error: Failed to open file " << filePath << std::endl;
      return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
      if (line.empty())
        continue;

      std::vector<std::string> parts = Utils::splitString(line, ' ');
      parseLine(points, texs, normals, parts);
    }
    file.close();
  }

  // Clear temp data.
  points.clear();
  texs.clear();
  normals.clear();
  uniqueVertices.clear();

  // Normalize the vertices.
  if (normalized)
  {
    // Step 1: Calculate the bounding box.
    glm::vec3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 maxPoint(FLT_MIN, FLT_MIN, FLT_MIN);

    for (const auto &vertex : vertices)
    {
      minPoint = glm::min(minPoint, vertex.position);
      maxPoint = glm::max(maxPoint, vertex.position);
    }

    // Step 2: Calculate the maximum side length of the bounding box.
    glm::vec3 bboxSize = maxPoint - minPoint;
    float maxSideLength =
        glm::max(glm::max(bboxSize.x, bboxSize.y), bboxSize.z);

    // Step 3: Normalize the vertices.
    for (auto &vertex : vertices)
    {
      vertex.position = (vertex.position - minPoint) / maxSideLength;
    }

    // reset the minPoint and maxPoint
    minPoint = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPoint = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);

    for (const auto &vertex : vertices)
    {
      minPoint = glm::min(minPoint, vertex.position);
      maxPoint = glm::max(maxPoint, vertex.position);
    }

    // Calculate the center and extent of the object.
    objCenter = (minPoint + maxPoint) * 0.5f;
    objExtent = maxPoint - minPoint;

    // Step 4: Move the object to the origin.
    glm::vec3 center = (minPoint + maxPoint) * 0.5f;
    for (auto &vertex : vertices)
    {
      vertex.position -= center;
    }
  }
  else
  {
    // Calculate the center and extent of the object.
    glm::vec3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 maxPoint(FLT_MIN, FLT_MIN, FLT_MIN);

    for (const auto &vertex : vertices)
    {
      minPoint = glm::min(minPoint, vertex.position);
      maxPoint = glm::max(maxPoint, vertex.position);
    }

    objCenter = (minPoint + maxPoint) * 0.5f;
    objExtent = maxPoint - minPoint;
  }

  // Calculate the number of vertices and triangles.
  numVertices = vertices.size();

  return true;
}

void TriangleMesh::createBuffer()
{
  glGenBuffers(1, &vboId);

  for (auto &subMesh : subMeshes)
  {
    subMesh.createBuffer();
  }

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPTN),
               vertices.data(), GL_STATIC_DRAW);
}

void TriangleMesh::bindBuffer() { glBindBuffer(GL_ARRAY_BUFFER, vboId); }

void TriangleMesh::draw(PhongShadingDemoShaderProg *shader)
{
  bindBuffer();
  // 遍歷所有子網格並繪製
  for (auto &subMesh : subMeshes)
  {
    if (subMesh.material)
    {
      // 設定材質屬性
      glUniform3fv(shader->GetLocKa(), 1,
                   glm::value_ptr(subMesh.material->GetKa()));
      glUniform3fv(shader->GetLocKd(), 1,
                   glm::value_ptr(subMesh.material->GetKd()));
      glUniform3fv(shader->GetLocKs(), 1,
                   glm::value_ptr(subMesh.material->GetKs()));
      glUniform1f(shader->GetLocNs(), subMesh.material->GetNs());

      // 綁定貼圖（如果有的話）
      if (subMesh.material->GetMapKd())
      {
        glActiveTexture(GL_TEXTURE0);
        subMesh.material->GetMapKd()->Bind(GL_TEXTURE0);
        glUniform1i(shader->GetLocMapKd(), 0);
        glUniform3fv(shader->GetLocKd(), 1, glm::value_ptr(glm::vec3(0.0f)));
      }
      else
      {
        // 如果沒有貼圖，設定為0
        glUniform1i(shader->GetLocMapKd(), 0);

        // 如果沒有貼圖，使用 Kd 作為顏色
        glUniform3fv(shader->GetLocKd(), 1,
                     glm::value_ptr(subMesh.material->GetKd()));
      }

      if (subMesh.material->GetMapKs())
      {
        glActiveTexture(GL_TEXTURE1);
        subMesh.material->GetMapKs()->Bind(GL_TEXTURE1);
        glUniform1i(shader->GetLocMapKs(), 1);
        glUniform3fv(shader->GetLocKs(), 1, glm::value_ptr(glm::vec3(1.0f)));
      }
      else
      {
        glUniform1i(shader->GetLocMapKs(), 0);
        glUniform3fv(shader->GetLocKs(), 1,
                     glm::value_ptr(subMesh.material->GetKs()));
      }
    }

    // 繪製子網格
    subMesh.draw();
  }
}

// Show model information.
void TriangleMesh::ShowInfo()
{
  std::cout << "# Vertices: " << numVertices << std::endl;
  std::cout << "# Triangles: " << numTriangles << std::endl;
  std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
  for (unsigned int i = 0; i < subMeshes.size(); ++i)
  {
    const SubMesh &g = subMeshes[i];
    if (g.material != nullptr)
    {
      std::cout << "Material: " << g.material->GetName() << std::endl;
    }
    std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3
              << std::endl;
  }
  std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", "
            << objCenter.z << std::endl;
  std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x "
            << objExtent.z << std::endl;
}
