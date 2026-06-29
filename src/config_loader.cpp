#include "config_loader.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>

std::optional<Config> loadConfig(const std::string& colorsPath, const std::string& favoritesPath) {
    Config cfg;

    cfg.colors.foreground = "#FFFFFF";
    cfg.colors.background = "#000000";
    cfg.colors.selected = "#FFFF00";
    cfg.colors.highlight = "#00FF00";

    try {
        YAML::Node node = YAML::LoadFile(colorsPath);
        if (node["colors"]) {
            auto c = node["colors"];
            if (c["foreground"]) cfg.colors.foreground = c["foreground"].as<std::string>();
            if (c["background"]) cfg.colors.background = c["background"].as<std::string>();
            if (c["selected"])   cfg.colors.selected   = c["selected"].as<std::string>();
            if (c["highlight"])  cfg.colors.highlight  = c["highlight"].as<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: could not load colors config (" << colorsPath << "): " << e.what() << std::endl;
    }

    try {
        YAML::Node node = YAML::LoadFile(favoritesPath);
        if (node["favorites"]) {
            for (const auto& entry : node["favorites"]) {
                std::string name = entry.first.as<std::string>();
                auto f = entry.second;
                Favorite fav;
                fav.binary   = f["binary"].as<std::string>("llama-server");
                fav.model    = f["model"].as<std::string>("");
                fav.context  = f["context"].as<int>(4096);
                fav.kv_key   = f["kv_key"].as<std::string>("f16");
                fav.kv_value = f["kv_value"].as<std::string>("f16");
                fav.gpu_layers = f["gpu_layers"].as<int>(20);
                fav.agent    = f["agent"].as<bool>(false);
                fav.port     = f["port"].as<int>(0);
                cfg.favorites[name] = fav;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: could not load favorites config (" << favoritesPath << "): " << e.what() << std::endl;
    }

    return cfg;
}
