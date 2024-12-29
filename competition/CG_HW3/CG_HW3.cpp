#include "camera.h"
#include "gui.h"
#include "headers.h"
#include "imagetexture.h"
#include "light.h"
#include "scene.h"
#include "shaderprog.h"
#include "skybox.h"
#include "trianglemesh.h"

const std::string modelDirectory = "../TestModels_HW3/";
const std::string skyboxDirectory = "../TestTextures_HW3/";
const std::string defaultModelPath = "../TestModels_HW3/TexCube/TexCube.obj";
const std::string fbxRoomModelPath = "../TestModels_HW3/scene/scene.fbx";
// Global variables.
int screenWidth = 600;
int screenHeight = 600;

float scale = 1.0f;
// GUI.
GUI *gui = nullptr;
// Lights.
DirectionalLight *dirLight = nullptr;
PointLight *pointLight = nullptr;
SpotLight *spotLight = nullptr;
glm::vec3 dirLightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.6f, 0.6f, 0.6f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.0f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(0.5f, 0.1f, 0.1f);
glm::vec3 spotLightPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightIntensity = glm::vec3(0.25f, 0.25f, 0.1f);
float spotLightCutoffStartInDegree = 30.0f;
float spotLightTotalWidthInDegree = 45.0f;
glm::vec3 ambientLight = glm::vec3(0.2f, 0.2f, 0.2f);
// Camera.
Camera *camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;
// Shader.
FillColorShaderProg *fillColorShader = nullptr;
PhongShadingDemoShaderProg *phongShadingShader = nullptr;
SkyboxShaderProg *skyboxShader = nullptr;
bool isBlingPhong = true;
// Light control.
bool showDirLightArrow = true;
float dirLightArrowScale = 1.0f;
bool onPointLight = true;
bool onSpotLight = true;
bool onDirLight = true;
bool onAmbientLight = true;
bool onDiffuseLight = true;
bool onSpecularLight = true;
// UI.
const float lightMoveSpeed = 0.2f;
// Skybox.
Skybox *skybox = nullptr;

Scene *scene = nullptr;

// Function prototypes.
void ReleaseResources();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(GLFWwindow, int, int);
void ProcessKeysCB(GLFWwindow, int, int, int, int);
void SetupRenderState();
void LoadObjects(const std::string &);
void CreateCamera();
void CreateSkybox(const std::string);
void CreateShaderLib();
void CreateScene();

