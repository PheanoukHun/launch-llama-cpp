#include <notcurses/notcurses.h>
#include <vector>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include "parser.hpp"
#include "config_loader.hpp"
#include <cstdlib>
#include <string>
#include <filesystem>
#include <csignal>
#include <optional>
#include <unistd.h>

static constexpr const char* KV_TYPES[9] = {
    "f32", "f16", "bf16", "q8_0", "q4_0", "q4_1", "iq4_nl", "q5_0", "q5_1"
};

static constexpr int CONTEXT_SIZES[8] = {
    256, 512, 1024, 2048, 4096, 8192, 16384, 32768
};

struct App {
    notcurses *nc;

    enum class Screen {
        MAIN_MENU,
        MODEL_LIST,
        CONTEXT_SIZE,
        KV_KEY_QUANT,
        KV_VALUE_QUANT,
        GPU_LAYERS,
        AGENT_MODE,
        RUN_CMD,
        EXIT
    } current = Screen::MAIN_MENU;

    bool running = true;

    int menuIdx = 0;

    std::string binary;
    std::vector<std::string> models;
    int selectedModelIdx = 0;
    std::string modelDir = "./models";

    int contextIdx = 4;   /* 4096 */
    int kvKeyIdx   = 1;   /* f16 */
    int kvValueIdx = 0;   /* f32 */
    int gpuLayers  = 20;
    bool agentMode = false;
    int  port       = 8080;
    bool portFromCli = false;

    bool commandExecuted = false;

    std::unordered_map<std::string, Favorite> favorites;
};

static App *g_app = nullptr;

static std::string getExeDir() {
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::string p(buf);
        auto pos = p.rfind('/');
        if (pos != std::string::npos)
            return p.substr(0, pos);
    }
    return ".";
}

static int menuItemCount(App *app) {
    return 2 + (int)app->favorites.size();
}

static void render_binary_menu(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2,
        "Select Binary Launch Screen\n");
    ncplane_printf_yx(stdp, 2, 2,
        "----------------------------\n");

    int y = 4;
    int num = menuItemCount(app);
    for (int i = 0; i < num; ++i) {
        const char *sel = i == app->menuIdx ? "> " : "  ";
        const char *label;
        if (i == 0)
            label = "llama-server";
        else if (i == 1)
            label = "llama-swap";
        else {
            auto it = app->favorites.begin();
            std::advance(it, i - 2);
            label = it->first.c_str();
        }
        ncplane_printf_yx(stdp, y++, 4, "%s%d) %s", sel, i + 1, label);
    }
    ncplane_printf_yx(stdp, y + 1, 2,
        "Use Up/Down to navigate, Enter to select, 'q' to quit.");
}

static void render_model_list(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "Choose a model:\n");
    ncplane_printf_yx(stdp, 2, 2, "---------------\n");

    int y = 4;
    for (size_t i = 0; i < app->models.size(); ++i) {
        const char *sel = (int)i == app->selectedModelIdx ? "> " : "  ";
        ncplane_printf_yx(stdp, y++, 4, "%s%s",
            sel, app->models[i].c_str());
    }
    ncplane_printf_yx(stdp, y + 1, 2,
        "Use Up/Down to navigate, Enter to confirm, 'q' to quit.");
}

static void render_context_size(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "Context size:\n");
    ncplane_printf_yx(stdp, 3, 4, "%d  (power of 2)",
        CONTEXT_SIZES[app->contextIdx]);
    ncplane_printf_yx(stdp, 5, 2,
        "Use Left/Right to change, Enter to confirm, 'q' to quit.");
}

static void render_kv_key_quant(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "KV key quantization format:\n");
    ncplane_printf_yx(stdp, 3, 4, "%s", KV_TYPES[app->kvKeyIdx]);
    ncplane_printf_yx(stdp, 5, 2,
        "Use Left/Right to change, Enter to confirm, 'q' to quit.");
}

static void render_kv_value_quant(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "KV value quantization format:\n");
    ncplane_printf_yx(stdp, 3, 4, "%s", KV_TYPES[app->kvValueIdx]);
    ncplane_printf_yx(stdp, 5, 2,
        "Use Left/Right to change, Enter to confirm, 'q' to quit.");
}

static void render_gpu_layers(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "GPU layers:\n");
    ncplane_printf_yx(stdp, 3, 4, "%d  (0-40)", app->gpuLayers);
    ncplane_printf_yx(stdp, 5, 2,
        "Use Up/Down to adjust, Enter to confirm, 'q' to quit.");
}

