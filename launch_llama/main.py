import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from curses import wrapper, has_colors, init_pair, curs_set
import curses

from launch_llama import builder
from launch_llama import config
from launch_llama import models
from launch_llama import prompts
from launch_llama import runner


def curses_app(stdscr):
    stdscr.keypad(True)

    if curses.has_colors():
        curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)

    try:
        curses.curs_set(0)
    except:
        pass

    cfg = config.Config()
    cfg.load("config.yaml", "favorites.yaml", "include/default.yaml")

    models_dir = "models"
    model_list = models.discover_models(models_dir)

    if not model_list:
        stdscr.clear()
        stdscr.addstr(0, 2, "No Models Found", curses.A_BOLD)
        stdscr.addstr(2, 2, "Add model files to the models/ directory")
        stdscr.addstr(3, 2, "Supported: .gguf .bin .safetensors .ggml .onnx")
        stdscr.addstr(5, 2, "Press any key to exit")
        stdscr.refresh()
        stdscr.getch()
        return None

    menu = ["llama-server"]
    fav_keys = list(cfg.favorites.keys())
    menu.extend(f"favorite: {k}" for k in fav_keys)
    menu.append("llama-swap")

    choice = prompts.selection(stdscr, "Launch Menu", menu, default_idx=0)
    if choice is None:
        return None

    if choice == "llama-server":
        selected = prompts.selection(stdscr, "Select Model", model_list, default_idx=0)
        if selected is None:
            return None
        model_path = os.path.join(models_dir, selected)

        ctx_size = prompts.integer(
            stdscr, "Context Size", min_val=1, max_val=65535, default=4096
        )
        if ctx_size is None:
            return None

        quants = ["f32", "f16", "bf16", "q8_0", "q4_0", "q4_1", "iq4_nl", "q5_0", "q5_1"]
        key_quant = prompts.selection(stdscr, "Key Quantization", quants, default_idx=3)
        if key_quant is None:
            return None

        value_quant = prompts.selection(stdscr, "Value Quantization", quants, default_idx=3)
        if value_quant is None:
            return None

        gpu_layers = prompts.integer(
            stdscr, "GPU Layers", min_val=0, max_val=40, default=20
        )
        if gpu_layers is None:
            return None

        agent_mode = prompts.confirm(stdscr, "Enable Agent Mode?", default=False)
        if agent_mode is None:
            return None

        return ("llama-server", {
            "model_path": model_path,
            "key_quant": key_quant,
            "value_quant": value_quant,
            "port": cfg.port,
            "context_size": ctx_size,
            "gpu_layers": gpu_layers,
            "agent_mode": agent_mode,
        })

    if choice.startswith("favorite: "):
        fav_name = choice[len("favorite: ") :]
        return ("favorite", cfg.favorites[fav_name])

    if choice == "llama-swap":
        return ("llama-swap", cfg.default_port)

    return None


def main():
    try:
        result = wrapper(curses_app)
    except KeyboardInterrupt:
        return 130

    if result is None:
        return 0

    action, data = result

    if action == "llama-server":
        cmd = builder.build_llama_server_command(**data)
        print(f"\n{cmd}\n")
        return runner.Runner().run(cmd)

    if action == "favorite":
        fav = data
        model_path = fav.get("model", "")
        cmd = builder.build_llama_server_command(
            model_path=model_path,
            key_quant=fav.get("key_quant", "q8_0"),
            value_quant=fav.get("value_quant", "q8_0"),
            port=int(fav.get("port", 8080)),
            context_size=int(fav.get("context_size", 4096)),
            gpu_layers=int(fav.get("gpu_layers", 20)),
            agent_mode=fav.get("agent_mode", "").lower() == "true",
        )
        print(f"\n{cmd}\n")
        return runner.Runner().run(cmd)

    if action == "llama-swap":
        cmd = builder.build_llama_swap_command(data)
        print(f"\n{cmd}\n")
        return runner.Runner().run(cmd)

    return 1


if __name__ == "__main__":
    sys.exit(main())
