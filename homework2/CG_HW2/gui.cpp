#include "gui.h"
static int item_current_idx = 0;
static float lightMoveSpeed = 0.2f;

GUI::GUI(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  // 初始化 ImGui 的 GLFW 和 OpenGL3 後端
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

GUI::~GUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GUI::render(DirectionalLight* dirLight,
                 void (*LoadObjects)(const std::string& filePath),
                 std::vector<std::string> objFilePaths, bool& isBlingPhong,
                 bool& showDirLightArrow,  bool& onPointLight, bool& onSpotLight, bool& onDirLight,
                 bool& onAmbientLight, bool& onDiffuseLight,
                 bool& onSpecularLight,float& dirLightArrowScale) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // 創建一個選單
  ImGui::Begin("Obj Selector");
  if (ImGui::BeginCombo(
          "Objs",
          splitString(objFilePaths[item_current_idx], '/').back().c_str())) {
    for (int n = 0; n < objFilePaths.size(); n++) {
      bool is_selected = (item_current_idx == n);
      if (ImGui::Selectable(objFilePaths[n].c_str(), is_selected)) {
        item_current_idx = n;
        // 當選取物品時執行的操作
        LoadObjects(objFilePaths[n]);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::End();

  // 創建一個控制方向光方向的窗口
  ImGui::Begin("Directional Light");
  if (ImGui::ArrowButton("##upleft", ImGuiDir_Left)) {
    dirLight->MoveUpLeft(lightMoveSpeed);
  }
  ImGui::SameLine();
  if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
    dirLight->MoveUp(lightMoveSpeed);
  }
  ImGui::SameLine();
  if (ImGui::ArrowButton("##upright", ImGuiDir_Right)) {
    dirLight->MoveUpRight(lightMoveSpeed);
  }

  if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
    dirLight->MoveLeft(lightMoveSpeed);
  }
  ImGui::SameLine();
  if (ImGui::Button(" ")) {
    dirLight->ResetDirection(glm::vec3(0.0f, 0.0f, -1.0f));
  }
  ImGui::SameLine();
  if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
    dirLight->MoveRight(lightMoveSpeed);
  }

  if (ImGui::ArrowButton("##downleft", ImGuiDir_Left)) {
    dirLight->MoveDownLeft(lightMoveSpeed);
  }
  ImGui::SameLine();
  if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
    dirLight->MoveDown(lightMoveSpeed);
  }
  ImGui::SameLine();
  if (ImGui::ArrowButton("##downright", ImGuiDir_Right)) {
    dirLight->MoveDownRight(lightMoveSpeed);
  }

  ImGui::SliderFloat("Scale", &dirLightArrowScale, 0.1f, 10.0f);
  ImGui::End();

  ImGui::Begin("Settings");
  ImGui::Checkbox("Use Bling-Phong Shading", &isBlingPhong);
  ImGui::Checkbox("Show DirLight Arrow", &showDirLightArrow);
  ImGui::Checkbox("Enable Point Light", &onPointLight);
  ImGui::Checkbox("Enable Spot Light", &onSpotLight);
  ImGui::Checkbox("Enable Directional Light", &onDirLight);
  ImGui::Checkbox("Enable Ambient Light", &onAmbientLight);
  ImGui::Checkbox("Enable Diffuse Light", &onDiffuseLight);
  ImGui::Checkbox("Enable Specular Light", &onSpecularLight);
  ImGui::End();

  // 渲染 ImGui
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}