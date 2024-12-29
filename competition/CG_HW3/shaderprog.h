#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "headers.h"

// 光源結構體
struct DirectionalLightLocation {
  GLint direction;
  GLint radiance;
};

struct PointLightLocation {
  GLint position;
  GLint intensity;
  GLint constant;
  GLint linear;
  GLint quadratic;
  GLint decayStart;
};

struct SpotLightLocation {
  GLint position;
  GLint direction;
  GLint intensity;
  GLint cosCutoffStart;
  GLint cosCutoffEnd;
  GLint constant;
  GLint linear;
  GLint quadratic;
  GLint decayStart;
};

// 最大光源數量
#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8
#define MAX_AREA_LIGHTS 4  // 新增最大區域光源數量

// 新增 AreaLightLocation 結構體
struct AreaLightLocation {
  GLint position;
  GLint intensity;
  GLint direction;
  GLint width;
  GLint height;
  GLint samples;
  GLint constant;
  GLint linear;
  GLint quadratic;
  GLint decayStart;
};

// ShaderProg 宣告
class ShaderProg {
 public:
  // ShaderProg Public Methods.
  ShaderProg();
  virtual ~ShaderProg();

  bool LoadFromFiles(const std::string vsFilePath,
                     const std::string fsFilePath);
  void Bind() { glUseProgram(shaderProgId); };
  void UnBind() { glUseProgram(0); };

  GLint GetLocMVP() const { return locMVP; }

  // ShaderProg Protected Methods.
  virtual void GetUniformVariableLocation();

  // ShaderProg Protected Data.
  GLuint shaderProgId;

  // 光源的 uniform 位置
  GLint locNumDirLights;
  DirectionalLightLocation dirLightLocations[MAX_DIR_LIGHTS];

  GLint locNumPointLights;
  PointLightLocation pointLightLocations[MAX_POINT_LIGHTS];

  GLint locNumSpotLights;
  SpotLightLocation spotLightLocations[MAX_SPOT_LIGHTS];

  // 新增區域光的 uniform 位置
  GLint locNumAreaLights;
  AreaLightLocation areaLightLocations[MAX_AREA_LIGHTS];

 private:
  // ShaderProg Private Methods.
  GLuint AddShader(const std::string &sourceText, GLenum shaderType);
  static bool LoadShaderTextFromFile(const std::string filePath,
                                     std::string &sourceText);

  // ShaderProg Private Data.
  GLint locMVP;
};

// ------------------------------------------------------------------------------------------------

// FillColorShaderProg 宣告.
class FillColorShaderProg : public ShaderProg {
 public:
  // FillColorShaderProg Public Methods.
  FillColorShaderProg();
  virtual ~FillColorShaderProg();

  GLint GetLocFillColor() const { return locFillColor; }

 protected:
  // FillColorShaderProg Protected Methods.
  void GetUniformVariableLocation() override;

 private:
  // FillColorShaderProg Private Data.
  GLint locFillColor;
};

// ------------------------------------------------------------------------------------------------

// PhongShadingDemoShaderProg 宣告.
class PhongShadingDemoShaderProg : public ShaderProg {
 public:
  // PhongShadingDemoShaderProg Public Methods.
  PhongShadingDemoShaderProg();
  virtual ~PhongShadingDemoShaderProg();

  // Getter 方法
  GLint GetLocM() const { return locM; }
  GLint GetLocV() const { return locV; }
  GLint GetLocNM() const { return locNM; }
  GLint GetLocCameraPos() const { return locCameraPos; }
  GLint GetLocKa() const { return locKa; }
  GLint GetLocKd() const { return locKd; }
  GLint GetLocMapKd() const { return locMapKd; }
  GLint GetLocMapKs() const { return locMapKs; }
  GLint GetLocKs() const { return locKs; }
  GLint GetLocNs() const { return locNs; }
  GLint GetLocAmbientLight() const { return locAmbientLight; }
  GLint GetLocIsBlingPhong() const { return locIsBlingPhong; }
  GLint GetLocOnAmbientLight() const { return locOnAmbientLight; }
  GLint GetLocOnDiffuseLight() const { return locOnDiffuseLight; }
  GLint GetLocOnSpecularLight() const { return locOnSpecularLight; }

 protected:
  // PhongShadingDemoShaderProg Protected Methods.
  void GetUniformVariableLocation() override;

 private:
  // PhongShadingDemoShaderProg Private Data.
  // Transformation matrix.
  GLint locM;
  GLint locV;
  GLint locNM;
  GLint locCameraPos;
  // Material properties.
  GLint locKa;
  GLint locKd;
  GLint locKs;
  GLint locNs;
  // Light data.
  GLint locAmbientLight;
  GLint locIsBlingPhong;
  GLint locOnAmbientLight;
  GLint locOnDiffuseLight;
  GLint locOnSpecularLight;
  // Texture data.
  GLint locMapKd;
  GLint locMapKs;
};

// ------------------------------------------------------------------------------------------------

// SkyboxShaderProg 宣告.
class SkyboxShaderProg : public ShaderProg {
 public:
  // SkyboxShaderProg Public Methods.
  SkyboxShaderProg();
  virtual ~SkyboxShaderProg();

  GLint GetLocMapKd() const { return locMapKd; }

 protected:
  // SkyboxShaderProg Protected Methods.
  void GetUniformVariableLocation() override;

 private:
  // SkyboxShaderProg Private Data.
  GLint locMapKd;
};

#endif