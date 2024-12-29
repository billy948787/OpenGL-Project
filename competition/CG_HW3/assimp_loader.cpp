#include "assimp_loader.h"

AssimpLoader::AssimpLoader(TriangleMesh *mesh) { this->mesh = mesh; }

AssimpLoader::~AssimpLoader() {}
inline float divideAndTruncate(float value)
{
    return static_cast<float>(static_cast<int>(value / 100.0f));
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 &from)
{
    glm::mat4 to;

    to[0][0] = divideAndTruncate(from.a1);
    to[0][1] = divideAndTruncate(from.b1);
    to[0][2] = divideAndTruncate(from.c1);
    to[0][3] = divideAndTruncate(from.d1);

    to[1][0] = divideAndTruncate(from.a2);
    to[1][1] = divideAndTruncate(from.b2);
    to[1][2] = divideAndTruncate(from.c2);
    to[1][3] = divideAndTruncate(from.d2);

    to[2][0] = divideAndTruncate(from.a3);
    to[2][1] = divideAndTruncate(from.b3);
    to[2][2] = divideAndTruncate(from.c3);
    to[2][3] = divideAndTruncate(from.d3);

    to[3][0] = divideAndTruncate(from.a4);
    to[3][1] = divideAndTruncate(from.b4);
    to[3][2] = divideAndTruncate(from.c4);
    to[3][3] = divideAndTruncate(from.d4);

    return to;
}

bool AssimpLoader::loadFbx(const std::string &filePath, Scene *scene) 
{
    Assimp::Importer importer;

    const aiScene *aiscene = importer.ReadFile(
        filePath, aiProcess_Triangulate | aiProcess_FlipUVs |
                      aiProcess_GenNormals | aiProcess_GenUVCoords |
                      aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);

    if (!aiscene)
    {
        std::cout << "Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    aiCamera *aiCamera = nullptr;

    Camera *camera;

    if (aiscene->HasCameras())
    {
        aiCamera = aiscene->mCameras[0];

        aiNode *cameraNode = aiscene->mRootNode->FindNode(aiCamera->mName);

        glm::mat4 transform = aiMatrix4x4ToGlm(cameraNode->mTransformation);

        glm::vec3 position =
            glm::vec3(aiCamera->mPosition.x, aiCamera->mPosition.y,
                      aiCamera->mPosition.z);
        glm::vec3 lookAt = glm::vec3(aiCamera->mLookAt.x, aiCamera->mLookAt.y,
                                     aiCamera->mLookAt.z);
        glm::vec3 up = glm::vec3(aiCamera->mUp.x, aiCamera->mUp.y, aiCamera->mUp.z);
        float horizontalFOV = glm::degrees(aiCamera->mHorizontalFOV);
        float aspectRatio = aiCamera->mAspect;
        float clipPlaneNear = aiCamera->mClipPlaneNear;
        float clipPlaneFar = aiCamera->mClipPlaneFar;

        position = glm::vec3(transform * glm::vec4(position, 1.0f));
        lookAt = glm::vec3(transform * glm::vec4(lookAt, 1.0f));
        up = glm::vec3(transform * glm::vec4(up, 1.0f));

        camera = new Camera(position, lookAt, up, horizontalFOV, aspectRatio,
                            clipPlaneNear, clipPlaneFar);

        scene->camera = camera;
    }

    if (aiscene->HasLights())
    {
        for (unsigned int i = 0; i < aiscene->mNumLights; i++)
        {
            aiLight *aiLight = aiscene->mLights[i];

            std::cout << "aiLight->mType: " << aiLight->mType << std::endl;

            switch (aiLight->mType)
            {
            case aiLightSource_AMBIENT:
            {
                // 提取顏色
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                scene->ambientLight = color;
                break;
            }

            case aiLightSource_DIRECTIONAL:
            {
                // 提取方向和顏色
                glm::vec3 direction(aiLight->mDirection.x, aiLight->mDirection.y,
                                    aiLight->mDirection.z);
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                // 創建 DirectionalLight 
                DirectionalLight *directionalLight =
                    new DirectionalLight(direction, color);

                // 添加到場景的 DirectionalLights 向量中
                scene->dirLights.push_back(directionalLight);
                break;
            }

            case aiLightSource_POINT:
            {
                // 提取位置和顏色
                glm::vec3 position(aiLight->mPosition.x, aiLight->mPosition.y,
                                   aiLight->mPosition.z);
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                // 創建 PointLight 對象
                PointLight *pointLight = new PointLight(position, color);

                // 添加到場景的 PointLights 向量中
                scene->pointLights.push_back(pointLight);
                break;
            }

            case aiLightSource_SPOT:
            {
                // 提取位置、方向和顏色
                glm::vec3 position(aiLight->mPosition.x, aiLight->mPosition.y,
                                   aiLight->mPosition.z);
                glm::vec3 direction(aiLight->mDirection.x, aiLight->mDirection.y,
                                    aiLight->mDirection.z);
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                // 提取剪裁角度並轉換為度數
                float cutoffStart = glm::degrees(aiLight->mAngleInnerCone);
                float totalWidth =
                    glm::degrees(aiLight->mAngleOuterCone - aiLight->mAngleInnerCone);

                // 創建 SpotLight 對象
                SpotLight *spotLight = new SpotLight(position, color, direction,
                                                     cutoffStart, totalWidth);

                // 添加到場景的 SpotLights 向量中
                scene->spotLights.push_back(spotLight);
                break;
            }

            case aiLightSource_AREA:
            {
                // 提取位置、方向和顏色
                glm::vec3 position(aiLight->mPosition.x, aiLight->mPosition.y,
                                   aiLight->mPosition.z);
                glm::vec3 direction(aiLight->mDirection.x, aiLight->mDirection.y,
                                    aiLight->mDirection.z);
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                // 提取尺寸
                float width = aiLight->mSize.x;
                float height = aiLight->mSize.y;

                // 創建 AreaLight 對象
                AreaLight *areaLight =
                    new AreaLight(position, color, direction, width, height, 10);

                // 添加到場景的 AreaLights 向量中
                scene->areaLights.push_back(areaLight);
                break;
            }

            default:

                std::cout << "default" << std::endl;
                std::cout << aiLight->mName.C_Str() << std::endl;
                // 提取位置、方向和顏色
                glm::vec3 position(aiLight->mPosition.x, aiLight->mPosition.y,
                                   aiLight->mPosition.z);
                glm::vec3 direction(aiLight->mDirection.x, aiLight->mDirection.y,
                                    aiLight->mDirection.z);
                glm::vec3 color(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g,
                                aiLight->mColorDiffuse.b);

                // 提取尺寸
                float width = aiLight->mSize.x;
                float height = aiLight->mSize.y;

                // 創建 AreaLight 對象
                AreaLight *areaLight =
                    new AreaLight(position, color, direction, width, height, 10);

                // 添加到場景的 AreaLights 向量中
                scene->areaLights.push_back(areaLight);
                break;
            }
        }
    }

    processNode(aiscene->mRootNode, aiscene, glm::mat4(1.0f));

    return true;
}

void AssimpLoader::processNode(aiNode *node, const aiScene *aiscene,
                               const glm::mat4 &parentTransform)

{
    aiMatrix4x4 aiMat = node->mTransformation;
    // 轉換 Assimp 的矩陣到 GLM
    glm::mat4 nodeTransform = aiMatrix4x4ToGlm(aiMat);

    // 累積父節點的轉換
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (node->mName.C_Str() == "Camera")
    {
        glm::mat4 cameraTransform = globalTransform;
    }

    // 處理當前節點的所有 Mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = aiscene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, aiscene, globalTransform);
    }

    // 遞迴處理子節點
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], aiscene, globalTransform);
    }
}

