#pragma once

#include <string>
#include <string_view>

#include <pl/Config.hpp>

namespace clange_me {

struct ModConfig {
    int version = 1;
    bool enabled = true;
    int clickIntervalMs = 100;
    int numberOfTargets = 1;
    bool modeAutoHold = false;
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

} // namespace clange_me

template <> struct pl::config::Schema<clange_me::ModConfig> {
    static constexpr std::string_view title = "Konfigurasi Autoclicker";
    static constexpr std::string_view description = {};

    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        if (name == "enabled")
            return {.title = "Aktifkan Autoclicker", .description = "Nyalakan atau matikan fitur autoclicker."};
        if (name == "clickIntervalMs")
            return {.title = "Kecepatan (ms)", .description = "Kecepatan klik dalam milidetik (misal: 100).", .minimum = 1, .maximum = 1000};
        if (name == "numberOfTargets")
            return {.title = "Jumlah Titik Klik", .description = "Berapa banyak target klik di layar (1-5).", .minimum = 1, .maximum = 5};
        if (name == "modeAutoHold")
            return {.title = "Mode Auto-Hold", .description = "Tahan klik alih-alih klik berkali-kali."};
        return {};
    }
};
