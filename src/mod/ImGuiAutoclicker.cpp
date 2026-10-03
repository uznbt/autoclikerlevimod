#include <string>
#include <vector>

// Dummy ImGui namespace to make the boilerplate compile/show syntax
namespace ImGui {
    struct ImVec2 { float x, y; ImVec2(float _x, float _y) : x(_x), y(_y) {} };
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    bool Checkbox(const char* label, bool* v);
    bool SliderInt(const char* label, int* v, int v_min, int v_max);
    void SetNextWindowPos(const ImVec2& pos, int cond);
    bool Button(const char* label);
    ImVec2 GetWindowPos();
    const int ImGuiCond_FirstUseEver = 1;
    const int ImGuiWindowFlags_NoTitleBar = 1;
    const int ImGuiWindowFlags_AlwaysAutoResize = 2;
}

// Variables for Autoclicker Settings
bool isModMenuOpen = true;
int clickIntervalMs = 100;
int numberOfTargets = 1;
bool modeAutoHold = false;
bool isAutoclickerActive = false;

// Target coordinates (X, Y)
std::vector<float> targetPosX = {100.0f, 200.0f, 300.0f, 400.0f, 500.0f};
std::vector<float> targetPosY = {100.0f, 200.0f, 300.0f, 400.0f, 500.0f};

// This function represents the ImGui UI Rendering logic
// It is injected into the Android graphics render loop (e.g., EGL SwapBuffers)
void RenderAutoclickerUI() {
    if (isModMenuOpen) {
        // Main Settings Menu
        ImGui::Begin("Pengaturan Autoclicker", &isModMenuOpen);
        
        ImGui::Checkbox("Aktifkan Autoclicker", &isAutoclickerActive);
        ImGui::Checkbox("Mode Auto-Hold (Tahan)", &modeAutoHold);
        ImGui::SliderInt("Kecepatan (ms)", &clickIntervalMs, 1, 1000);
        ImGui::SliderInt("Jumlah Titik Klik", &numberOfTargets, 1, 5);
        
        ImGui::End();
    }

    // Render Draggable Target Buttons Overlay
    for (int i = 0; i < numberOfTargets; i++) {
        // Set the window position (draggable)
        ImGui::SetNextWindowPos(ImGui::ImVec2(targetPosX[i], targetPosY[i]), ImGui::ImGuiCond_FirstUseEver);
        
        std::string windowName = "Target " + std::to_string(i + 1);
        ImGui::Begin(windowName.c_str(), nullptr, ImGui::ImGuiWindowFlags_NoTitleBar | ImGui::ImGuiWindowFlags_AlwaysAutoResize);
        
        // Target Button in UI
        std::string buttonLabel = "[Titik " + std::to_string(i + 1) + "] (Tahan & Geser)";
        ImGui::Button(buttonLabel.c_str());
        
        // Save the new position if the user drags it!
        ImGui::ImVec2 windowPos = ImGui::GetWindowPos();
        targetPosX[i] = windowPos.x;
        targetPosY[i] = windowPos.y;
        
        ImGui::End();
    }
}
