#include "fbx_loader.h"

// 構造函式
FbxSdkLoader::FbxSdkLoader(TriangleMesh *mesh)
    : mesh(mesh),
      fbxsdkManager(nullptr),
      ios(nullptr),
      importer(nullptr),
      fbxScene(nullptr)
{
  // 初始化 FBX SDK 管理器
  fbxsdkManager = FbxManager::Create();
  if (!fbxsdkManager)
  {
    std::cerr << "Error: Unable to create FBX Manager!" << std::endl;
    return;
  }

  // 創建 IO 設定
  ios = FbxIOSettings::Create(fbxsdkManager, IOSROOT);
  fbxsdkManager->SetIOSettings(ios);

  // 創建導入器
  importer = FbxImporter::Create(fbxsdkManager, "");

  // 創建場景
  fbxScene = FbxScene::Create(fbxsdkManager, "fbxScene");
}

// 解構函式
FbxSdkLoader::~FbxSdkLoader()
{
  if (fbxsdkManager)
  {
    if (fbxScene)
      fbxScene->Destroy();
    if (importer)
      importer->Destroy();
    if (ios)
      ios->Destroy();
    fbxsdkManager->Destroy();
  }
}

void FbxSdkLoader::polygonSubdivision(std::vector<VertexPTN> &vertices,
                                      TriangleMesh *mesh, SubMesh &subMesh)
{
  VertexPTN v0 = vertices[0];
  for (int i = 1; i < vertices.size() - 1; i++)
  {
    VertexPTN v1 = vertices[i];
    VertexPTN v2 = vertices[i + 1];

    subMesh.vertexIndices.push_back(mesh->vertices.size());
    mesh->vertices.push_back(v0);
    subMesh.vertexIndices.push_back(mesh->vertices.size());
    mesh->vertices.push_back(v1);
    subMesh.vertexIndices.push_back(mesh->vertices.size());
    mesh->vertices.push_back(v2);
  }
}

// 載入 FBX 檔案
bool FbxSdkLoader::loadFbx(const std::string &filePath, Scene *scene)
{
  // 初始化導入器
  if (!importer->Initialize(filePath.c_str(), -1,
                            fbxsdkManager->GetIOSettings()))
  {
    std::cerr << "Error: Unable to initialize FBX Importer for " << filePath
              << std::endl;
    std::cerr << "Error: " << importer->GetStatus().GetErrorString()
              << std::endl;
    return false;
  }

  // 將檔案導入場景
  if (!importer->Import(fbxScene))
  {
    std::cerr << "Error: Unable to import FBX file: " << filePath << std::endl;
    return false;
  }

  // 定义目标坐标系（Y-Up 转为 Z-Up 或反之）
  FbxAxisSystem targetAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);

  // 将场景转换到目标坐标系
  targetAxisSystem.ConvertScene(fbxScene);
  // 轉換單位
  FbxSystemUnit::m.ConvertScene(fbxScene);

  // 取得ambient light
  FbxColor ambientColor = fbxScene->GetGlobalSettings().GetAmbientColor();

  scene->ambientLight = glm::vec3(static_cast<float>(ambientColor.mRed),
                                  static_cast<float>(ambientColor.mGreen),
                                  static_cast<float>(ambientColor.mBlue));

  // 處理場景根節點
  FbxNode *rootNode = fbxScene->GetRootNode();
  if (rootNode)
  {
    FbxAMatrix identity;
    identity.SetIdentity();
    processNode(rootNode, identity, scene);
  }

  return true;
}

// 處理節點
void FbxSdkLoader::processNode(FbxNode *node, const FbxAMatrix &parentTransform,
                               Scene *scene)
{
  FbxAMatrix localTransform = node->EvaluateLocalTransform();

  FbxAMatrix globalTransform = parentTransform * localTransform;

  // 處理光源
  processLights(node, scene);

  // 處理相機
  processCamera(node, scene);

  // 處理網格
  if (node->GetNodeAttribute())
  {
    FbxNodeAttribute::EType attributeType =
        node->GetNodeAttribute()->GetAttributeType();
    if (attributeType == FbxNodeAttribute::eMesh)
    {
      FbxMesh *fbxMesh = node->GetMesh();

      // get geometry transform
      FbxVector4 geoTranslation = node->GetGeometricTranslation(FbxNode::eSourcePivot);
      FbxVector4 geoRotation = node->GetGeometricRotation(FbxNode::eSourcePivot);
      FbxVector4 geoScaling = node->GetGeometricScaling(FbxNode::eSourcePivot);

      FbxAMatrix geometryTransform;
      geometryTransform.SetT(geoTranslation);
      geometryTransform.SetR(geoRotation);
      geometryTransform.SetS(geoScaling);

      globalTransform = globalTransform * geometryTransform;

      processMesh(fbxMesh, globalTransform, scene);
    }
  }

  // 遞迴處理子節點
  for (int i = 0; i < node->GetChildCount(); i++)
  {
    processNode(node->GetChild(i), globalTransform, scene);
  }
}