void AssimpLoader::processMesh(aiMesh *mesh, const aiScene *aiscene,
                               const glm::mat4 &transform)
{
    std::vector<VertexPTN> vertices;
    std::vector<PhongMaterial> materials;
    SubMesh subMesh;

    const aiMaterial *material = aiscene->mMaterials[mesh->mMaterialIndex];

    aiColor3D ambient;
    aiColor3D diffuse;
    aiColor3D specular;
    float n;

    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    material->Get(AI_MATKEY_SHININESS, n);

    PhongMaterial *phongMaterial = new PhongMaterial();

    phongMaterial->SetName(material->GetName().C_Str());
    phongMaterial->SetKa(glm::vec3(ambient.r, ambient.g, ambient.b));
    phongMaterial->SetKd(glm::vec3(diffuse.r, diffuse.g, diffuse.b));
    phongMaterial->SetKs(glm::vec3(specular.r, specular.g, specular.b));
    phongMaterial->SetNs(n);

    subMesh.material = phongMaterial;

    const int offset = this->mesh->vertices.size();

    // 處理頂點
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D aiVertex = mesh->mVertices[i];
        aiVector3D aiNormal = mesh->mNormals[i];
        aiVector3D aiTexCoord = mesh->HasTextureCoords(0)
                                    ? mesh->mTextureCoords[0][i]
                                    : aiVector3D(0.0f, 0.0f, 0.0f);

        // 應用轉換矩陣到頂點位置
        glm::vec4 position =
            transform * glm::vec4(aiVertex.x, aiVertex.y, aiVertex.z, 1.0f);
        // 對法線應用正常矩陣（轉換的逆轉置矩陣）
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
        glm::vec3 normal =
            normalMatrix * glm::vec3(aiNormal.x, aiNormal.y, aiNormal.z);

        if (position.w > 1)
        {
            std::cout << "position.w > 1" << std::endl;
        }

        VertexPTN vertex;
        vertex.position = glm::vec3(position) / position.w;
        vertex.normal = glm::normalize(normal);
        vertex.texcoord = glm::vec2(aiTexCoord.x, aiTexCoord.y);

        this->mesh->vertices.push_back(vertex);
    }
    // 處理索引
    for (unsigned int j = 0; j < mesh->mNumFaces; j++)
    {
        const aiFace &face = mesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; k++)
        {
            subMesh.vertexIndices.push_back(face.mIndices[k] + offset);
        }
    }

    this->mesh->subMeshes.push_back(subMesh);
}
