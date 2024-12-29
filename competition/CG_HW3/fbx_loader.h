#pragma once
#include "camera.h"
#include "headers.h"
#include "light.h"
#include "material.h"
#include "scene.h"
#include "trianglemesh.h"
#include "fbx_model_loader.h"

class FbxSdkLoader : public FbxModelLoader
{
public:
  FbxSdkLoader(TriangleMesh *);
  ~FbxSdkLoader();
  bool loadFbx(const std::string &filePath, Scene *scene) override;

private:
  TriangleMesh *mesh;

  FbxManager *fbxsdkManager;
  FbxIOSettings *ios;
  FbxImporter *importer;
  FbxScene *fbxScene;

  void polygonSubdivision(std::vector<VertexPTN> &vertices, TriangleMesh *mesh,
                          SubMesh &subMesh);

  void processNode(FbxNode *node, const FbxAMatrix &parentTransform,
                   Scene *scene);
  void processMesh(FbxMesh *fbxMesh, const FbxAMatrix &transform, Scene *scene);
  void processMaterial(FbxSurfaceMaterial *fbxMaterial,
                       PhongMaterial *phongMaterial);

  glm::mat4 convertFbxMatrixToGlm(const FbxAMatrix &fbxMat);

  void processTexture(FbxProperty &prop, PhongMaterial *phongMaterial, const std::string &textureType);
  void processLights(FbxNode *node, Scene *scene);
  void processCamera(FbxNode *node, Scene *scene);
};
