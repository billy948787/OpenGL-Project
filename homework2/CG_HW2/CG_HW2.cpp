#include "camera.h"
#include "gui.h"
#include "headers.h"
#include "light.h"
#include "shaderprog.h"
#include "trianglemesh.h"

// Global variables
// model file directory path
const std::string modelDirectory = "../TestModels_HW2/";
const std::string defaultModelPath =
    "../TestModels_HW2/ColorCube/ColorCube.obj";

int screenWidth = 700;
int screenHeight = 700;
// GUI.
GUI* gui = nullptr;
// Triangle mesh.
TriangleMesh* mesh = nullptr;
// Lights.
DirectionalLight* dirLight = nullptr;
PointLight* pointLight = nullptr;
SpotLight* spotLight = nullptr;
// Parameters: PLEASE DO NOT CHANGE THESE PARAMETERS.
glm::vec3 dirLightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.6f, 0.6f, 0.6f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.0f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(0.5f, 0.1f, 0.1f);
glm::vec3 spotLightPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightIntensity = glm::vec3(0.5f, 0.5f, 0.1f);
float spotLightCutoffStartInDegree = 30.0f;
float spotLightTotalWidthInDegree = 45.0f;
glm::vec3 ambientLight = glm::vec3(0.2f, 0.2f, 0.2f);
// Camera.
Camera* camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;
// Shaders.
FillColorShaderProg* fillColorShader = nullptr;
PhongShadingDemoShaderProg* phongShadingShader = nullptr;
bool isBlingPhong = true;
// Light control.
bool showDirLightArrow = true;
bool onPointLight = true;
bool onSpotLight = true;
bool onDirLight = true;
bool onAmbientLight = true;
bool onDiffuseLight = true;
bool onSpecularLight = true;
// UI.
const float lightMoveSpeed = 0.2f;

// SceneObject.
struct SceneObject {
  SceneObject() {
    mesh = nullptr;
    worldMatrix = glm::mat4x4(1.0f);
  }
  TriangleMesh* mesh;
  glm::mat4x4 worldMatrix;
};
SceneObject sceneObj;

// ScenePointLight (for visualization of a point light).
struct ScenePointLight {
  ScenePointLight() {
    light = nullptr;
    worldMatrix = glm::mat4x4(1.0f);
    visColor = glm::vec3(1.0f, 1.0f, 1.0f);
  }
  PointLight* light;
  glm::mat4x4 worldMatrix;
  glm::vec3 visColor;
};
ScenePointLight pointLightObj;
ScenePointLight spotLightObj;

struct SceneDirLight {
  SceneDirLight() {
    light = nullptr;
    worldMatrix = glm::mat4x4(1.0f);
    visColor = glm::vec3(0.0f, 0.0f, 0.0f);
    position = glm::vec3(0.0f, 0.0f, -1.0f);
  }
  DirectionalLight* light;
  glm::mat4x4 worldMatrix;
  glm::vec3 visColor;
  glm::vec3 position;
};
SceneDirLight dirLightObj;

// Function prototypes.
void ReleaseResources();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(GLFWwindow, int, int);
void ProcessKeysCB(GLFWwindow, int, int, int, int);
void SetupRenderState();
void CreateShaderLib();
void CreateCamera();

void ReleaseResources() {
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------

  // Delete scene objects and lights.
  if (mesh != nullptr) {
    delete mesh;
    mesh = nullptr;
  }
  if (pointLight != nullptr) {
    delete pointLight;
    pointLight = nullptr;
  }
  if (spotLight != nullptr) {
    delete spotLight;
    spotLight = nullptr;
  }
  if (dirLight != nullptr) {
    delete dirLight;
    dirLight = nullptr;
  }
  // Delete camera.
  if (camera != nullptr) {
    delete camera;
    camera = nullptr;
  }
  // Delete shaders.
  if (fillColorShader != nullptr) {
    delete fillColorShader;
    fillColorShader = nullptr;
  }
  if (phongShadingShader != nullptr) {
    delete phongShadingShader;
    phongShadingShader = nullptr;
  }
  if (gui != nullptr) {
    delete gui;
    gui = nullptr;
  }
}