void ReleaseResources() {
  // Delete scene objects and lights.
  if (pointLight != nullptr) {
    delete pointLight;
    pointLight = nullptr;
  }
  if (dirLight != nullptr) {
    delete dirLight;
    dirLight = nullptr;
  }
  if (spotLight != nullptr) {
    delete spotLight;
    spotLight = nullptr;
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
  if (skyboxShader != nullptr) {
    delete skyboxShader;
    skyboxShader = nullptr;
  }
}

static float curObjRotationY = 0.0f;
static float curObjRotationX = 0.0f;

static float skyboxRotation = 0.0f;
// 新增輔助函數
void UploadDirectionalLights(PhongShadingDemoShaderProg *shader,
                             const std::vector<DirectionalLight *> &dirLights) {
  glUniform1i(shader->locNumDirLights, static_cast<GLint>(dirLights.size()));
  for (int i = 0; i < dirLights.size(); i++) {
    glUniform3fv(shader->dirLightLocations[i].direction, 1,
                 glm::value_ptr(dirLights[i]->GetDirection()));
    glUniform3fv(shader->dirLightLocations[i].radiance, 1,
                 glm::value_ptr(dirLights[i]->GetIntensity()));
  }
}

void UploadPointLights(PhongShadingDemoShaderProg *shader,
                       const std::vector<PointLight *> &pointLights) {
  glUniform1i(shader->locNumPointLights,
              static_cast<GLint>(pointLights.size()));
  for (int i = 0; i < pointLights.size(); i++) {
    glUniform3fv(shader->pointLightLocations[i].position, 1,
                 glm::value_ptr(pointLights[i]->GetPosition()));
    glUniform3fv(shader->pointLightLocations[i].intensity, 1,
                 glm::value_ptr(pointLights[i]->GetIntensity()));
    glUniform1f(shader->pointLightLocations[i].constant,
                pointLights[i]->GetConstant());
    glUniform1f(shader->pointLightLocations[i].linear,
                pointLights[i]->GetLinear());
    glUniform1f(shader->pointLightLocations[i].quadratic,
                pointLights[i]->GetQuadratic());
    glUniform1f(shader->pointLightLocations[i].decayStart,
                pointLights[i]->GetDecayStart());
  }
}

void UploadSpotLights(PhongShadingDemoShaderProg *shader,
                      const std::vector<SpotLight *> &spotLights) {
  glUniform1i(shader->locNumSpotLights, static_cast<GLint>(spotLights.size()));
  for (int i = 0; i < spotLights.size(); i++) {
    glUniform3fv(shader->spotLightLocations[i].position, 1,
                 glm::value_ptr(spotLights[i]->GetPosition()));
    glUniform3fv(shader->spotLightLocations[i].direction, 1,
                 glm::value_ptr(spotLights[i]->GetDirection()));
    glUniform3fv(shader->spotLightLocations[i].intensity, 1,
                 glm::value_ptr(spotLights[i]->GetIntensity()));
    glUniform1f(shader->spotLightLocations[i].cosCutoffStart,
                spotLights[i]->GetCosCutoffStart());
    glUniform1f(shader->spotLightLocations[i].cosCutoffEnd,
                spotLights[i]->GetCosCutoffEnd());
    glUniform1f(shader->spotLightLocations[i].constant,
                spotLights[i]->GetConstant());
    glUniform1f(shader->spotLightLocations[i].linear,
                spotLights[i]->GetLinear());
    glUniform1f(shader->spotLightLocations[i].quadratic,
                spotLights[i]->GetQuadratic());
    glUniform1f(shader->spotLightLocations[i].decayStart,
                spotLights[i]->GetDecayStart());
  }
}

void UploadAreaLights(PhongShadingDemoShaderProg *shader,
                      const std::vector<AreaLight *> &areaLights) {
  glUniform1i(shader->locNumAreaLights, static_cast<GLint>(areaLights.size()));
  for (int i = 0; i < areaLights.size(); i++) {
    glUniform3fv(shader->areaLightLocations[i].position, 1,
                 glm::value_ptr(areaLights[i]->GetPosition()));
    glUniform3fv(shader->areaLightLocations[i].intensity, 1,
                 glm::value_ptr(areaLights[i]->GetIntensity()));
    glUniform3fv(shader->areaLightLocations[i].direction, 1,
                 glm::value_ptr(areaLights[i]->GetDirection()));
    glUniform1f(shader->areaLightLocations[i].width, areaLights[i]->GetWidth());
    glUniform1f(shader->areaLightLocations[i].height,
                areaLights[i]->GetHeight());
    glUniform1i(shader->areaLightLocations[i].samples,
                areaLights[i]->GetSamples());
    glUniform1f(shader->areaLightLocations[i].constant,
                areaLights[i]->GetConstant());
    glUniform1f(shader->areaLightLocations[i].linear,
                areaLights[i]->GetLinear());
    glUniform1f(shader->areaLightLocations[i].quadratic,
                areaLights[i]->GetQuadratic());
    glUniform1f(shader->areaLightLocations[i].decayStart,
                areaLights[i]->GetDecayStart());
  }
}

void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render a triangle mesh with Phong shading.
  Camera *camera = scene->camera;

  phongShadingShader->Bind();

  // 上傳環境光
  glUniform3fv(phongShadingShader->GetLocAmbientLight(), 1,
               glm::value_ptr(scene->ambientLight));

  // 上傳各類光源
  UploadDirectionalLights(phongShadingShader, scene->dirLights);
  UploadPointLights(phongShadingShader, scene->pointLights);
  UploadSpotLights(phongShadingShader, scene->spotLights);
  UploadAreaLights(phongShadingShader, scene->areaLights);

  glUniformMatrix4fv(phongShadingShader->GetLocV(), 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));

  // 設置燈光開關
  glUniform1i(phongShadingShader->GetLocIsBlingPhong(), isBlingPhong);
  glUniform1i(phongShadingShader->GetLocOnAmbientLight(), onAmbientLight);
  glUniform1i(phongShadingShader->GetLocOnDiffuseLight(), onDiffuseLight);
  glUniform1i(phongShadingShader->GetLocOnSpecularLight(), onSpecularLight);

  for (auto sceneObj : scene->objects) {
    // Update transform.
    glm::mat4x4 S =
        glm::scale(glm::mat4x4(1.0f), glm::vec3(scale, scale, scale));
    glm::mat4x4 RY = glm::rotate(
        glm::mat4x4(1.0f), glm::radians(curObjRotationY), glm::vec3(0, 1, 0));
    glm::mat4x4 RX = glm::rotate(
        glm::mat4x4(1.0f), glm::radians(curObjRotationX), glm::vec3(1, 0, 0));
    sceneObj.worldMatrix = S * RY * RX;
    glm::mat4x4 normalMatrix = glm::transpose(
        glm::inverse(camera->GetViewMatrix() * sceneObj.worldMatrix));
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      sceneObj.worldMatrix;
    glUniformMatrix4fv(phongShadingShader->GetLocM(), 1, GL_FALSE,
                       glm::value_ptr(sceneObj.worldMatrix));
    glUniformMatrix4fv(phongShadingShader->GetLocNM(), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));
    glUniformMatrix4fv(phongShadingShader->GetLocV(), 1, GL_FALSE,
                       glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(phongShadingShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    sceneObj.mesh->draw(phongShadingShader);
  }

  phongShadingShader->UnBind();

  // Render skybox.
  if (skybox != nullptr) {
    skybox->SetRotation(skyboxRotation);
    skybox->Render(camera, skyboxShader);
  }
}

