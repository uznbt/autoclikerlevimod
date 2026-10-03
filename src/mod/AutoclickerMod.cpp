/**
 * AutoclickerMod.cpp
 *
 * Cara kerja:
 *  1. Daftarkan satu tombol "AC" di HUD Levi Launcher (bisa digeser oleh user).
 *  2. Tombol itu berperilaku Toggle (tekan = ON, tekan lagi = OFF).
 *  3. Saat ON, thread background mengirim MotionEvent (touch DOWN + UP) via JNI
 *     ke Activity Minecraft pada koordinat tengah tombol — tanpa ubah setting MC.
 *  4. Mode Auto-Hold: hanya kirim ACTION_DOWN terus (tidak ada UP) → blok/mining.
 *  5. Mode Auto-Click: kirim DOWN lalu UP setiap intervalMs milidetik → spam pukulan.
 */

#include "mod/AutoclickerMod.h"

#include <pl/ModMenu.hpp>
#include <pl/Input.hpp>
#include <pl/Mod.hpp>
#include <android/log.h>

#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <string>

#define LOG_TAG "AutoclickerMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace clange_me {

// ─── State global ──────────────────────────────────────────────────────────
static std::atomic<bool> s_clicking  {false};  // thread berjalan?
static std::atomic<bool> s_pauseClicking{false}; // smart pause saat jari asli nempel
static std::atomic<bool> s_autoHold  {false};  // mode hold atau spam?
static std::atomic<int>  s_intervalMs{100};    // ms antar klik

// Koordinat pusat tombol di layar (diperbarui setiap kali tombol dideteksi)
// Levi Launcher menempatkan tombol; kita baca posisinya dari touch event ON saat
// pertama kali user menyentuh tombol toggle.
static std::atomic<float> s_tapX{-1.f};
static std::atomic<float> s_tapY{-1.f};
static std::atomic<float> s_lastTouchX{-1.f};
static std::atomic<float> s_lastTouchY{-1.f};

// JavaVM untuk JNI dari background thread
static JavaVM* s_jvm = nullptr;

// ID tombol
static std::string s_toggleButtonId;
static std::string s_modId;

// ─── JNI Helper: kirim MotionEvent ke Activity ─────────────────────────────
static void sendMotionEvent(JNIEnv* env, jobject instrumentation, int action, float x, float y, jlong downTime) {
    jclass scCls = env->FindClass("android/os/SystemClock");
    if (!scCls) { env->ExceptionClear(); return; }
    jmethodID uptime = env->GetStaticMethodID(scCls, "uptimeMillis", "()J");
    jlong eventTime = env->CallStaticLongMethod(scCls, uptime);

    jclass meCls = env->FindClass("android/view/MotionEvent");
    if (!meCls) { env->ExceptionClear(); return; }

    jmethodID obtain = env->GetStaticMethodID(
        meCls, "obtain",
        "(JJIFFI)Landroid/view/MotionEvent;");
    if (!obtain) { env->ExceptionClear(); return; }

    jobject event = env->CallStaticObjectMethod(
        meCls, obtain,
        downTime, eventTime,
        (jint)action,
        (jfloat)x, (jfloat)y,
        (jint)0);        // metaState

    if (!event) { env->ExceptionClear(); return; }

    jclass instCls = env->GetObjectClass(instrumentation);
    jmethodID sendPointerSync = env->GetMethodID(instCls, "sendPointerSync", "(Landroid/view/MotionEvent;)V");
    if (sendPointerSync) {
        env->CallVoidMethod(instrumentation, sendPointerSync, event);
    } else {
        env->ExceptionClear();
        LOGE("sendPointerSync not found");
    }
    
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGE("sendPointerSync failed (SecurityException?)");
    }

    env->DeleteLocalRef(event);
    env->DeleteLocalRef(instCls);
    env->DeleteLocalRef(meCls);
    env->DeleteLocalRef(scCls);
}

static jobject getInstrumentation(JNIEnv* env) {
    jclass atCls = env->FindClass("android/app/ActivityThread");
    if (!atCls) { env->ExceptionClear(); LOGE("ActivityThread not found"); return nullptr; }

    jmethodID curThread = env->GetStaticMethodID(atCls, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!curThread) { env->ExceptionClear(); LOGE("currentActivityThread not found"); return nullptr; }

    jobject at = env->CallStaticObjectMethod(atCls, curThread);
    if (!at) { env->ExceptionClear(); LOGE("ActivityThread instance is null"); return nullptr; }

    jmethodID getInst = env->GetMethodID(atCls, "getInstrumentation", "()Landroid/app/Instrumentation;");
    if (!getInst) { env->ExceptionClear(); LOGE("getInstrumentation not found"); return nullptr; }

    jobject inst = env->CallObjectMethod(at, getInst);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGE("getInstrumentation call failed");
        return nullptr;
    }
    
    env->DeleteLocalRef(at);
    env->DeleteLocalRef(atCls);
    return inst; // caller must DeleteLocalRef
}

