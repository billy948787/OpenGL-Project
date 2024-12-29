#include "gui.h"
static int item_current_idx_obj = 0;
static int item_current_idx_skybox = 0;
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
    void (*CreateSkybox)(std::string skyBoxFilePath),
    std::vector<std::string> objFilePaths,
    std::vector<std::string> skyBoxFilePath, GUIState& guiState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 創建一個選單
    ImGui::Begin("Selector");
    if (ImGui::BeginCombo(
        "Objs", Utils::splitString(objFilePaths[item_current_idx_obj], '/')
        .back()
        .c_str())) {
        for (int n = 0; n < objFilePaths.size(); n++) {
            bool is_selected = (item_current_idx_obj == n);
            if (ImGui::Selectable(objFilePaths[n].c_str(), is_selected)) {
                item_current_idx_obj = n;
                // 當選取物品時執行的操作
                LoadObjects(objFilePaths[n]);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo(
        "Skybox",
        Utils::splitString(skyBoxFilePath[item_current_idx_skybox], '/')
        .back()
        .c_str())) {
        for (int n = 0; n < skyBoxFilePath.size(); n++) {
            bool is_selected = (item_current_idx_skybox == n);
            if (ImGui::Selectable(skyBoxFilePath[n].c_str(), is_selected)) {
                item_current_idx_skybox = n;
                // 當選取物品時執行的操作
                CreateSkybox(skyBoxFilePath[n]);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
    ImGui::End();


    ImGui::Begin("Model Rotation");

    // 向上旋轉
    if (ImGui::ArrowButton("##rotate_up", ImGuiDir_Up)) {
        guiState.curObjRotationX -= 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.curObjRotationX -= 1.0f;
    }

    // 向左旋轉
    if (ImGui::ArrowButton("##rotate_left", ImGuiDir_Left)) {
        guiState.curObjRotationY -= 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.curObjRotationY -= 1.0f;
    }
    ImGui::SameLine();

    // 向下旋轉
    if (ImGui::ArrowButton("##rotate_down", ImGuiDir_Down)) {
        guiState.curObjRotationX += 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.curObjRotationX += 1.0f;
    }

    ImGui::SameLine();
    // 向右旋轉
    if (ImGui::ArrowButton("##rotate_right", ImGuiDir_Right)) {
        guiState.curObjRotationY += 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.curObjRotationY += 1.0f;
    }
    // 重置旋轉
    if (ImGui::Button("Reset")) {
        guiState.curObjRotationX = 0.0f;
        guiState.curObjRotationY = 0.0f;
    }

    ImGui::End();

    // 創建一個控制天空盒旋轉的窗口
    ImGui::Begin("Skybox Rotation");

    // 向左旋轉
    if (ImGui::ArrowButton("##skybox_rotate_left", ImGuiDir_Left)) {
        guiState.skyboxRotation -= 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.skyboxRotation -= 1.0f;
    }
    ImGui::SameLine();

    // 向右旋轉
    if (ImGui::ArrowButton("##skybox_rotate_right", ImGuiDir_Right)) {
        guiState.skyboxRotation += 1.0f;
    }
    if (ImGui::IsItemActive()) {
        guiState.skyboxRotation += 1.0f;
    }

    // 重置旋轉
    if (ImGui::Button("Reset")) {
        guiState.skyboxRotation = 0.0f;
    }

    ImGui::End();

    ImGui::Begin("Settings");
    ImGui::Checkbox("Use Bling-Phong Shading", &guiState.isBlingPhong);
    ImGui::Checkbox("Show DirLight Arrow", &guiState.showDirLightArrow);
    ImGui::Checkbox("Enable Point Light", &guiState.onPointLight);
    ImGui::Checkbox("Enable Spot Light", &guiState.onSpotLight);
    ImGui::Checkbox("Enable Directional Light", &guiState.onDirLight);
    ImGui::Checkbox("Enable Ambient Light", &guiState.onAmbientLight);
    ImGui::Checkbox("Enable Diffuse Light", &guiState.onDiffuseLight);
    ImGui::Checkbox("Enable Specular Light", &guiState.onSpecularLight);
    ImGui::End();

    // 渲染 ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}