void ReshapeCB(GLFWwindow *window, int w, int h) {
  // Update viewport.
  screenWidth = w;
  screenHeight = h;
  glViewport(0, 0, screenWidth, screenHeight);
  // Adjust camera and projection.
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void ProcessKeysCB(GLFWwindow *window, int key, int scancode, int action,
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

    // 相機控制
    case GLFW_KEY_W:
      scene->camera->moveForward(0.1f);
      break;

    case GLFW_KEY_S:
      scene->camera->moveBackward(0.1f);
      break;

    case GLFW_KEY_A:
      scene->camera->moveLeft(0.1f);
      break;

    case GLFW_KEY_D:
      scene->camera->moveRight(0.1f);
      break;

    case GLFW_KEY_Q:
      scene->camera->moveUp(0.1f);
      break;

    case GLFW_KEY_E:
      scene->camera->moveDown(0.1f);
      break;

      // 相機旋轉

    case GLFW_KEY_UP:
      scene->camera->rotate(0.0f, 1.0f);
      break;

    case GLFW_KEY_DOWN:
      scene->camera->rotate(0.0f, -1.0f);
      break;

    case GLFW_KEY_LEFT:
      scene->camera->rotate(-1.0f, 0.0f);
      break;

    case GLFW_KEY_RIGHT:
      scene->camera->rotate(1.0f, 0.0f);
      break;
    case GLFW_KEY_KP_ADD:
      scale *= 2.0f;
      break;
    case GLFW_KEY_KP_SUBTRACT:
      scale /= 2.0f;
      break;
    default:
      break;
  }
}

void SetupRenderState() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
  glClearColor((GLclampf)(clearColor.r), (GLclampf)(clearColor.g),
               (GLclampf)(clearColor.b), (GLclampf)(clearColor.a));
}

void CreateScene() {
  if (scene != nullptr) {
    delete scene;
    scene = nullptr;
  }

  scene = new Scene();
}

void LoadObjects(const std::string &modelPath) {
  // -------------------------------------------------------
  // Note: you can change the code below if you want to load
  //       the model dynamically.
  // -------------------------------------------------------

  TriangleMesh *mesh = new TriangleMesh();
  mesh->LoadFromFile(modelPath, false, scene);

  if(scene->camera == nullptr) {
    scene->camera = camera;
  }

  // scene->camera = camera;

  mesh->createBuffer();
  mesh->ShowInfo();
  SceneObject sceneObj;

  sceneObj.mesh = mesh;

  scene->objects.push_back(sceneObj);
}

void CreateCamera() {
  // Create a camera and update view and proj matrices.
  camera = new Camera((float)screenWidth / (float)screenHeight);
  camera->UpdateView(cameraPos, cameraTarget, cameraUp);
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateSkybox(const std::string texFilePath) {
  // -------------------------------------------------------
  // Note: you can change the code below if you want to change
  //       the skybox texture dynamically.
  // -------------------------------------------------------

  if (skybox != nullptr) {
    delete skybox;
    skybox = nullptr;
  }

  const int numSlices = 36;
  const int numStacks = 18;

  const float radius = 50.0f;
  skybox = new Skybox(texFilePath, numSlices, numStacks, radius);
}

void CreateShaderLib() {
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

  skyboxShader = new SkyboxShaderProg();
  if (!skyboxShader->LoadFromFiles("shaders/skybox.vs", "shaders/skybox.fs"))
    exit(1);
}

int main(int argc, char **argv) {
  std::ofstream outFile("output.txt");
  if (!outFile) {
    std::cerr << "無法開啟檔案進行輸出。" << std::endl;
    return 1;
  }
  std::cout.rdbuf(outFile.rdbuf());
  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed!" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 8);

  GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight,
                                        "Texture Mapping", NULL, NULL);

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
  CreateScene();
  SetupRenderState();
  CreateCamera();
  LoadObjects(fbxRoomModelPath);
  CreateSkybox("textures/photostudio_02_2k.png");
  CreateShaderLib();

  // Register callback functions.
  glfwSetFramebufferSizeCallback(window, ReshapeCB);
  glfwSetKeyCallback(window, ProcessKeysCB);

  // Initialize ImGui.
  gui = new GUI(window);
  GUIState guiState = GUIState(
      isBlingPhong, showDirLightArrow, onPointLight, onSpotLight, onDirLight,
      onAmbientLight, onDiffuseLight, onSpecularLight, dirLightArrowScale,
      curObjRotationX, curObjRotationY, skyboxRotation);

  std::vector<std::string> objFileDirectory =
      Utils::getFilesInDirectory(modelDirectory, ".obj");

  std::vector<std::string> skyboxFileDirectory =
      Utils::getFilesInDirectory(skyboxDirectory, ".png");

  // Enter main event loop.
  while (!glfwWindowShouldClose(window)) {
    RenderSceneCB();
    // Render ImGui.
    gui->render(dirLight, LoadObjects, CreateSkybox, objFileDirectory,
                skyboxFileDirectory, guiState);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Release resources.
  ReleaseResources();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
