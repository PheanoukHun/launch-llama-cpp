#include <notcurses/notcurses.h>
#include <vector>
#include <cstdio>
#include <iostream>
#include "parser.hpp"
#include "config_loader.hpp"
#include <cstdlib>
#include <string>
#include <algorithm>
#include <filesystem>
#include <csignal>

// Forward declaration for signal handler
#include <optional>

static App *g_app = nullptr;

static App *g_app = nullptr;

// Application state including UI selections
struct App {
    notcurses_t *nc;
    enum class Screen {
        MAIN_MENU,
        MODEL_LIST,
        KV_QUANT,
        GPU_LAYERS,
        AGENT_MODE,
        RUN_CMD,
        EXIT
    } current = Screen::MAIN_MENU;
    bool running = true;
    // UI selections
    std::string binary = "llama-swap";
    int selectedModelIdx = 0;
    std::vector<std::string> models = {"modelA", "modelB", "modelC"};
    int kvKeyIdx = 0;
    int kvValueIdx = 0;
    int gpuLayers = 20;
    bool agentMode = false;
    int port = 8080;
    int contextSize = 4096;
};

static void render_main_menu(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_printf_at(stdp, 1, 2, "Launch‑Llama: Select binary\n");
    const char *sel = (app->binary == "llama-swap") ? "> " : "  ";
    ncplane_printf_at(stdp, 3, 4, "%s1) llama‑swap", sel);
    sel = (app->binary == "llama-server") ? "> " : "  ";
    ncplane_printf_at(stdp, 4, 4, "%s2) llama‑server", sel);
    ncplane_printf_at(stdp, 6, 2, "Press Enter to continue, 'q' to quit.");
}

static void render_model_list(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_printf_at(stdp, 1, 2, "Model List:\n");
    int y = 3;
    for (size_t i = 0; i < app->models.size(); ++i) {
        const char *prefix = (int)i == app->selectedModelIdx ? "> " : "  ";
        ncplane_printf_at(stdp, y++, 4, "%s%s", prefix, app->models[i].c_str());
    }
    ncplane_printf_at(stdp, y+1, 2, "Press Enter to proceed.");
}

static void render_kv_quant(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    const char *kvTypes[9] = {"f32","f16","bf16","q8_0","q4_0","q4_1","iq4_nl","q5_0","q5_1"};
    ncplane_printf_at(stdp, 1, 2, "KV Quantization:\n");
    ncplane_printf_at(stdp, 3, 4, "Key: %s", kvTypes[app->kvKeyIdx]);
    ncplane_printf_at(stdp, 4, 4, "Value: %s", kvTypes[app->kvValueIdx]);
    ncplane_printf_at(stdp, 6, 2, "Use Left/Right to change, Enter to proceed.");
}

static void render_gpu_layers(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_printf_at(stdp, 1, 2, "GPU Layers:\n");
    ncplane_printf_at(stdp, 3, 4, "Layers: %d (0-40)", app->gpuLayers);
    ncplane_printf_at(stdp, 5, 2, "Use Up/Down to adjust, Enter to proceed.");
}

static void render_agent_mode(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_printf_at(stdp, 1, 2, "Agent Mode:\n");
    ncplane_printf_at(stdp, 3, 4, "Mode: %s", app->agentMode ? "Enabled" : "Disabled");
    ncplane_printf_at(stdp, 5, 2, "Use Left/Right to toggle, Enter to proceed.");
}

static void render_run_cmd(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_printf_at(stdp, 1, 2, "Run Command:\n");
    ncplane_printf_at(stdp, 3, 4, "(This is a placeholder for the actual command output)");
    ncplane_printf_at(stdp, 5, 2, "Press 'q' to exit.");
}

static void render_screen(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_clear(stdp);
    switch (app->current) {
        case App::Screen::MAIN_MENU:     render_main_menu(app);     break;
        case App::Screen::MODEL_LIST:    render_model_list(app);    break;
        case App::Screen::KV_QUANT:      render_kv_quant(app);      break;
        case App::Screen::GPU_LAYERS:    render_gpu_layers(app);    break;
        case App::Screen::AGENT_MODE:    render_agent_mode(app);    break;
        case App::Screen::RUN_CMD:       render_run_cmd(app);       break;
        case App::Screen::EXIT:          /* nothing */;             break;
    }
    notcurses_render(app->nc, 0, 0, NULL);
}

