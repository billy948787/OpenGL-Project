#include "shaderprog.h"

#define MAX_BUFFER_SIZE 1024

ShaderProg::ShaderProg() {
  // Create OpenGL shader program.
  shaderProgId = glCreateProgram();
  if (shaderProgId == 0) {
    std::cerr << "[ERROR] Failed to create shader program" << std::endl;
    exit(1);
  }
  // Initialize uniform locations
  locMVP = -1;
  locNumDirLights = -1;
  locNumPointLights = -1;
  locNumSpotLights = -1;
  locNumAreaLights = -1;  // 初始化區域光數量
}

ShaderProg::~ShaderProg() { glDeleteProgram(shaderProgId); }

bool ShaderProg::LoadFromFiles(const std::string vsFilePath,
                               const std::string fsFilePath) {
  // Load the vertex shader from a source file and attach it to the shader
  // program.
  std::string vs, fs;
  if (!LoadShaderTextFromFile(vsFilePath, vs)) {
    std::cerr << "[ERROR] Failed to load vertex shader source: " << vsFilePath
              << std::endl;
    return false;
  }
  GLuint vsId = AddShader(vs, GL_VERTEX_SHADER);

  // Load the fragment shader from a source file and attach it to the shader
  // program.
  if (!LoadShaderTextFromFile(fsFilePath, fs)) {
    std::cerr << "[ERROR] Failed to load fragment shader source: " << fsFilePath
              << std::endl;
    return false;
  };
  GLuint fsId = AddShader(fs, GL_FRAGMENT_SHADER);

  // Link and compile shader programs.
  GLint success = 0;
  GLchar errorLog[MAX_BUFFER_SIZE] = {0};
  glLinkProgram(shaderProgId);
  glGetProgramiv(shaderProgId, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(shaderProgId, sizeof(errorLog), NULL, errorLog);
    std::cerr << "[ERROR] Failed to link shader program: " << errorLog
              << std::endl;
    return false;
  }

  // Now the program already has all stage information, we can delete the
  // shaders now.
  glDeleteShader(vsId);
  glDeleteShader(fsId);

  // Validate program.
  glValidateProgram(shaderProgId);
  glGetProgramiv(shaderProgId, GL_VALIDATE_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgId, sizeof(errorLog), NULL, errorLog);
    std::cerr << "[ERROR] Invalid shader program: " << errorLog << std::endl;
    return false;
  }

  // Update the location of uniform variables.
  GetUniformVariableLocation();

  return true;
}

void ShaderProg::GetUniformVariableLocation() {
  locMVP = glGetUniformLocation(shaderProgId, "MVP");

  // 方向光
  locNumDirLights = glGetUniformLocation(shaderProgId, "numDirLights");
  for (int i = 0; i < MAX_DIR_LIGHTS; i++) {
    dirLightLocations[i].direction = glGetUniformLocation(
        shaderProgId,
        ("dirLights[" + std::to_string(i) + "].direction").c_str());
    dirLightLocations[i].radiance = glGetUniformLocation(
        shaderProgId,
        ("dirLights[" + std::to_string(i) + "].radiance").c_str());
  }

  // 點光源
  locNumPointLights = glGetUniformLocation(shaderProgId, "numPointLights");
  for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
    pointLightLocations[i].position = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].position").c_str());
    pointLightLocations[i].intensity = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].intensity").c_str());
    pointLightLocations[i].constant = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].constant").c_str());
    pointLightLocations[i].linear = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].linear").c_str());
    pointLightLocations[i].quadratic = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].quadratic").c_str());
    pointLightLocations[i].decayStart = glGetUniformLocation(
        shaderProgId,
        ("pointLights[" + std::to_string(i) + "].decayStart").c_str());
  }

  // 聚光燈
  locNumSpotLights = glGetUniformLocation(shaderProgId, "numSpotLights");
  for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
    spotLightLocations[i].position = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].position").c_str());
    spotLightLocations[i].direction = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].direction").c_str());
    spotLightLocations[i].intensity = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].intensity").c_str());
    spotLightLocations[i].cosCutoffStart = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].cosCutoffStart").c_str());
    spotLightLocations[i].cosCutoffEnd = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].cosCutoffEnd").c_str());
    spotLightLocations[i].constant = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].constant").c_str());
    spotLightLocations[i].linear = glGetUniformLocation(
        shaderProgId, ("spotLights[" + std::to_string(i) + "].linear").c_str());
    spotLightLocations[i].quadratic = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].quadratic").c_str());
    spotLightLocations[i].decayStart = glGetUniformLocation(
        shaderProgId,
        ("spotLights[" + std::to_string(i) + "].decayStart").c_str());
  }

  // 區域光
  locNumAreaLights = glGetUniformLocation(shaderProgId, "numAreaLights");
  for (int i = 0; i < MAX_AREA_LIGHTS; i++) {
    areaLightLocations[i].position = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].position").c_str());
    areaLightLocations[i].intensity = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].intensity").c_str());
    areaLightLocations[i].direction = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].direction").c_str());
    areaLightLocations[i].width = glGetUniformLocation(
        shaderProgId, ("areaLights[" + std::to_string(i) + "].width").c_str());
    areaLightLocations[i].height = glGetUniformLocation(
        shaderProgId, ("areaLights[" + std::to_string(i) + "].height").c_str());
    areaLightLocations[i].samples = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].samples").c_str());  // 新增
    areaLightLocations[i].constant = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].constant").c_str());
    areaLightLocations[i].linear = glGetUniformLocation(
        shaderProgId, ("areaLights[" + std::to_string(i) + "].linear").c_str());
    areaLightLocations[i].quadratic = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].quadratic").c_str());
    areaLightLocations[i].decayStart = glGetUniformLocation(
        shaderProgId,
        ("areaLights[" + std::to_string(i) + "].decayStart").c_str());
  }
}