static void render_agent_mode(App *app) {
    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "Agent mode:\n");
    ncplane_printf_yx(stdp, 3, 4, "%s",
        app->agentMode ? "Enabled" : "Disabled");
    ncplane_printf_yx(stdp, 5, 2,
        "Use Left/Right to toggle, Enter to confirm, 'q' to quit.");
}



static void render_screen(App *app) {
    switch (app->current) {
    case App::Screen::MAIN_MENU:      render_binary_menu(app);     break;
    case App::Screen::MODEL_LIST:     render_model_list(app);      break;
    case App::Screen::CONTEXT_SIZE:   render_context_size(app);    break;
    case App::Screen::KV_KEY_QUANT:   render_kv_key_quant(app);   break;
    case App::Screen::KV_VALUE_QUANT: render_kv_value_quant(app);  break;
    case App::Screen::GPU_LAYERS:     render_gpu_layers(app);      break;
    case App::Screen::AGENT_MODE:     render_agent_mode(app);      break;
    case App::Screen::RUN_CMD:
    case App::Screen::EXIT:
        return;
    }
    notcurses_refresh(app->nc, nullptr, nullptr);
}

static std::string buildCommand(App *app) {
    if (app->binary == "llama-swap")
        return "llama-swap -config llama-swap-config.yaml -listen localhost:"
             + std::to_string(app->port);

    std::string cmd = "llama-server";
    cmd += " --model " + app->modelDir + "/" + app->models[app->selectedModelIdx];
    cmd += std::string(" -ctk ") + KV_TYPES[app->kvKeyIdx];
    cmd += std::string(" -ctv ") + KV_TYPES[app->kvValueIdx];
    cmd += " -fa on";
    cmd += " -ngl " + std::to_string(app->gpuLayers);
    cmd += " --port " + std::to_string(app->port);
    cmd += " -c " + std::to_string(CONTEXT_SIZES[app->contextIdx]);
    cmd += app->agentMode ? " --agent" : " --no-agent";
    return cmd;
}

static void execCommand(App *app) {
    app->commandExecuted = true;
    std::string cmd = buildCommand(app);

    ncplane *stdp = notcurses_stdplane(app->nc);
    ncplane_erase(stdp);
    ncplane_printf_yx(stdp, 1, 2, "Running:\n");
    ncplane_printf_yx(stdp, 2, 2, "--------\n");
    ncplane_printf_yx(stdp, 3, 2, "%s", cmd.c_str());
    ncplane_printf_yx(stdp, 5, 2, "Output:\n");
    notcurses_render(app->nc);

    FILE *fp = popen((cmd + " 2>&1").c_str(), "r");
    if (!fp) {
        ncplane_printf_yx(stdp, 6, 2, "Failed to execute command.");
        notcurses_render(app->nc);
        sleep(2);
        app->running = false;
        return;
    }

    char buf[256];
    int y = 6;
    while (fgets(buf, sizeof(buf), fp)) {
        ncplane_printf_yx(stdp, y++, 4, "%s", buf);
        notcurses_render(app->nc);
        if (y > 90) break;
    }
    int rc = pclose(fp);

    ncplane_printf_yx(stdp, y + 1, 2, "Command exited with code %d.", rc);
    ncplane_printf_yx(stdp, y + 2, 2, "Press any key to exit.");
    notcurses_render(app->nc);

    notcurses_get_blocking(app->nc, nullptr);
    app->running = false;
}

static int kvStringToIdx(const std::string &s) {
    for (int i = 0; i < 9; ++i)
        if (s == KV_TYPES[i]) return i;
    return 1;
}

static int contextStringToIdx(const std::string &s) {
    int val = std::stoi(s);
    for (int i = 0; i < 8; ++i)
        if (CONTEXT_SIZES[i] == val) return i;
    return 4;
}

static void loadModels(const std::string &dir, std::vector<std::string> &out) {
    namespace fs = std::filesystem;
    try {
        if (!fs::exists(dir)) return;
        for (const auto &p : fs::directory_iterator(dir))
            if (p.is_directory())
                out.push_back(p.path().filename().string());
    } catch (const std::exception &) {}
}

static void sigint_handler(int) {
    if (g_app) g_app->running = false;
}

