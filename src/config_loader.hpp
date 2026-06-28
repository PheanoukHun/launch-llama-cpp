#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <unordered_map>
#include <optional>

struct ColorConfig {
    std::string foreground;
    std::string background;
    std::string selected;
    std::string highlight;
};

struct Favorite {
    std::string model;
    std::string kv_key;
    std::string kv_value;
    int gpu_layers;
    bool agent;
};

struct Config {
    ColorConfig colors;
    std::unordered_map<std::string, Favorite> favorites;
};

std::optional<Config> loadConfig(const std::string& path);

#endif // CONFIG_LOADER_HPP
