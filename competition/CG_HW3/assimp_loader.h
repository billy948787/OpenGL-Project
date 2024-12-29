#pragma once
#include "headers.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "scene.h"
#include "trianglemesh.h"
#include "fbx_model_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssimpLoader : public FbxModelLoader
{
public:
    AssimpLoader(TriangleMesh *mesh);
    ~AssimpLoader();
    bool loadFbx(const std::string &filePath, Scene *scene) override;

private:
    TriangleMesh *mesh;
    Assimp::Importer importer;
    const aiScene *sceneData;

    void processNode(aiNode *node, const aiScene *scene, const glm::mat4 &parentTransform);
    void processMesh(aiMesh *ai_mesh, const aiScene *scene, const glm::mat4 &transform);
    void processMaterial(aiMaterial *ai_material, PhongMaterial *phongMaterial);
    void processTexture(aiMaterial *ai_material, PhongMaterial *phongMaterial, const std::string &textureType);
    void processLights(aiLight *ai_light, Scene *sceneOut, const glm::mat4 &transform);
    void processCamera(aiCamera *ai_camera, Scene *sceneOut, const glm::mat4 &transform);
};