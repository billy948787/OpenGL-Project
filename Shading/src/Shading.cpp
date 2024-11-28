#include "camera.h"
#include "headers.h"
#include "light.h"
#include "shaderprog.h"
#include "sphere.h"

// Global variables.
int screenWidth = 600;
int screenHeight = 600;
// Scene object.
Sphere* sphereMesh = nullptr;
// Lights objects and parameters.
DirectionalLight* dirLight = nullptr;
PointLight* pointLight = nullptr;
glm::vec3 dirLightDirection = glm::vec3(1.0f, -1.0f, 0.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.3f, 0.3f, 0.9f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.8f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(0.5f, 0.1f, 0.1f);
glm::vec3 ambientLight = glm::vec3(0.6f, 0.6f, 0.6f);
// Camera object parameters.
Camera* camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;
// Shaders.
FillColorShaderProg* fillColorShader = nullptr;
GouraudShadingDemoShaderProg* gouraudShadingShader = nullptr;
// UI.
const float lightMoveSpeed = 0.2f;

// SceneObject.
struct SceneObject {
  SceneObject() {
    mesh = nullptr;
    worldMatrix = glm::mat4x4(1.0f);
    Ka = glm::vec3(0.5f, 0.5f, 0.5f);
    Kd = glm::vec3(0.8f, 0.8f, 0.8f);
    Ks = glm::vec3(0.6f, 0.6f, 0.6f);
    Ns = 10000.0f;
  }
  Sphere* mesh;
  glm::mat4x4 worldMatrix;
  // Material properties.
  glm::vec3 Ka;
  glm::vec3 Kd;
  glm::vec3 Ks;
  float Ns;
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

// Function prototypes.
void ReleaseResources();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(GLFWwindow, int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(GLFWwindow, int, int, int, int);
void SetupRenderState();
void SetupScene();
void CreateShaderLib();

void ReleaseResources() {
  // Delete scene objects and lights.
  if (sphereMesh != nullptr) {
    delete sphereMesh;
    sphereMesh = nullptr;
  }
  if (pointLight != nullptr) {
    delete pointLight;
    pointLight = nullptr;
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
  if (gouraudShadingShader != nullptr) {
    delete gouraudShadingShader;
    gouraudShadingShader = nullptr;
  }
}

void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render a triangle mesh with Gouraud shading.
  // ----------------------------------------------
  if (sceneObj.mesh != nullptr) {
    // Update transform (assuming there might be dynamic transformations).
    glm::mat4x4 S = glm::scale(glm::mat4x4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    sceneObj.worldMatrix = S;
    glm::mat4x4 normalMatrix = glm::transpose(
        glm::inverse(camera->GetViewMatrix() * sceneObj.worldMatrix));
    glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() *
                      sceneObj.worldMatrix;

    gouraudShadingShader->Bind();

    // Transformation matrix.
    glUniformMatrix4fv(gouraudShadingShader->GetLocM(), 1, GL_FALSE,
                       glm::value_ptr(sceneObj.worldMatrix));
    glUniformMatrix4fv(gouraudShadingShader->GetLocV(), 1, GL_FALSE,
                       glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(gouraudShadingShader->GetLocNM(), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));
    glUniformMatrix4fv(gouraudShadingShader->GetLocMVP(), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    // Material properties.
    glUniform3fv(gouraudShadingShader->GetLocKa(), 1,
                 glm::value_ptr(sceneObj.Ka));
    glUniform3fv(gouraudShadingShader->GetLocKd(), 1,
                 glm::value_ptr(sceneObj.Kd));
    glUniform3fv(gouraudShadingShader->GetLocKs(), 1,
                 glm::value_ptr(sceneObj.Ks));
    glUniform1f(gouraudShadingShader->GetLocNs(), sceneObj.Ns);
    // Light data.
    if (dirLight != nullptr) {
      glUniform3fv(gouraudShadingShader->GetLocDirLightDir(), 1,
                   glm::value_ptr(dirLight->GetDirection()));
      glUniform3fv(gouraudShadingShader->GetLocDirLightRadiance(), 1,
                   glm::value_ptr(dirLight->GetRadiance()));
    }
    if (pointLight != nullptr) {
      glUniform3fv(gouraudShadingShader->GetLocPointLightPos(), 1,
                   glm::value_ptr(pointLight->GetPosition()));
      glUniform3fv(gouraudShadingShader->GetLocPointLightIntensity(), 1,
                   glm::value_ptr(pointLight->GetIntensity()));
    }
    glUniform3fv(gouraudShadingShader->GetLocAmbientLight(), 1,
                 glm::value_ptr(ambientLight));

    // Render the mesh.
    sceneObj.mesh->Render();

    gouraudShadingShader->UnBind();
  }
  // -------------------------------------------------------------------------------------------

  // Visualize the light with fill color.
  // ------------------------------------------------------ Bind shader and set
  // parameters.
  PointLight* pointLight = pointLightObj.light;
  if (pointLight != nullptr) {
    glm::mat4x4 T =
        glm::translate(glm::mat4x4(1.0f), (pointLight->GetPosition()));
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
  // -------------------------------------------------------------------------------------------
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

void ProcessSpecialKeysCB(GLFWwindow* window, int key, int scancode, int action,
                          int mods) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT) {
    return;
  }

  switch (key) {
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
    default:
      break;
  }
}

void ProcessKeysCB(GLFWwindow* window, int key, int scancode, int action,
                   int mods) {
  // Handle other keyboard inputs those are not defined as special keys.
  if (key == 27) {
    // Release memory allocation if needed.
    ReleaseResources();
    exit(0);
  }
}

void SetupRenderState() {
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);

  glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
  glClearColor((GLclampf)(clearColor.r), (GLclampf)(clearColor.g),
               (GLclampf)(clearColor.b), (GLclampf)(clearColor.a));
}

void SetupScene() {
  // Scene object
  // ------------------------------------------------------------------------------
  sphereMesh = new Sphere(32, 32, 0.5f);
  sceneObj.mesh = sphereMesh;

  // Scene lights
  // ------------------------------------------------------------------------------
  // Create a directional light.
  dirLight = new DirectionalLight(dirLightDirection, dirLightRadiance);
  // Create a point light.
  pointLight = new PointLight(pointLightPosition, pointLightIntensity);
  pointLightObj.light = pointLight;
  pointLightObj.visColor =
      glm::normalize(((PointLight*)pointLightObj.light)->GetIntensity());

  // Create a camera and update view and proj matrices.
  camera = new Camera((float)screenWidth / (float)screenHeight);
  camera->UpdateView(cameraPos, cameraTarget, cameraUp);
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateShaderLib() {
  fillColorShader = new FillColorShaderProg();
  if (!fillColorShader->LoadFromFiles("shaders/fixed_color.vs",
                                      "shaders/fixed_color.fs"))
    exit(1);

  gouraudShadingShader = new GouraudShadingDemoShaderProg();
  if (!gouraudShadingShader->LoadFromFiles("shaders/gouraud_shading_demo.vs",
                                           "shaders/gouraud_shading_demo.fs"))
    exit(1);
}

int main(int argc, char** argv) {
  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed!" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow* window = glfwCreateWindow(
      screenWidth, screenHeight, "HW2: Lighting and Shading", NULL, NULL);

  if (!window) {
    std::cerr << "GLFW window creation failed!" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  // Initialize GLEW.
  // Must be done after glut is initialized!
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    std::cerr << "GLEW initialization error: " << glewGetErrorString(res)
              << std::endl;
    return 1;
  }

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

  // Initialization.
  SetupRenderState();
  SetupScene();
  CreateShaderLib();

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Register callback functions.
  glfwSetFramebufferSizeCallback(window, ReshapeCB);
  glfwSetKeyCallback(window, ProcessSpecialKeysCB);

  // Enter main event loop.
  while (!glfwWindowShouldClose(window)) {
    RenderSceneCB();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Release resources.
  ReleaseResources();
  glfwDestroyWindow(window);
  glfwTerminate();
  glDeleteVertexArrays(1, &VAO);

  return 0;
}
