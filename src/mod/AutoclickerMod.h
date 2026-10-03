#pragma once
#include "mod/Config.h"
#include <string>

namespace clange_me {

class AutoclickerMod {
public:
    // Setup: daftarkan module menu Levi + tombol target layar
    static void setup(const ModConfig& cfg, const std::string& modId);

    // Teardown: unregister semua
    static void teardown();

    // Ambil snapshot pengaturan terkini untuk disimpan ke config
    static ModConfig currentConfig();
};

} // namespace clange_me