static float curRotationY = 0.0f;
const float rotStep = 0.025f;
void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render a triangle mesh with Phong shading.
  // ------------------------------------------------
  TriangleMesh* mesh = sceneObj.mesh;
  if (sceneObj.mesh != nullptr) {
    // Update transform.
    // curRotationY += rotStep;
    glm::mat4x4 S = glm::scale(glm::mat4x4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    glm::mat4x4 R = glm::rotate(glm::mat4x4(1.0f), glm::radians(curRotationY),
                                glm::vec3(0, 1, 0));
    sceneObj.worldMatrix = S * R;
    // -------------------------------------------------------
    // Note: if you want to compute lighting in the View Space,
    //       you might need to change the code below.
    // -------------------------------------------------------
    glm::mat4x4 normalMatrix = glm::transpose(
        glm::inverse(camera->GetViewMatrix() * sceneObj.worldMatrix));
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      sceneObj.worldMatrix;

    // -------------------------------------------------------
    // Add your rendering code here.
    // -------------------------------------------------------
    phongShadingShader->Bind();
    mesh->bindBuffer();
    std::vector<SubMesh>& subMeshes = sceneObj.mesh->getSubMeshes();

    // matrix upload
    glUniformMatrix4fv(phongShadingShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniformMatrix4fv(phongShadingShader->GetLocM(), 1, GL_FALSE,
                       glm::value_ptr(sceneObj.worldMatrix));
    glUniformMatrix4fv(phongShadingShader->GetLocV(), 1, GL_FALSE,
                       glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(phongShadingShader->GetLocNM(), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    // camera position
    glUniform3fv(phongShadingShader->GetLocCameraPos(), 1,
                 glm::value_ptr(camera->GetCameraPos()));

    // ambient light
    glUniform3fv(phongShadingShader->GetLocAmbientLight(), 1,
                 glm::value_ptr(ambientLight));
    // directional light
    glUniform3fv(phongShadingShader->GetLocDirLightDir(), 1,
                 glm::value_ptr(dirLight->GetDirection()));
    glUniform3fv(phongShadingShader->GetLocDirLightRadiance(), 1,
                 glm::value_ptr(dirLight->GetRadiance()));

    // bool variables for light and shader.
    glUniform1i(phongShadingShader->GetLocIsBlingPhong(), isBlingPhong);
    glUniform1i(phongShadingShader->GetLocOnPointLight(), onPointLight);
    glUniform1i(phongShadingShader->GetLocOnSpotLight(), onSpotLight);
    glUniform1i(phongShadingShader->GetLocOnDirLight(), onDirLight);
    glUniform1i(phongShadingShader->GetLocOnAmbientLight(), onAmbientLight);
    glUniform1i(phongShadingShader->GetLocOnDiffuseLight(), onDiffuseLight);
    glUniform1i(phongShadingShader->GetLocOnSpecularLight(), onSpecularLight);

    // point light
    if (pointLight != nullptr) {
      glUniform3fv(phongShadingShader->GetLocPointLightPos(), 1,
                   glm::value_ptr(pointLight->GetPosition()));
      glUniform3fv(phongShadingShader->GetLocPointLightIntensity(), 1,
                   glm::value_ptr(pointLight->GetIntensity()));
    }

    // spot light
    if (spotLight != nullptr) {
      glUniform3fv(phongShadingShader->GetLocSpotLightPos(), 1,
                   glm::value_ptr(spotLight->GetPosition()));
      glUniform3fv(phongShadingShader->GetLocSpotLightIntensity(), 1,
                   glm::value_ptr(spotLight->GetIntensity()));
      glUniform3fv(phongShadingShader->GetLocSpotLightDir(), 1,
                   glm::value_ptr(spotLight->GetDirection()));
      glUniform1f(phongShadingShader->GetLocSpotLightCutOffStart(),
                  spotLight->GetCutoffStart());
      glUniform1f(phongShadingShader->GetLocSpotLightTotalWidth(),
                  spotLight->GetTotalWidth());
    }

    // upload per material data and draw
    for (auto& subMesh : subMeshes) {
      glUniform3fv(phongShadingShader->GetLocKa(), 1,
                   glm::value_ptr(subMesh.material->GetKa()));

      glUniform3fv(phongShadingShader->GetLocKd(), 1,
                   glm::value_ptr(subMesh.material->GetKd()));

      glUniform3fv(phongShadingShader->GetLocKs(), 1,
                   glm::value_ptr(subMesh.material->GetKs()));
      glUniform1f(phongShadingShader->GetLocNs(), subMesh.material->GetNs());

      subMesh.draw();
    }
  }

  phongShadingShader->UnBind();

  // Visualize the light with fill color.
  // ------------------------------------------------------
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------
  PointLight* pointLight = pointLightObj.light;
  if (pointLight != nullptr) {
    glm::mat4x4 T =
        glm::translate(glm::mat4x4(1.0f), pointLight->GetPosition());
    pointLightObj.worldMatrix = T;
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      pointLightObj.worldMatrix;
    fillColorShader->Bind();
    glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniform3fv(fillColorShader->GetLocFillColor(), 1,
                 glm::value_ptr(pointLightObj.visColor));
    // Render the point light.
    pointLight->Draw();
    fillColorShader->UnBind();
  }
  SpotLight* spotLight = (SpotLight*)(spotLightObj.light);
  if (spotLight != nullptr) {
    glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), spotLight->GetPosition());
    spotLightObj.worldMatrix = T;
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      spotLightObj.worldMatrix;
    fillColorShader->Bind();
    glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniform3fv(fillColorShader->GetLocFillColor(), 1,
                 glm::value_ptr(spotLightObj.visColor));
    // Render the point light.
    spotLight->Draw();
    fillColorShader->UnBind();
  }

  // Visualize the directional light with fill color.

  DirectionalLight* dirLight = dirLightObj.light;
  if (dirLight != nullptr && showDirLightArrow) {
    // 將深度測試關掉 這樣箭頭不會被模型擋住
    glDisable(GL_DEPTH_TEST);
    // 使用固定位置
    glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), dirLightObj.position);

    glm::vec3 dir = -dirLight->GetDirection();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(dir, up));
    up = glm::normalize(glm::cross(right, dir));

    // 構建旋轉矩陣，使箭頭對齊光的方向
    glm::mat4x4 R =
        glm::mat4x4(right.x, right.y, right.z, 0.0f, up.x, up.y, up.z, 0.0f,
                    dir.x, dir.y, dir.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    dirLightObj.worldMatrix = T * R;

    // 渲染箭頭
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      dirLightObj.worldMatrix;
    fillColorShader->Bind();
    glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniform3fv(fillColorShader->GetLocFillColor(), 1,
                 glm::value_ptr(dirLightObj.visColor));
    dirLight->Draw();
    fillColorShader->UnBind();

    glEnable(GL_DEPTH_TEST);  // 恢復深度測試
  }
}

