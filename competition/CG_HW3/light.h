#ifndef LIGHT_H
#define LIGHT_H

#include "headers.h"
#include "shaderprog.h"

class Light {
 public:
  // 建構函式
  Light()
      : intensity(glm::vec3(1.0f)),
        constant(1.0f),
        linear(0.09f),
        quadratic(0.032f) {}

  Light(const glm::vec3& I)
      : intensity(I), constant(1.0f), linear(0.09f), quadratic(0.032f) {}

  // Getter 方法
  glm::vec3 GetIntensity() const { return intensity; }
  float GetConstant() const { return constant; }
  float GetLinear() const { return linear; }
  float GetQuadratic() const { return quadratic; }
  float GetDecayStart() const { return decayStart; }

  // Setter 方法
  void SetIntensity(const glm::vec3& I) { intensity = I; }
  void SetConstant(float c) { constant = c; }
  void SetLinear(float l) { linear = l; }
  void SetQuadratic(float q) { quadratic = q; }
  void SetDecayStart(float decayStart) { this->decayStart = decayStart; }

 protected:
  glm::vec3 intensity;
  float decayStart;
  float constant;
  float linear;
  float quadratic;
};

// PointLight 類別
class PointLight : public Light {
 public:
  // 建構函式
  PointLight() : Light(), position(glm::vec3(1.5f)) {}

  PointLight(const glm::vec3& p, const glm::vec3& I) : Light(I), position(p) {}

  // Getter 方法
  glm::vec3 GetPosition() const { return position; }

  // Setter 方法
  void SetPosition(const glm::vec3& p) { position = p; }

  // 移動方法
  void MoveLeft(float moveSpeed) {
    position += moveSpeed * glm::vec3(-0.1f, 0.0f, 0.0f);
  }
  void MoveRight(float moveSpeed) {
    position += moveSpeed * glm::vec3(0.1f, 0.0f, 0.0f);
  }
  void MoveUp(float moveSpeed) {
    position += moveSpeed * glm::vec3(0.0f, 0.1f, 0.0f);
  }
  void MoveDown(float moveSpeed) {
    position += moveSpeed * glm::vec3(0.0f, -0.1f, 0.0f);
  }

 protected:
  glm::vec3 position;
};

// SpotLight 類別
class SpotLight : public Light {
 public:
  // 建構函式
  SpotLight()
      : Light(),
        position(glm::vec3(1.5f)),
        direction(glm::vec3(0.0f, -1.0f, 0.0f)),
        cosCutoffEnd(glm::cos(glm::radians(15.0f))),
        cosCutoffStart(glm::cos(glm::radians(10.0f))) {}

  SpotLight(const glm::vec3& p, const glm::vec3& I, const glm::vec3& D,
            float cutoffDegStart, float cutoffDegEnd)
      : Light(I),
        position(p),
        direction(glm::normalize(D)),
        cosCutoffStart(glm::cos(glm::radians(cutoffDegStart))),
        cosCutoffEnd(glm::cos(glm::radians(cutoffDegEnd))) {}

  // Getter 方法
  glm::vec3 GetPosition() const { return position; }
  glm::vec3 GetDirection() const { return direction; }
  float GetCosCutoffStart() const { return cosCutoffStart; }
  float GetCosCutoffEnd() const { return cosCutoffEnd; }

  // Setter 方法
  void SetPosition(const glm::vec3& P) { position = P; }
  void SetDirection(const glm::vec3& D) { direction = glm::normalize(D); }
  void SetCutoffStart(float cutoffDeg) {
    cosCutoffStart = glm::cos(glm::radians(cutoffDeg));
  }
  void SetCutoffEnd(float cutoffDeg) {
    cosCutoffEnd = glm::cos(glm::radians(cutoffDeg));
  }

 protected:
  glm::vec3 position;
  glm::vec3 direction;
  float cosCutoffEnd;
  float cosCutoffStart;
};

// DirectionalLight 類別
class DirectionalLight : public Light {
 public:
  // 建構函式
  DirectionalLight() : Light(), direction(glm::vec3(-0.2f, -1.0f, -0.3f)) {}

  DirectionalLight(const glm::vec3& dir, const glm::vec3& I)
      : Light(I), direction(glm::normalize(dir)) {}

  // Getter 方法
  glm::vec3 GetDirection() const { return direction; }

  // Setter 方法
  void SetDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }

  // 移動方法
  void MoveLeft(float moveSpeed) {
    glm::mat4 rot =
        glm::rotate(glm::mat4(1.0f), -moveSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    direction = glm::vec3(rot * glm::vec4(direction, 0.0f));
  }
  void MoveRight(float moveSpeed) {
    glm::mat4 rot =
        glm::rotate(glm::mat4(1.0f), moveSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    direction = glm::vec3(rot * glm::vec4(direction, 0.0f));
  }
  void MoveUp(float moveSpeed) {
    glm::vec3 right =
        glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), -moveSpeed, right);
    direction = glm::vec3(rot * glm::vec4(direction, 0.0f));
  }
  void MoveDown(float moveSpeed) {
    glm::vec3 right =
        glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), moveSpeed, right);
    direction = glm::vec3(rot * glm::vec4(direction, 0.0f));
  }

 protected:
  glm::vec3 direction;
};

// AreaLight 類別
class AreaLight : public Light {
 public:
  // 建構函式
  AreaLight()
      : Light(),
        position(glm::vec3(0.0f, 2.0f, 0.0f)),
        direction(glm::vec3(0.0f, -1.0f, 0.0f)),
        width(0.5f),
        height(0.5f),
        samples(10) {}

  AreaLight(const glm::vec3& p, const glm::vec3& I, const glm::vec3& d, float w,
            float h, int s)
      : Light(I),
        position(p),
        direction(glm::normalize(d)),
        width(w),
        height(h),
        samples(s) {}

  // Getter 方法
  glm::vec3 GetPosition() const { return position; }
  glm::vec3 GetDirection() const { return direction; }
  float GetWidth() const { return width; }
  float GetHeight() const { return height; }
  int GetSamples() const { return samples; }

  // Setter 方法
  void SetPosition(const glm::vec3& p) { position = p; }
  void SetDirection(const glm::vec3& d) { direction = glm::normalize(d); }
  void SetWidth(float w) { width = w; }
  void SetHeight(float h) { height = h; }
  void SetSamples(int s) { samples = s; }

 protected:
  glm::vec3 position;
  glm::vec3 direction;
  float width;
  float height;
  int samples;
};

#endif