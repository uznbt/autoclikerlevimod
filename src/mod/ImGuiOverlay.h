#pragma once
#include "mod/Config.h"

namespace clange_me {

class ImGuiOverlay {
public:
    // Pasang EGL hook + touch hook, baca pengaturan awal dari config
    static void setup(const ModConfig& cfg);

    // Cabut semua hook, bersihkan ImGui context
    static void teardown();

    // Buka/tutup jendela menu in-game (dipanggil dari tombol Levi)
    static void toggleMenu();

    // Ambil pengaturan terkini (untuk disimpan kembali ke config)
    static ModConfig currentConfig();
};

} // namespace clange_me
