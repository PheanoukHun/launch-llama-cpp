#include "config_loader.hpp"
#include <fstream>
#include <iostream>

// Dummy implementation: yaml-cpp is not available in this environment.
// The function simply returns an empty optional after printing a warning.
std::optional<Config> loadConfig(const std::string& path) {
    std::cerr << "yaml-cpp not available; skipping config load from " << path << std::endl;
    return std::nullopt;
}