// 處理網格
void FbxSdkLoader::processMesh(FbxMesh *fbxMesh, const FbxAMatrix &transform,
                               Scene *scene)
{
  int polygonCount = fbxMesh->GetPolygonCount();
  int vertexCount = fbxMesh->GetControlPointsCount();

  auto axisSystem = fbxMesh->GetScene()->GetGlobalSettings().GetAxisSystem();

  // 獲取控制點
  FbxVector4 *controlPoints = fbxMesh->GetControlPoints();

  // 頂點資料
  std::vector<VertexPTN> vertices;

  // submesh map
  std::map<int, SubMesh> subMeshMap;

  // 轉換法線
  FbxGeometryElementNormal *normalElement = fbxMesh->GetElementNormal();
  bool hasNormals =
      normalElement &&
      (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint ||
       normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

  // 轉換 UV
  FbxGeometryElementUV *uvElement = fbxMesh->GetElementUV();
  bool hasUVs =
      uvElement &&
      (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint ||
       uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

  for (int p = 0; p < polygonCount; p++)
  {

    int polySize = fbxMesh->GetPolygonSize(p);
    std::vector<VertexPTN> polyVertices;

    for (int v = 0; v < polySize; v++)
    {
      int controlIndex = fbxMesh->GetPolygonVertex(p, v);

      // 位置
      // 位置轉換
      FbxVector4 pos = controlPoints[controlIndex];

      FbxVector4 transformedPos = transform.MultT(pos);
      glm::vec3 glmPos(static_cast<float>(transformedPos[0]),
                       static_cast<float>(transformedPos[1]),
                       static_cast<float>(transformedPos[2]));

      // 法線轉換
      glm::vec3 normal(0.0f);
      if (hasNormals)
      {
        int normalIndex =
            (normalElement->GetReferenceMode() ==
             FbxGeometryElement::eIndexToDirect)
                ? normalElement->GetIndexArray().GetAt(p * polySize + v)
                : p * polySize + v;
        FbxVector4 norm = normalElement->GetDirectArray().GetAt(normalIndex);
        FbxAMatrix normalTransform = transform.Inverse().Transpose();
        FbxVector4 transformedNorm = normalTransform.MultT(norm);
        normal =
            glm::normalize(glm::vec3(static_cast<float>(transformedNorm[0]),
                                     static_cast<float>(transformedNorm[1]),
                                     static_cast<float>(transformedNorm[2])));
      }

      // UV
      glm::vec2 texcoord(0.0f, 0.0f);
      if (hasUVs)
      {
        int uvIndex = 0;
        if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
        {
          uvIndex = controlIndex;
        }
        else if (uvElement->GetReferenceMode() ==
                 FbxGeometryElement::eIndexToDirect)
        {
          uvIndex = uvElement->GetIndexArray().GetAt(p * polySize + v);
        }
        FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
        texcoord =
            glm::vec2(static_cast<float>(uv[0]), static_cast<float>(uv[1]));
      }

      VertexPTN vertex;
      vertex.position = glmPos;
      vertex.normal = normal;
      vertex.texcoord = texcoord;

      polyVertices.push_back(vertex);
    }

    // 獲取或創建 SubMesh
    SubMesh subMesh;
    // 獲取材質索引
    int materialIndex = fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(p);

    if (subMeshMap.find(materialIndex) == subMeshMap.end())
    {
      subMesh.material = new PhongMaterial();
      processMaterial(fbxMesh->GetNode()->GetMaterial(materialIndex),
                      subMesh.material);
      subMeshMap[materialIndex] = subMesh;
    }
    else
    {
      subMesh = subMeshMap[materialIndex];
    }

    // 三角形化
    polygonSubdivision(polyVertices, mesh, subMesh);

    subMeshMap[materialIndex] = subMesh;
  }

  // 將所有 SubMesh 推送到 TriangleMesh
  for (auto &pair : subMeshMap)
  {
    mesh->subMeshes.push_back(pair.second);
  }
}

// 處理材質
void FbxSdkLoader::processMaterial(FbxSurfaceMaterial *fbxMaterial,
                                   PhongMaterial *phongMaterial)
{
  std::cout << "Material: " << fbxMaterial->GetName() << std::endl;

  // 獲取漫反射顏色
  FbxProperty diffuseProp =
      fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
  if (diffuseProp.IsValid())
  {
    FbxDouble3 color = diffuseProp.Get<FbxDouble3>();
    phongMaterial->SetKd(glm::vec3(static_cast<float>(color[0]),
                                   static_cast<float>(color[1]),
                                   static_cast<float>(color[2])));
    processTexture(diffuseProp, phongMaterial, "diffuse");
  }

  // 獲取環境顏色
  FbxProperty ambientProp =
      fbxMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
  if (ambientProp.IsValid())
  {
    FbxDouble3 color = ambientProp.Get<FbxDouble3>();
    phongMaterial->SetKa(glm::vec3(static_cast<float>(color[0]),
                                   static_cast<float>(color[1]),
                                   static_cast<float>(color[2])));
  }

  // 獲取鏡面顏色
  FbxProperty specularProp =
      fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
  if (specularProp.IsValid())
  {
    FbxDouble3 color = specularProp.Get<FbxDouble3>();
    phongMaterial->SetKs(glm::vec3(static_cast<float>(color[0]),
                                   static_cast<float>(color[1]),
                                   static_cast<float>(color[2])));
    processTexture(specularProp, phongMaterial, "specular");
  }

  // 獲取鏡面高光係數
  FbxProperty shininessProp =
      fbxMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
  if (shininessProp.IsValid())
  {
    double shininess = shininessProp.Get<FbxDouble>();
    phongMaterial->SetNs(static_cast<float>(shininess));
  }
}

void FbxSdkLoader::processTexture(FbxProperty &prop, PhongMaterial *phongMaterial,
                                  const std::string &textureType)
{
  if (!prop.IsValid())
    return;

  int textureCount = prop.GetSrcObjectCount<FbxFileTexture>();
  if (textureCount == 0)
    return;

  for (int i = 0; i < textureCount; ++i)
  {
    FbxFileTexture *texture = prop.GetSrcObject<FbxFileTexture>(i);
    if (!texture)
      continue;

    std::string texturePath = texture->GetFileName();
    if (texturePath.empty())
      continue;

    // tell if the texture is .dds
    std::string extension = texturePath.substr(texturePath.find_last_of(".") + 1);

    if (extension == "dds")
    {
      // transform .dds to .png
      texturePath = texturePath.substr(0, texturePath.find_last_of(".")) + ".png";
    }

    // 創建 ImageTexture 實例並加載貼圖
    ImageTexture *imgTex = new ImageTexture(texturePath);

    // 根據貼圖類型設置到 PhongMaterial
    if (textureType == "diffuse")
      phongMaterial->SetMapKd(imgTex);
    else if (textureType == "specular")
      phongMaterial->SetMapKs(imgTex);
  }
}

// 處理光源
void FbxSdkLoader::processLights(FbxNode *node, Scene *scene)
{
  FbxLight *fbxLight = node->GetLight();
  if (!fbxLight)
    return;

  FbxLight::EType lightType = fbxLight->LightType.Get();
  glm::vec3 color(static_cast<float>(fbxLight->Color.Get()[0]),
                  static_cast<float>(fbxLight->Color.Get()[1]),
                  static_cast<float>(fbxLight->Color.Get()[2]));

  // 獲取節點的全局轉換矩陣
  FbxAMatrix globalTransform = node->EvaluateGlobalTransform();

  // 獲取光源方向
  FbxVector4 dir = globalTransform.GetR();

  glm::vec3 direction(static_cast<float>(dir[0]), static_cast<float>(dir[1]),
                      static_cast<float>(dir[2]));

  // // 獲取光照大小
  // FbxDouble intensity = fbxLight->Intensity.Get();
  // color *= static_cast<float>(intensity);

  // 獲取衰減參數
  float constant = 1.0f;
  float linear = 0.0f;
  float quadratic = 0.0f;

  switch (fbxLight->DecayType.Get())
  {
  case FbxLight::eNone:
    // 無衰減，保持預設值
    break;
  case FbxLight::eLinear:
    linear = 1.0f;
    break;
  case FbxLight::eQuadratic:
    quadratic = 1.0f;
    break;
  case FbxLight::eCubic:
    // 如果需要處理立方衰減，可自行設計
    break;
  default:
    break;
  }

  // 取得decay start
  double decayStart = fbxLight->DecayStart.Get();

  switch (lightType)
  {
  case FbxLight::eDirectional:
  {
    DirectionalLight *directionalLight =
        new DirectionalLight(glm::normalize(direction), color);
    scene->dirLights.push_back(directionalLight);
    std::cout << "Directional light added" << std::endl;
    break;
  }
  case FbxLight::ePoint:
  {
    FbxVector4 position = globalTransform.GetT();
    glm::vec3 pos(static_cast<float>(position[0]),
                  static_cast<float>(position[1]),
                  static_cast<float>(position[2]));
    PointLight *pointLight = new PointLight(pos, color);
    pointLight->SetLinear(linear);
    pointLight->SetQuadratic(quadratic);
    pointLight->SetConstant(constant);
    pointLight->SetDecayStart(static_cast<float>(decayStart));
    scene->pointLights.push_back(pointLight);
    std::cout << "Point light added" << std::endl;
    break;
  }
  case FbxLight::eSpot:
  {
    FbxVector4 position = globalTransform.GetT();
    glm::vec3 pos(static_cast<float>(position[0]),
                  static_cast<float>(position[1]),
                  static_cast<float>(position[2]));

    double innerAngle = fbxLight->InnerAngle.Get();
    double outerAngle = fbxLight->OuterAngle.Get();

    SpotLight *spotLight = new SpotLight(
        pos, color, glm::normalize(direction), static_cast<float>(innerAngle),
        static_cast<float>(outerAngle));
    spotLight->SetLinear(linear);
    spotLight->SetQuadratic(quadratic);
    spotLight->SetConstant(constant);
    spotLight->SetDecayStart(static_cast<float>(decayStart));
    scene->spotLights.push_back(spotLight);
    std::cout << "Spot light added" << std::endl;
    break;
  }
  case FbxLight::eArea:
  {
    FbxVector4 position = globalTransform.GetT();
    glm::vec3 pos(static_cast<float>(position[0]),
                  static_cast<float>(position[1]),
                  static_cast<float>(position[2]));

    FbxDouble3 scaling = node->LclScaling.Get();
    double width = scaling[0];
    double height = scaling[1];

    int samples = 2; // 可以根據需求調整

    AreaLight *areaLight = new AreaLight(
        pos, color, glm::normalize(direction), width, height, samples);
    areaLight->SetLinear(linear);
    areaLight->SetQuadratic(quadratic);
    areaLight->SetConstant(constant);
    areaLight->SetDecayStart(static_cast<float>(decayStart));
    scene->areaLights.push_back(areaLight);
    std::cout << "Area light added" << std::endl;
    break;
  }
  default:
    std::cout << "Unsupported light type: " << lightType << std::endl;
    break;
  }
}

// 處理相機
void FbxSdkLoader::processCamera(FbxNode *node, Scene *scene)
{
  FbxCamera *fbxCamera = node->GetCamera();
  if (!fbxCamera)
    return;

  // 獲取相機位置
  FbxVector4 cameraPosition = fbxCamera->EvaluatePosition();

  // 計算 LookAt 目標點
  FbxVector4 interestPosition = fbxCamera->EvaluateLookAtPosition();

  glm::vec3 position(static_cast<float>(cameraPosition[0]),
                     static_cast<float>(cameraPosition[1]),
                     static_cast<float>(cameraPosition[2]));

  glm::vec3 target(static_cast<float>(interestPosition[0]),
                   static_cast<float>(interestPosition[1]),
                   static_cast<float>(interestPosition[2]));

  // 獲取上方向
  FbxVector4 up = fbxCamera->EvaluateUpDirection(cameraPosition, interestPosition);
  glm::vec3 upVec(static_cast<float>(up[0]), static_cast<float>(up[1]),
                  static_cast<float>(up[2]));

  // 獲取水平視野 (Field of View)
  double horizontalFOV = fbxCamera->FieldOfView.Get(); // 單位為度

  // 獲取寬高比
  double filmWidth = fbxCamera->FilmWidth.Get();
  double filmHeight = fbxCamera->FilmHeight.Get();
  double aspectRatio = filmWidth / filmHeight;

  // 獲取近裁剪平面
  double clipPlaneNear = fbxCamera->NearPlane.Get();

  // 獲取遠裁剪平面
  double clipPlaneFar = fbxCamera->FarPlane.Get();

  Camera *camera = new Camera(
      position, target, upVec, static_cast<float>(horizontalFOV),
      static_cast<float>(aspectRatio), static_cast<float>(clipPlaneNear),
      static_cast<float>(clipPlaneFar));
  scene->camera = camera;
}