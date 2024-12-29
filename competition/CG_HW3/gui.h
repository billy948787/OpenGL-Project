#ifndef GUI_H
#define GUI_H

#include "headers.h"
#include "light.h"
#include "trianglemesh.h"

struct GUIState {
    bool& isBlingPhong;
    bool& showDirLightArrow;
    bool& onPointLight;
    bool& onSpotLight;
    bool& onDirLight;
    bool& onAmbientLight;
    bool& onDiffuseLight;
    bool& onSpecularLight;
    float& dirLightArrowScale;

    float& curObjRotationX;
    float& curObjRotationY;
    float& skyboxRotation;

    GUIState(bool& isBlingPhong, bool& showDirLightArrow, bool& onPointLight,
        bool& onSpotLight, bool& onDirLight, bool& onAmbientLight,
        bool& onDiffuseLight, bool& onSpecularLight,
        float& dirLightArrowScale, float& curObjRotationX, float& curObjRotationY,
        float& skyboxRotation)
        : isBlingPhong(isBlingPhong),
        showDirLightArrow(showDirLightArrow),
        onPointLight(onPointLight),
        onSpotLight(onSpotLight),
        onDirLight(onDirLight),
        onAmbientLight(onAmbientLight),
        onDiffuseLight(onDiffuseLight),
        onSpecularLight(onSpecularLight),
        dirLightArrowScale(dirLightArrowScale),
        curObjRotationX(curObjRotationX),
        curObjRotationY(curObjRotationY),
        skyboxRotation(skyboxRotation) {};
};
class GUI {
public:
    GUI(GLFWwindow* window);
    ~GUI();

    void render(DirectionalLight*,
        void (*LoadObjects)(const std::string& filePath),
        void (*CreateSkybox)(std::string skyBoxFilePath),
        std::vector<std::string> objFilePaths,
        std::vector<std::string> skyboxFilePath, GUIState& guiState);

private:
};

#endif  // GUI_H