static void execCommand(App *app) {
    const char *kvTypes[9] = {"f32","f16","bf16","q8_0","q4_0","q4_1","iq4_nl","q5_0","q5_1"};
    std::string cmd = std::string("./bin/") + app->binary;
    cmd += " --model " + app->models[app->selectedModelIdx];
    cmd += " --context " + std::to_string(app->contextSize);
    cmd += " --kv-key " + kvTypes[app->kvKeyIdx];
    cmd += " --kv-value " + kvTypes[app->kvValueIdx];
    cmd += " --gpu-layers " + std::to_string(app->gpuLayers);
    if (app->binary == "llama-server") {
        cmd += " --port " + std::to_string(app->port);
    }
    if (app->agentMode) cmd += " --agent"; else cmd += " --no-agent";
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_clear(stdp);
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) {
        ncplane_printf_at(stdp, 1, 2, "Failed to execute command: %s", cmd.c_str());
        notcurses_render(app->nc, 0, 0, NULL);
        return;
    }
    char buffer[256];
    int y = 3;
    while (fgets(buffer, sizeof(buffer), fp)) {
        ncplane_printf_at(stdp, y++, 4, "%s", buffer);
        notcurses_render(app->nc, 0, 0, NULL);
    }
    pclose(fp);
    // After command finishes, exit application
    app->running = false;
}

static void handle_input(App *app, int ch) {
    using Screen = App::Screen;
    switch (app->current) {
        case Screen::MAIN_MENU:
            if (ch == 'q' || ch == 'Q') app->running = false;
            else if (ch == '1') app->binary = "llama-swap";
            else if (ch == '2') app->binary = "llama-server";
            else if (ch == NC_KEY_ENTER || ch == '\n') {
                app->current = Screen::MODEL_LIST;
            }
            break;
        case Screen::MODEL_LIST:
            if (ch == NC_KEY_DOWN) {
                if (app->selectedModelIdx < (int)app->models.size() - 1) app->selectedModelIdx++;
            } else if (ch == NC_KEY_UP) {
                if (app->selectedModelIdx > 0) app->selectedModelIdx--;
            } else if (ch == NC_KEY_ENTER || ch == '\n') {
                app->current = Screen::KV_QUANT;
            } else if (ch == 'q' || ch == 'Q') app->running = false;
            break;
        case Screen::KV_QUANT:
            if (ch == NC_KEY_LEFT) {
                if (app->kvKeyIdx > 0) app->kvKeyIdx--;
            } else if (ch == NC_KEY_RIGHT) {
                if (app->kvKeyIdx < 8) app->kvKeyIdx++;
            } else if (ch == NC_KEY_ENTER || ch == '\n') {
                app->current = Screen::GPU_LAYERS;
            } else if (ch == 'q' || ch == 'Q') app->running = false;
            break;
        case Screen::GPU_LAYERS:
            if (ch == NC_KEY_UP) {
                if (app->gpuLayers < 40) app->gpuLayers++;
            } else if (ch == NC_KEY_DOWN) {
                if (app->gpuLayers > 0) app->gpuLayers--;
            } else if (ch == NC_KEY_ENTER || ch == '\n') {
                app->current = Screen::AGENT_MODE;
            } else if (ch == 'q' || ch == 'Q') app->running = false;
            break;
        case Screen::AGENT_MODE:
            if (ch == NC_KEY_LEFT) app->agentMode = false;
            else if (ch == NC_KEY_RIGHT) app->agentMode = true;
            else if (ch == NC_KEY_ENTER || ch == '\n') {
                app->current = Screen::RUN_CMD;
                execCommand(app);
            } else if (ch == 'q' || ch == 'Q') app->running = false;
            break;
        case Screen::RUN_CMD:
            if (ch == 'q' || ch == 'Q') app->running = false;
            break;
        default:
            break;
    }
}