GLuint ShaderProg::AddShader(const std::string &sourceText, GLenum shaderType) {
  GLuint shaderObj = glCreateShader(shaderType);
  if (shaderObj == 0) {
    std::cerr << "[ERROR] Failed to create shader with type " << shaderType
              << std::endl;
    exit(0);
  }

  const GLchar *p[1];
  p[0] = sourceText.c_str();
  GLint lengths[1];
  lengths[0] = (GLint)(sourceText.length());
  glShaderSource(shaderObj, 1, p, lengths);
  glCompileShader(shaderObj);

  GLint success;
  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[MAX_BUFFER_SIZE];
    glGetShaderInfoLog(shaderObj, MAX_BUFFER_SIZE, NULL, infoLog);
    std::cerr << "[ERROR] Failed to compile shader with type: " << shaderType
              << ". Info: " << infoLog << std::endl;
    exit(1);
  }

  glAttachShader(shaderProgId, shaderObj);

  return shaderObj;
}

bool ShaderProg::LoadShaderTextFromFile(const std::string filePath,
                                        std::string &sourceText) {
  std::ifstream sourceFile(filePath.c_str());
  if (!sourceFile) {
    std::cerr << "[ERROR] Failed to open shader source file: " << filePath
              << std::endl;
    return false;
  }
  sourceText.assign((std::istreambuf_iterator<char>(sourceFile)),
                    std::istreambuf_iterator<char>());
  return true;
}

// 其他 ShaderProg 類別實作...

// ------------------------------------------------------------------------------------------------

FillColorShaderProg::FillColorShaderProg() { locFillColor = -1; }

FillColorShaderProg::~FillColorShaderProg() {}

void FillColorShaderProg::GetUniformVariableLocation() {
  ShaderProg::GetUniformVariableLocation();
  locFillColor = glGetUniformLocation(shaderProgId, "fillColor");
}

// ------------------------------------------------------------------------------------------------

PhongShadingDemoShaderProg::PhongShadingDemoShaderProg() {
  locM = -1;
  locV = -1;
  locNM = -1;
  locCameraPos = -1;
  locKa = -1;
  locKd = -1;
  locKs = -1;
  locNs = -1;
  locAmbientLight = -1;
  locIsBlingPhong = -1;
  locOnAmbientLight = -1;
  locOnDiffuseLight = -1;
  locOnSpecularLight = -1;
  locMapKd = -1;
  locMapKs = -1;
}

PhongShadingDemoShaderProg::~PhongShadingDemoShaderProg() {}

void PhongShadingDemoShaderProg::GetUniformVariableLocation() {
  ShaderProg::GetUniformVariableLocation();
  locM = glGetUniformLocation(shaderProgId, "worldMatrix");
  locV = glGetUniformLocation(shaderProgId, "viewMatrix");
  locNM = glGetUniformLocation(shaderProgId, "normalMatrix");
  locCameraPos = glGetUniformLocation(shaderProgId, "cameraPos");
  locKa = glGetUniformLocation(shaderProgId, "Ka");
  locKd = glGetUniformLocation(shaderProgId, "Kd");
  locKs = glGetUniformLocation(shaderProgId, "Ks");
  locNs = glGetUniformLocation(shaderProgId, "Ns");
  locAmbientLight = glGetUniformLocation(shaderProgId, "ambientLight");
  locIsBlingPhong = glGetUniformLocation(shaderProgId, "isBlingPhong");
  locOnAmbientLight = glGetUniformLocation(shaderProgId, "onAmbientLight");
  locOnDiffuseLight = glGetUniformLocation(shaderProgId, "onDiffuseLight");
  locOnSpecularLight = glGetUniformLocation(shaderProgId, "onSpecularLight");
  locMapKd = glGetUniformLocation(shaderProgId, "mapKd");
  locMapKs = glGetUniformLocation(shaderProgId, "mapKs");
}

// ------------------------------------------------------------------------------------------------

SkyboxShaderProg::SkyboxShaderProg() { locMapKd = -1; }

SkyboxShaderProg::~SkyboxShaderProg() {}

void SkyboxShaderProg::GetUniformVariableLocation() {
  ShaderProg::GetUniformVariableLocation();
  locMapKd = glGetUniformLocation(shaderProgId, "mapKd");
}