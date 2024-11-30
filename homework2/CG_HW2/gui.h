#ifndef GUI_H
#define GUI_H

#include "headers.h"
#include "light.h"
#include "trianglemesh.h"

class GUI {
 public:
  GUI(GLFWwindow* window);
  ~GUI();

  void render(DirectionalLight*,
              void (*LoadObjects)(const std::string& filePath),
              std::vector<std::string> objFilePaths, bool& isBlingPhong,
              bool& showDirLightArrow,bool& onPointLight, bool& onSpotLight, bool& onDirLight,
              bool& onAmbientLight, bool& onDiffuseLight, bool& onSpecularLight

  );

 private:
};

#endif  // GUI_H