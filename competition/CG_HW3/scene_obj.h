#pragma once
#include "headers.h"

class TriangleMesh;
// SceneObject.
struct SceneObject {
  SceneObject() {
    mesh = nullptr;
    worldMatrix = glm::mat4x4(1.0f);
  }
  TriangleMesh* mesh;
  glm::mat4x4 worldMatrix;
};