void ReshapeCB(GLFWwindow* window, int w, int h) {
  // Update viewport.
  screenWidth = w;
  screenHeight = h;
  glViewport(0, 0, screenWidth, screenHeight);
  // Adjust camera and projection.
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void ProcessKeysCB(GLFWwindow* window, int key, int scancode, int action,
                   int mods) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT) {
    return;
  }

  switch (key) {
    case GLFW_KEY_ESCAPE:
      ReleaseResources();
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;

    // 渲染模式
    case GLFW_KEY_F1:
      // 點模式渲染
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    case GLFW_KEY_F2:
      // 線框模式渲染
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case GLFW_KEY_F3:
      // 填充模式渲染
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;

    // 光源控制
    case GLFW_KEY_LEFT:
      if (pointLight != nullptr) pointLight->MoveLeft(lightMoveSpeed);
      break;
    case GLFW_KEY_RIGHT:
      if (pointLight != nullptr) pointLight->MoveRight(lightMoveSpeed);
      break;
    case GLFW_KEY_UP:
      if (pointLight != nullptr) pointLight->MoveUp(lightMoveSpeed);
      break;
    case GLFW_KEY_DOWN:
      if (pointLight != nullptr) pointLight->MoveDown(lightMoveSpeed);
      break;
    // spot light
    case GLFW_KEY_A:
      if (spotLight != nullptr) spotLight->MoveLeft(lightMoveSpeed);
      break;
    case GLFW_KEY_D:
      if (spotLight != nullptr) spotLight->MoveRight(lightMoveSpeed);
      break;
    case GLFW_KEY_W:
      if (spotLight != nullptr) spotLight->MoveUp(lightMoveSpeed);
      break;
    case GLFW_KEY_S:
      if (spotLight != nullptr) spotLight->MoveDown(lightMoveSpeed);
      break;
    default:
      break;
  }
}
void SetupRenderState() {
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);

  glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
  glClearColor((GLclampf)(clearColor.r), (GLclampf)(clearColor.g),
               (GLclampf)(clearColor.b), (GLclampf)(clearColor.a));
}

