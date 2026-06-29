#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <optional>

struct Args {
    std::optional<std::string> binary;
    std::optional<std::string> config;
    std::optional<int> port;
    std::optional<std::string> model;
    std::optional<int> context;
    std::optional<std::string> kvKey;
    std::optional<std::string> kvValue;
    std::optional<int> gpuLayers;
    bool agent = false;
    bool noAgent = false;
    bool verbose = false;
    std::optional<std::string> modelDir;
};

Args parseArgs(int argc, char* argv[]);

#endif