static int kvStringToIdx(const std::string &s) {
    const char *kvTypes[9] = {"f32","f16","bf16","q8_0","q4_0","q4_1","iq4_nl","q5_0","q5_1"};
    for (int i=0;i<9;++i) if (s==kvTypes[i]) return i;
    return 0;
}

static void applyFavorite(App *app, const Favorite &fav) {
    auto it = std::find(app->models.begin(), app->models.end(), fav.model);
    if (it != app->models.end()) app->selectedModelIdx = std::distance(app->models.begin(), it);
    else { app->models.push_back(fav.model); app->selectedModelIdx = app->models.size()-1; }
    app->kvKeyIdx = kvStringToIdx(fav.kv_key);
    app->kvValueIdx = kvStringToIdx(fav.kv_value);
    app->gpuLayers = fav.gpu_layers;
    app->agentMode = fav.agent;
}

static void loadModels(const std::string &dir, std::vector<std::string> &out){
    namespace fs = std::filesystem;
    try {
        if (!fs::exists(dir)) return;
        for (const auto &p : fs::directory_iterator(dir)) {
            if (p.is_directory()) {
                out.push_back(p.path().filename().string());
            }
        }
    } catch (const std::exception &e) {
        // ignore errors
    }
}

static void sigint_handler(int){ if(g_app){ g_app->running=false; }}

int main(int argc, char *argv[]) {
    Args args = parseArgs(argc, argv);
    std::optional<Config> cfgOpt;
    if (args.config) {
        cfgOpt = loadConfig(*args.config);
        if (!cfgOpt) {
            std::cerr << "Unable to load configuration file." << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "Loaded config for " << cfgOpt->favorites.size() << " favorites." << std::endl;
    }
    struct nc_opts opts = {0};
    opts.flags = NC_OPTION_STDIN;
    notcurses_t *nc = notcurses_init(&opts, NULL);
    if (!nc) {
        std::cerr << "Failed to initialize notcurses." << std::endl;
        return EXIT_FAILURE;
    }
    App app{nc};
    g_app = &app; // for signal handler
    signal(SIGINT, sigint_handler);
    // Load model directory
    if (args.modelDir) {
        loadModels(*args.modelDir, app.models);
        if (app.models.empty()) app.models.push_back("modelA");
    } else {
        loadModels("./models", app.models);
        if (app.models.empty()) app.models.push_back("modelA");
    }
    // Apply args and favorites
    if (args.binary) app.binary = *args.binary;
    if (cfgOpt) {
        auto it = cfgOpt->favorites.find(app.binary);
        if (it != cfgOpt->favorites.end()) {
            applyFavorite(&app, it->second);
        }
    }
    if (args.model) {
        auto it = std::find(app.models.begin(), app.models.end(), *args.model);
        if (it != app.models.end()) app.selectedModelIdx = std::distance(app.models.begin(), it);
        else {
            app.models.push_back(*args.model);
            app.selectedModelIdx = app.models.size() - 1;
        }
    }
    if (args.port) app.port = *args.port;
    if (args.contextSize) app.contextSize = *args.contextSize;
    if (args.kvKey) app.kvKeyIdx = kvStringToIdx(*args.kvKey);
    if (args.kvValue) app.kvValueIdx = kvStringToIdx(*args.kvValue);
    if (args.gpuLayers) app.gpuLayers = *args.gpuLayers;
    if (args.agent) app.agentMode = true;
    if (args.noAgent) app.agentMode = false;
    if (args.verbose) {
        std::cout << "Verbose mode enabled." << std::endl;
        std::cout << "Selected binary: " << app.binary << std::endl;
        std::cout << "Model list: ";
        for (auto &m: app.models) std::cout << m << ", ";
        std::cout << std::endl;
    }
    while (app.running) {
        render_screen(&app);
        int ch = notcurses_getch(nc, NULL, -1);
        if (ch == -1) continue;
        handle_input(&app, ch);
    }
    notcurses_stop(nc, true);
    return EXIT_SUCCESS;
}