// ─── Background thread: spam klik ──────────────────────────────────────────
static void* clickThreadFunc(void*) {
    if (!s_jvm) { LOGE("No JavaVM"); return nullptr; }

    JNIEnv* env = nullptr;
    JavaVMAttachArgs args{JNI_VERSION_1_6, "AutoclickerThread", nullptr};
    if (s_jvm->AttachCurrentThread(&env, &args) != JNI_OK) {
        LOGE("Failed to attach JNI thread");
        return nullptr;
    }

    // ACTION_DOWN = 0, ACTION_UP = 1
    static constexpr int ACTION_DOWN = 0;
    static constexpr int ACTION_UP   = 1;

    LOGI("Autoclicker thread started");

    jclass scCls = env->FindClass("android/os/SystemClock");
    jmethodID uptime = env->GetStaticMethodID(scCls, "uptimeMillis", "()J");
    jlong downTime = 0;
    bool wasHolding = false;

    // ACTION_MOVE = 2
    static constexpr int ACTION_MOVE = 2;

    while (s_clicking.load()) {
        if (s_pauseClicking.load()) {
            usleep(50 * 1000);
            continue;
        }

        float x = s_tapX.load();
        float y = s_tapY.load();

        if (x < 0.f || y < 0.f) {
            usleep(50 * 1000);
            continue;
        }

        jobject instrumentation = getInstrumentation(env);
        if (!instrumentation) {
            LOGE("Instrumentation is null, retry in 500ms");
            usleep(500 * 1000);
            continue;
        }

        bool isAutoHold = s_autoHold.load();

        if (isAutoHold) {
            if (!wasHolding) {
                downTime = env->CallStaticLongMethod(scCls, uptime);
                sendMotionEvent(env, instrumentation, ACTION_DOWN, x, y, downTime);
                wasHolding = true;
            } else {
                // Terus kirim ACTION_MOVE di posisi yang sama supaya Minecraft tahu kita masih menahan layar
                sendMotionEvent(env, instrumentation, ACTION_MOVE, x, y, downTime);
            }
            // Sleep kecil saja saat menahan, tidak perlu ikut intervalMs agar lebih responsif
            usleep(50 * 1000); 
        } else {
            if (wasHolding) {
                sendMotionEvent(env, instrumentation, ACTION_UP, x, y, downTime);
                wasHolding = false;
                usleep(50 * 1000);
            }
            downTime = env->CallStaticLongMethod(scCls, uptime);
            sendMotionEvent(env, instrumentation, ACTION_DOWN, x, y, downTime);
            usleep(30 * 1000);
            sendMotionEvent(env, instrumentation, ACTION_UP, x, y, downTime);
            usleep(s_intervalMs.load() * 1000);
        }

        env->DeleteLocalRef(instrumentation);
    }

    float x = s_tapX.load();
    float y = s_tapY.load();
    if (x >= 0.f && y >= 0.f) {
        jobject instrumentation = getInstrumentation(env);
        if (instrumentation) {
            // Pastikan dilepas saat thread berhenti
            sendMotionEvent(env, instrumentation, ACTION_UP, x, y, downTime);
            env->DeleteLocalRef(instrumentation);
        }
    }

    env->DeleteLocalRef(scCls);

    LOGI("Autoclicker thread stopped");
    s_jvm->DetachCurrentThread();
    return nullptr;
}

// ─── Start / Stop thread ───────────────────────────────────────────────────
static void startClicking() {
    if (s_clicking.load()) return;
    s_tapX.store(s_lastTouchX.load());
    s_tapY.store(s_lastTouchY.load());
    s_clicking.store(true);
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, clickThreadFunc, nullptr);
    pthread_attr_destroy(&attr);
    LOGI("Clicker started");
}

static void stopClicking() {
    s_clicking.store(false);
    LOGI("Clicker stopped");
}