static void handle_input(App *app, int ch) {
    using Screen = App::Screen;

    if (ch == 'q' || ch == 'Q') {
        app->running = false;
        return;
    }

    switch (app->current) {

    case Screen::MAIN_MENU: {
        int num = menuItemCount(app);
        if (ch == NCKEY_UP && app->menuIdx > 0)
            --app->menuIdx;
        else if (ch == NCKEY_DOWN && app->menuIdx < num - 1)
            ++app->menuIdx;
        else if (ch == '\n' || ch == NCKEY_ENTER) {
            if (app->menuIdx == 0) {
                app->binary  = "llama-server";
                if (!app->portFromCli) app->port = 8080;
                app->current = Screen::MODEL_LIST;
            } else if (app->menuIdx == 1) {
                app->binary  = "llama-swap";
                if (!app->portFromCli) app->port = 9000;
                app->current = Screen::RUN_CMD;
            } else {
                auto it = app->favorites.begin();
                std::advance(it, app->menuIdx - 2);
                auto &fav = it->second;
                app->binary    = fav.binary;
                app->contextIdx = contextStringToIdx(std::to_string(fav.context));
                app->kvKeyIdx   = kvStringToIdx(fav.kv_key);
                app->kvValueIdx = kvStringToIdx(fav.kv_value);
                app->gpuLayers  = fav.gpu_layers;
                app->agentMode  = fav.agent;
                if (fav.port != 0)
                    app->port = fav.port;
                else if (!app->portFromCli)
                    app->port = (app->binary == "llama-swap") ? 9000 : 8080;
                if (!fav.model.empty()) {
                    auto mit = std::find(app->models.begin(), app->models.end(), fav.model);
                    if (mit == app->models.end()) {
                        app->models.push_back(fav.model);
                        app->selectedModelIdx = (int)app->models.size() - 1;
                    } else {
                        app->selectedModelIdx = (int)(mit - app->models.begin());
                    }
                }
                app->current = Screen::RUN_CMD;
            }
        }
        break;
    }

    case Screen::MODEL_LIST:
        if (ch == NCKEY_UP && app->selectedModelIdx > 0)
            --app->selectedModelIdx;
        else if (ch == NCKEY_DOWN && app->selectedModelIdx < (int)app->models.size() - 1)
            ++app->selectedModelIdx;
        else if (ch == '\n' || ch == NCKEY_ENTER)
            app->current = Screen::CONTEXT_SIZE;
        break;

    case Screen::CONTEXT_SIZE:
        if (ch == NCKEY_LEFT && app->contextIdx > 0)
            --app->contextIdx;
        else if (ch == NCKEY_RIGHT && app->contextIdx < 7)
            ++app->contextIdx;
        else if (ch == '\n' || ch == NCKEY_ENTER)
            app->current = Screen::KV_KEY_QUANT;
        break;

    case Screen::KV_KEY_QUANT:
        if (ch == NCKEY_LEFT && app->kvKeyIdx > 0)
            --app->kvKeyIdx;
        else if (ch == NCKEY_RIGHT && app->kvKeyIdx < 8)
            ++app->kvKeyIdx;
        else if (ch == '\n' || ch == NCKEY_ENTER)
            app->current = Screen::KV_VALUE_QUANT;
        break;

    case Screen::KV_VALUE_QUANT:
        if (ch == NCKEY_LEFT && app->kvValueIdx > 0)
            --app->kvValueIdx;
        else if (ch == NCKEY_RIGHT && app->kvValueIdx < 8)
            ++app->kvValueIdx;
        else if (ch == '\n' || ch == NCKEY_ENTER)
            app->current = Screen::GPU_LAYERS;
        break;

    case Screen::GPU_LAYERS:
        if (ch == NCKEY_UP && app->gpuLayers < 40)
            ++app->gpuLayers;
        else if (ch == NCKEY_DOWN && app->gpuLayers > 0)
            --app->gpuLayers;
        else if (ch == '\n' || ch == NCKEY_ENTER)
            app->current = Screen::AGENT_MODE;
        break;

    case Screen::AGENT_MODE:
        if (ch == NCKEY_LEFT)
            app->agentMode = false;
        else if (ch == NCKEY_RIGHT)
            app->agentMode = true;
        else if (ch == '\n' || ch == NCKEY_ENTER) {
            app->current = Screen::RUN_CMD;
            execCommand(app);
        }
        break;

    case Screen::RUN_CMD:
        app->running = false;
        break;

    default: break;
    }
}

