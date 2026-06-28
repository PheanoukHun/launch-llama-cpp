#include "config_loader.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>

std::optional<Config> loadConfig(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        Config cfg;
        if (root["foreground"]) cfg.colors.foreground = root["foreground"].as<std::string>();
        if (root["background"]) cfg.colors.background = root["background"].as<std::string>();
        if (root["selected"]) cfg.colors.selected = root["selected"].as<std::string>();
        if (root["highlight"]) cfg.colors.highlight = root["highlight"].as<std::string>();

        if (root.IsMap()) {
            for (auto it = root.begin(); it != root.end(); ++it) {
                const std::string& key = it->first.as<std::string>();
                const YAML::Node& node = it->second;
                if (node["model"]) {
                    Favorite fav;
                    fav.model = node["model"].as<std::string>();
                    fav.kv_key = node["kv_key"].as<std::string>();
                    fav.kv_value = node["kv_value"].as<std::string>();
                    fav.gpu_layers = node["gpu_layers"].as<int>();
                    fav.agent = node["agent"].as<bool>();
                    cfg.favorites[key] = fav;
                }
            }
        }
        return cfg;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return std::nullopt;
    }
}
