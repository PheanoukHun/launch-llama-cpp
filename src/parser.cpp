#include "parser.hpp"
#include <tclap/CmdLine.h>
#include <iostream>

Args parseArgs(int argc, char* argv[]) {
    Args result;
    try {
        TCLAP::CmdLine cmd("Launch Llama", ' ', "1.0");
        TCLAP::ValueArg<std::string> binaryArg("b", "binary", "Binary to launch (llama-swap | llama-server)", false, "", "string");
        TCLAP::ValueArg<std::string> configArg("c", "config", "Path to yaml config (skips interactive)", false, "", "string");
        TCLAP::ValueArg<int> portArg("p", "port", "Port number", false, 0, "int");
        TCLAP::ValueArg<std::string> modelArg("m", "model", "Model path", false, "", "string");
        TCLAP::ValueArg<int> contextArg("x", "context", "Context size", false, 0, "int");
        TCLAP::ValueArg<std::string> kvKeyArg("k", "kv-key", "Key quantization", false, "", "string");
        TCLAP::ValueArg<std::string> kvValueArg("v", "kv-value", "Value quantization", false, "", "string");
        TCLAP::ValueArg<int> gpuLayersArg("g", "gpu-layers", "GPU layers", false, 0, "int");
    TCLAP::SwitchArg agentSwitch("a", "agent", "Enable agent mode", false);
    TCLAP::SwitchArg noAgentSwitch("n", "no-agent", "Disable agent mode", false);
    TCLAP::SwitchArg verboseSwitch("v", "verbose", "Enable verbose output", false);
    TCLAP::ValueArg<std::string> modelDirArg("d", "model-dir", "Directory containing model subdirectories", false, "./models", "string");

    cmd.add(binaryArg);
    cmd.add(configArg);
    cmd.add(portArg);
    cmd.add(modelArg);
    cmd.add(contextArg);
    cmd.add(kvKeyArg);
    cmd.add(kvValueArg);
    cmd.add(gpuLayersArg);
    cmd.add(agentSwitch);
    cmd.add(noAgentSwitch);
    cmd.add(verboseSwitch);
    cmd.add(modelDirArg);

    cmd.parse(argc, argv);

    if (binaryArg.isSet()) result.binary = binaryArg.getValue();
    if (configArg.isSet()) result.config = configArg.getValue();
    if (portArg.isSet()) result.port = portArg.getValue();
    if (modelArg.isSet()) result.model = modelArg.getValue();
    if (contextArg.isSet()) result.context = contextArg.getValue();
    if (kvKeyArg.isSet()) result.kvKey = kvKeyArg.getValue();
    if (kvValueArg.isSet()) result.kvValue = kvValueArg.getValue();
    if (gpuLayersArg.isSet()) result.gpuLayers = gpuLayersArg.getValue();
    if (agentSwitch.getValue() && noAgentSwitch.getValue()) {
        std::cerr << "Both --agent and --no-agent specified; using --agent.\n";
        result.agent = true;
    } else if (agentSwitch.getValue()) {
        result.agent = true;
    } else if (noAgentSwitch.getValue()) {
        result.agent = false;
    }
    if (verboseSwitch.getValue()) result.verbose = true;
    if (modelDirArg.isSet()) result.modelDir = modelDirArg.getValue();

    } catch (const TCLAP::ArgException &e) {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(EXIT_FAILURE);
    }
    return result;
}