void LoadObjects(const std::string& modelPath) {
  // -------------------------------------------------------
  // Note: you can change the code below if you want to load
  //       the model dynamically.
  // -------------------------------------------------------
  if (mesh != nullptr) {
    delete mesh;
    mesh = nullptr;
  }

  mesh = new TriangleMesh();
  mesh->LoadFromFile(modelPath, true);
  mesh->ShowInfo();
  mesh->createBuffer();

  sceneObj.mesh = mesh;
}

void CreateLights() {
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------

  // Create a directional light.
  dirLight = new DirectionalLight(dirLightDirection, dirLightRadiance);
  dirLightObj.light = dirLight;
  dirLightObj.visColor = glm::normalize((dirLightObj.light)->GetRadiance());
  // Create a point light.
  pointLight = new PointLight(pointLightPosition, pointLightIntensity);
  pointLightObj.light = pointLight;
  pointLightObj.visColor =
      glm::normalize((pointLightObj.light)->GetIntensity());
  // Create a spot light.
  spotLight =
      new SpotLight(spotLightPosition, spotLightIntensity, spotLightDirection,
                    spotLightCutoffStartInDegree, spotLightTotalWidthInDegree);
  spotLightObj.light = spotLight;
  spotLightObj.visColor = glm::normalize((spotLightObj.light)->GetIntensity());
}

void CreateCamera() {
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------

  // Create a camera and update view and proj matrices.
  camera = new Camera((float)screenWidth / (float)screenHeight);
  camera->UpdateView(cameraPos, cameraTarget, cameraUp);
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateShaderLib() {
  // ----------------------------------------------------
  // You do not need to change the code.
  // ----------------------------------------------------

  GLuint Vao;

  glGenVertexArrays(1, &Vao);

  glBindVertexArray(Vao);

  fillColorShader = new FillColorShaderProg();
  if (!fillColorShader->LoadFromFiles("shaders/fixed_color.vs",
                                      "shaders/fixed_color.fs"))
    exit(1);

  phongShadingShader = new PhongShadingDemoShaderProg();
  if (!phongShadingShader->LoadFromFiles("shaders/phong_shading_demo.vs",
                                         "shaders/phong_shading_demo.fs"))
    exit(1);
}

int main(int argc, char** argv) {
  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed!" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow* window = glfwCreateWindow(
      screenWidth, screenHeight, "HW2: Lighting and Shading", NULL, NULL);

  if (!window) {
    std::cerr << "GLFW window creation failed!" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  // Initialize GLEW.
  // Must be done after glfw is initialized!
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    std::cerr << "GLEW initialization error: " << glewGetErrorString(res)
              << std::endl;
    return 1;
  }

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

  // Initialization.
  SetupRenderState();
  LoadObjects(defaultModelPath);
  CreateLights();
  CreateCamera();
  CreateShaderLib();

  // Register callback functions.
  glfwSetFramebufferSizeCallback(window, ReshapeCB);
  glfwSetKeyCallback(window, ProcessKeysCB);

  // Initialize ImGui.
  gui = new GUI(window);

  std::vector<std::string> objFileDirectory =
      getFilesInDirectory(modelDirectory);

  // Enter main event loop.
  while (!glfwWindowShouldClose(window)) {
    RenderSceneCB();
    // Render ImGui.
    gui->render(dirLight, LoadObjects, objFileDirectory, isBlingPhong,
                showDirLightArrow, onPointLight, onSpotLight, onDirLight, onAmbientLight,
                onDiffuseLight, onSpecularLight);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Release resources.
  ReleaseResources();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}