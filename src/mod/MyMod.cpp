#include "mod/MyMod.h"
#include "mod/AutoclickerMod.h"

#include <filesystem>
#include <pl/Mod.hpp>

namespace clange_me {

ClangeMeMod &ClangeMeMod::instance() {
    static ClangeMeMod instance;
    return instance;
}

ClangeMeMod::ClangeMeMod() : mSelf(*ll::mod::NativeMod::current()) {}

bool ClangeMeMod::load() {
    auto &self = getSelf();
    self.getLogger().debug("Loading...");

    std::error_code ec;
    std::filesystem::create_directories(self.getDataDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create data directory {}: {}",
                               self.getDataDir().string(), ec.message());
        return false;
    }
    std::filesystem::create_directories(self.getConfigDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create config directory {}: {}",
                               self.getConfigDir().string(), ec.message());
        return false;
    }

    mConfigFile.emplace();
    if (!mConfigFile->load()) {
        self.getLogger().warn("Failed to load config, using defaults");
    }
    mConfig = mConfigFile->value();

    self.getLogger().info("Loaded {}", self.getName());
    return true;
}

bool ClangeMeMod::enable() {
    auto &self = getSelf();
    self.getLogger().info("Enabling autoclicker...");

    // Daftarkan module menu + tombol target layar via SDK resmi
    AutoclickerMod::setup(mConfig, std::string(self.getId()));

    return true;
}

bool ClangeMeMod::disable() {
    getSelf().getLogger().info("Disabling autoclicker...");

    // Simpan pengaturan terkini ke file config
    mConfig = AutoclickerMod::currentConfig();
    if (mConfigFile) {
        mConfigFile->value() = mConfig;
        mConfigFile->save();
    }

    // Unregister semua dari SDK
    AutoclickerMod::teardown();
    return true;
}

bool ClangeMeMod::unload() {
    getSelf().getLogger().debug("Unloading...");
    mConfigFile.reset();
    return true;
}

} // namespace clange_me