// ─── Setup (dipanggil dari MyMod::enable) ─────────────────────────────────
void AutoclickerMod::setup(const ModConfig& cfg, const std::string& modId) {
    // Simpan JavaVM dari NativeMod
    s_jvm = pl::mod::NativeMod::current()->getJavaVM();

    s_autoHold.store(cfg.modeAutoHold);
    s_intervalMs.store(cfg.clickIntervalMs);
    s_modId = modId;

    // 1. Daftarkan Module di menu Levi (pengaturan)
    bool registered = pl::modmenu::ModuleBuilder("autoclicker_module", "AutoClicker")
        .modId(modId)
        .description("Tap area otomatis. Atur posisi tombol AC ke area yang ingin diklik.")
        .defaultEnabled(cfg.enabled)
        .config("intervalMs", "Kecepatan (ms)",
                pl::modmenu::ConfigType::SliderInt,
                std::to_string(cfg.clickIntervalMs), "30", "1000")
        .config("autoHold", "Mode Tahan (Auto-Hold)",
                pl::modmenu::ConfigType::Toggle,
                cfg.modeAutoHold ? "true" : "false")
        .onConfigChanged([](std::string_view, std::string_view key,
                             std::string_view value) {
            if (key == "intervalMs") {
                s_intervalMs.store(std::stoi(std::string(value)));
                LOGI("intervalMs = %d", s_intervalMs.load());
            } else if (key == "autoHold") {
                s_autoHold.store(value == "true");
                LOGI("autoHold = %s", s_autoHold.load() ? "true" : "false");
            }
        })
        .registerModule();
    LOGI("Module registered: %s", registered ? "OK" : "FAILED");

    // 2. Daftarkan tombol Toggle "AC" di HUD — user bisa geser ke posisi manapun
    s_toggleButtonId = "autoclicker_toggle_btn";
    bool okBtn = pl::modmenu::ButtonBuilder(s_toggleButtonId, "AutoClicker")
        .modId(modId)
        .moduleId("autoclicker_module")   // <-- wajib agar tombol muncul saat modul aktif
        .label("AC")
        .behavior(pl::modmenu::ButtonBehavior::Toggle)
        .defaultVisible(true)
        .stylePreset(pl::modmenu::ButtonStylePreset::Keycap)
        .onEvent([](std::string_view, pl::modmenu::ButtonEvent event, float value) {
            if (event == pl::modmenu::ButtonEvent::StateChanged) {
                if (value > 0.5f) {
                    startClicking();
                } else {
                    stopClicking();
                    // RESET TARGET POSISI!
                    // Supaya kalau user geser tombolnya, klik berikutnya 
                    // akan merekam posisi tombol yang baru.
                    s_tapX.store(-1.f);
                    s_tapY.store(-1.f);
                }
            }
        })
        .registerButton();
    LOGI("Toggle button registered: %s", okBtn ? "OK" : "FAILED");

    // 3. Touch callback: intercept event pada posisi tombol untuk tahu koordinatnya
    //    dan juga supaya klik palsu kita tidak ter-intercept lagi
    pl::input::registerTouchCallback([](const pl::input::TouchEvent& e) -> bool {
        if (e.action == 0) {
            s_lastTouchX.store(e.x);
            s_lastTouchY.store(e.y);
            
            // SMART PAUSE: Kalau jarak sentuhan jauh dari tombol target, 
            // berarti ini jari asli user (misal buat jalan/geser kamera). PAUSE CLICKER!
            if (s_clicking.load()) {
                float tx = s_tapX.load();
                float ty = s_tapY.load();
                if (tx >= 0.f && ty >= 0.f) {
                    float dx = e.x - tx;
                    float dy = e.y - ty;
                    // Jarak > 100 px = sentuhan asli
                    if (dx*dx + dy*dy > 10000.f) {
                        s_pauseClicking.store(true);
                    }
                }
            }
        } else if (e.action == 1) { // ACTION_UP
            s_pauseClicking.store(false);
        }
        return false;
    });

    // 4. Tombol Volume untuk mematikan darurat (Emergency Stop)
    pl::input::registerKeyCallback([](const pl::input::KeyEvent& e) -> bool {
        // 24 = Vol Up, 25 = Vol Down
        if (e.isKeyDown && (e.keyCode == 24 || e.keyCode == 25)) {
            if (s_clicking.load()) {
                stopClicking();
                s_tapX.store(-1.f);
                s_tapY.store(-1.f);
                return true; // intercept event supaya volume media tidak berubah
            }
        }
        return false;
    });

    LOGI("AutoclickerMod setup complete");
}

// ─── Teardown (dipanggil dari MyMod::disable) ─────────────────────────────
void AutoclickerMod::teardown() {
    stopClicking();
    if (!s_toggleButtonId.empty()) {
        pl::modmenu::unregisterButton(s_toggleButtonId);
        s_toggleButtonId.clear();
    }
    pl::modmenu::unregisterModule("autoclicker_module");
    s_tapX.store(-1.f);
    s_tapY.store(-1.f);
    LOGI("AutoclickerMod teardown complete");
}

// ─── Snapshot config saat ini ─────────────────────────────────────────────
ModConfig AutoclickerMod::currentConfig() {
    ModConfig cfg;
    cfg.enabled         = s_clicking.load();
    cfg.clickIntervalMs = s_intervalMs.load();
    cfg.numberOfTargets = 1;
    cfg.modeAutoHold    = s_autoHold.load();
    return cfg;
}

} // namespace clange_me
