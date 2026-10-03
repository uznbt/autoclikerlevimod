/**
 * ImGuiOverlay.cpp
 * 
 * Hook ke EGL SwapBuffers Minecraft Android agar ImGui bisa menggambar
 * menu Autoclicker di atas layar game.
 * 
 * Teknik: PLT/GOT hook via preloader-android SDK (dobby/frida-gum style)
 */

#include "mod/ImGuiOverlay.h"
#include "mod/Config.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/log.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_android.h>

#include <pl/hook/Hook.hpp>  // preloader-android hook API
#include <pl/Mod.hpp>

#include <chrono>
#include <string>
#include <vector>

#define LOG_TAG "AutoclickerMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace clange_me {

// ─── State ───────────────────────────────────────────────────────────────────

static bool  s_imguiInit       = false;
static bool  s_menuOpen        = false;   // dibuka/tutup tombol Levi
static EGLDisplay s_display    = EGL_NO_DISPLAY;
static EGLSurface s_surface    = EGL_NO_SURFACE;

// Pengaturan (dibaca dari Config saat mod enable, ditulis balik saat mod disable)
static bool  s_enabled         = true;
static int   s_intervalMs      = 100;
static int   s_numTargets      = 1;
static bool  s_autoHold        = false;

// Posisi setiap titik klik (bisa di-drag)
static float s_posX[5] = {150.f, 250.f, 350.f, 450.f, 550.f};
static float s_posY[5] = {300.f, 300.f, 300.f, 300.f, 300.f};

// Timer autoclicker
static auto  s_lastClick = std::chrono::steady_clock::now();

// ─── Forward declarations ─────────────────────────────────────────────────────
static void  InitImGui(EGLDisplay dpy, EGLSurface sfc);
static void  RenderOverlay();
static void  TickAutoclicker();

// ─── EGL SwapBuffers hook ─────────────────────────────────────────────────────

using PFN_eglSwapBuffers = EGLBoolean(*)(EGLDisplay, EGLSurface);
static PFN_eglSwapBuffers s_origEglSwapBuffers = nullptr;

static EGLBoolean Hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!s_imguiInit) {
        InitImGui(dpy, surface);
    }

    // Beri tahu ImGui backend resolusi layar saat ini
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);

    // Mulai frame ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (s_enabled) {
        TickAutoclicker();
    }

    if (s_menuOpen) {
        RenderOverlay();
    }

    // Render ke layar
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return s_origEglSwapBuffers(dpy, surface);
}

// ─── Touch input hook ─────────────────────────────────────────────────────────

using PFN_AInputEvent_getType = int32_t(*)(const AInputEvent*);
// Kita intercept input sebelum Minecraft memproses sentuhan
// agar sentuhan pada menu ImGui tidak tembus ke game
using PFN_processInput = bool(*)(void*, AInputEvent*);
static PFN_processInput s_origProcessInput = nullptr;

static bool Hook_processInput(void* self, AInputEvent* event) {
    if (s_menuOpen) {
        // Teruskan ke ImGui backend Android
        ImGui_ImplAndroid_HandleInputEvent(event);
        ImGuiIO& io = ImGui::GetIO();
        // Kalau ImGui mengklaim event ini (misal disentuh di atas menu), jangan teruskan ke game
        if (io.WantCaptureMouse) {
            return true;
        }
    }
    return s_origProcessInput(self, event);
}

// ─── Inisialisasi ImGui ───────────────────────────────────────────────────────

static void InitImGui(EGLDisplay dpy, EGLSurface sfc) {
    s_display = dpy;
    s_surface = sfc;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // jangan simpan .ini

    // Tema gelap agar pas di atas Minecraft
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 10.f;
    style.FrameRounding    = 6.f;
    style.GrabRounding     = 6.f;
    style.Alpha            = 0.92f;
    style.Colors[ImGuiCol_WindowBg]     = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
    style.Colors[ImGuiCol_TitleBgActive]= ImVec4(0.10f, 0.55f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]   = ImVec4(0.20f, 0.80f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]    = ImVec4(0.20f, 0.80f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Button]       = ImVec4(0.10f, 0.45f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]= ImVec4(0.15f, 0.65f, 0.45f, 1.00f);

    // Scaling font agar mudah dibaca di layar HP
    io.FontGlobalScale = 2.0f;

    ImGui_ImplAndroid_Init(nullptr); // nullptr = tanpa ANativeWindow khusus
    ImGui_ImplOpenGL3_Init("#version 300 es");

    s_imguiInit = true;
    LOGI("ImGui overlay initialized");
}