int main(int argc, char *argv[]) {
    Args args = parseArgs(argc, argv);

    std::string exeDir = getExeDir();
    std::string colorsPath = exeDir + "/../include/config.yaml";
    std::string favoritesPath = exeDir + "/../include/favorites.yaml";

    if (!std::filesystem::exists(colorsPath)) {
        colorsPath = "include/config.yaml";
        favoritesPath = "include/favorites.yaml";
    }

    if (args.config) {
        auto p = std::filesystem::path(*args.config);
        colorsPath = *args.config;
        favoritesPath = (p.parent_path() / "favorites.yaml").string();
    }

    std::optional<Config> cfg = loadConfig(colorsPath, favoritesPath);

    /* ---- bypass-TUI mode: --bin and --config both given ---- */
    if (args.binary && args.config) {
        std::string bin = *args.binary;
        Favorite fav;
        if (cfg) {
            for (auto &[_, f] : cfg->favorites) {
                if (f.binary == bin) { fav = f; break; }
            }
        }

        std::string model     = args.model.value_or(fav.model);
        int   context = args.context.value_or(fav.context ? fav.context : 4096);
        std::string kvKey   = args.kvKey.value_or(
            fav.kv_key.empty() ? "f16" : fav.kv_key);
        std::string kvValue = args.kvValue.value_or(
            fav.kv_value.empty() ? "f32" : fav.kv_value);
        int   gpuLayersArg = args.gpuLayers.value_or(
            fav.gpu_layers ? fav.gpu_layers : 20);
        bool   agent   = args.agent ? true : (args.noAgent ? false : fav.agent);
        int    defaultPort = (bin == "llama-swap") ? 9000 : 8080;
        int    portVal = args.port.value_or(fav.port != 0 ? fav.port : defaultPort);
        std::string mdir = args.modelDir.value_or("./models");

        if (bin == "llama-swap") {
            std::string cmd = "llama-swap -config llama-swap-config.yaml -listen localhost:"
                            + std::to_string(portVal);
            std::cout << "Running: " << cmd << std::endl;
            return system(cmd.c_str());
        }

        std::string cmd = "llama-server --model " + mdir + "/" + model
                        + " -ctk " + kvKey
                        + " -ctv " + kvValue
                        + " -fa on"
                        + " -ngl " + std::to_string(gpuLayersArg)
                        + " --port " + std::to_string(portVal)
                        + " -c " + std::to_string(context)
                        + (agent ? " --agent" : " --no-agent");

        std::cout << "Running: " << cmd << std::endl;
        return system(cmd.c_str());
    }

    /* ---- TUI mode ---- */
    notcurses_options opts{};
    opts.flags = NCOPTION_CLI_MODE;
    notcurses *nc = notcurses_init(&opts, nullptr);
    if (!nc) {
        std::cerr << "Failed to initialise notcurses." << std::endl;
        return EXIT_FAILURE;
    }
    notcurses_enter_alternate_screen(nc);

    App app{};
    app.nc = nc;
    g_app = &app;
    signal(SIGINT, sigint_handler);

    if (cfg)
        app.favorites = cfg->favorites;

    /* load models */
    std::string mdir = args.modelDir.value_or("./models");
    app.modelDir = mdir;
    loadModels(mdir, app.models);
    if (app.models.empty())
        loadModels(exeDir + "/../models", app.models);
    if (app.models.empty())
        loadModels("./models", app.models);
    if (app.models.empty())
        app.models = {"default"};

    /* apply CLI overrides */
    if (args.binary)  app.binary = *args.binary;
    if (args.model) {
        auto it = std::find(app.models.begin(), app.models.end(), *args.model);
        if (it != app.models.end())
            app.selectedModelIdx = (int)(it - app.models.begin());
        else {
            app.models.push_back(*args.model);
            app.selectedModelIdx = (int)app.models.size() - 1;
        }
    }
    if (args.context)  app.contextIdx = contextStringToIdx(std::to_string(*args.context));
    if (args.kvKey)    app.kvKeyIdx   = kvStringToIdx(*args.kvKey);
    if (args.kvValue)  app.kvValueIdx = kvStringToIdx(*args.kvValue);
    if (args.gpuLayers) app.gpuLayers = *args.gpuLayers;
    if (args.agent)    app.agentMode  = true;
    if (args.noAgent)  app.agentMode  = false;
    if (args.port)    { app.port = *args.port; app.portFromCli = true; }

    /* set starting screen */
    if (args.binary) {
        if (app.binary == "llama-swap" && !app.portFromCli)
            app.port = 9000;
        app.current = (app.binary == "llama-swap")
                     ? App::Screen::RUN_CMD
                     : App::Screen::MODEL_LIST;
    } else {
        app.current = App::Screen::MAIN_MENU;
    }

    if (args.verbose) {
        std::cout << "Verbose: binary=" << app.binary
                  << "  models=";
        for (auto &m : app.models) std::cout << m << ",";
        std::cout << std::endl;
    }

    /* main event loop */
    while (app.running) {
        render_screen(&app);
        if (!app.running) break;

        /* llama-swap (no questions) – exec immediately */
        if (app.current == App::Screen::RUN_CMD && !app.commandExecuted) {
            execCommand(&app);
            continue;
        }

        uint32_t ch = notcurses_get_blocking(nc, nullptr);
        if (ch == (uint32_t)-1) continue;
        handle_input(&app, (int)ch);
    }

    notcurses_stop(nc);
    return EXIT_SUCCESS;
}