// ─── Render overlay (menu + target buttons) ───────────────────────────────────

static void RenderOverlay() {
    // ── Jendela Pengaturan Utama ──────────────────────────────────────────────
    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(30, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Autoclicker", &s_menuOpen,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Aktifkan", &s_enabled);
    ImGui::Separator();

    ImGui::Text("Kecepatan klik:");
    ImGui::SliderInt("##interval", &s_intervalMs, 1, 1000);
    ImGui::SameLine();
    ImGui::Text("%d ms", s_intervalMs);

    ImGui::Text("Jumlah titik klik:");
    ImGui::SliderInt("##targets", &s_numTargets, 1, 5);

    ImGui::Checkbox("Mode Auto-Hold (tahan)", &s_autoHold);
    ImGui::Separator();
    ImGui::TextDisabled("Geser tombol titik di layar untuk atur posisi");

    ImGui::End();

    // ── Tombol Titik Klik (satu per target, bisa di-drag) ────────────────────
    for (int i = 0; i < s_numTargets; i++) {
        ImGui::SetNextWindowPos(ImVec2(s_posX[i], s_posY[i]), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::SetNextWindowBgAlpha(0.75f);

        std::string wname = "##t" + std::to_string(i);
        ImGui::Begin(wname.c_str(), nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings);

        std::string label = " [" + std::to_string(i + 1) + "] Geser ";
        ImGui::Button(label.c_str());

        // Simpan posisi baru setelah di-drag
        ImVec2 p = ImGui::GetWindowPos();
        s_posX[i] = p.x;
        s_posY[i] = p.y;

        ImGui::End();
    }
}

// ─── Logika Autoclicker (dipanggil tiap frame) ────────────────────────────────

static void TickAutoclicker() {
    if (!s_enabled) return;

    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastClick).count();

    if (elapsed < s_intervalMs) return;
    s_lastClick = now;

    for (int i = 0; i < s_numTargets; i++) {
        float x = s_posX[i];
        float y = s_posY[i];

        if (s_autoHold) {
            // Kirim ACTION_DOWN (tahan) ke koordinat titik klik
            // pl::input::simulateTouch(x, y, pl::input::TouchAction::Down);
            LOGI("AutoHold DOWN at (%.0f, %.0f)", x, y);
        } else {
            // Kirim ACTION_DOWN + ACTION_UP (klik cepat)
            // pl::input::simulateTouch(x, y, pl::input::TouchAction::Down);
            // pl::input::simulateTouch(x, y, pl::input::TouchAction::Up);
            LOGI("AutoClick at (%.0f, %.0f)", x, y);
        }
    }
}

// ─── Setup & teardown (dipanggil dari MyMod) ─────────────────────────────────

void ImGuiOverlay::setup(const ModConfig& cfg) {
    s_enabled    = cfg.enabled;
    s_intervalMs = cfg.clickIntervalMs;
    s_numTargets = cfg.numberOfTargets;
    s_autoHold   = cfg.modeAutoHold;

    // Hook eglSwapBuffers
    PL_HOOK_FUNC(eglSwapBuffers, Hook_eglSwapBuffers, s_origEglSwapBuffers);

    LOGI("ImGuiOverlay hooks installed");
}

void ImGuiOverlay::teardown() {
    if (s_imguiInit) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();
        s_imguiInit = false;
    }
    // Unhook
    PL_UNHOOK_FUNC(eglSwapBuffers);
    LOGI("ImGuiOverlay hooks removed");
}

void ImGuiOverlay::toggleMenu() {
    s_menuOpen = !s_menuOpen;
}

ModConfig ImGuiOverlay::currentConfig() {
    ModConfig cfg;
    cfg.enabled        = s_enabled;
    cfg.clickIntervalMs= s_intervalMs;
    cfg.numberOfTargets= s_numTargets;
    cfg.modeAutoHold   = s_autoHold;
    return cfg;
}

} // namespace clange_me
