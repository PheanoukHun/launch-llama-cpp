import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import curses
from curses import curs_set, has_colors, init_pair, wrapper

from launch_llama import builder, config, models, prompts, runner


def curses_app(stdscr, verbose=False):
    stdscr.keypad(True)

    if curses.has_colors():
        curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)

    try:
        curses.curs_set(0)
    except:
        pass

    if verbose:
        stdscr.addstr(0, 2, "Loading configuration...", curses.A_DIM)
        stdscr.refresh()

    script_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    config_path = os.path.join(script_dir, "config.yaml")

    config.generate_defaults(config_path)

    cfg = config.Config()
    cfg.load(config_path)

    model_list = models.discover_models(cfg.models_dir)

    if not model_list:
        stdscr.clear()
        stdscr.addstr(0, 2, "No Models Found", curses.A_BOLD)
        stdscr.addstr(2, 2, f"Add model files to {cfg.models_dir}/")
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
        model_path = os.path.join(cfg.models_dir, selected)

        ctx_size = prompts.integer(
            stdscr, "Context Size", min_val=1, max_val=1048576, default=4096
        )
        if ctx_size is None:
            return None

        quants = [
            "f32",
            "f16",
            "bf16",
            "q8_0",
            "q4_0",
            "q4_1",
            "iq4_nl",
            "q5_0",
            "q5_1",
        ]
        key_quant = prompts.selection(stdscr, "Key Quantization", quants, default_idx=3)
        if key_quant is None:
            return None

        value_quant = prompts.selection(
            stdscr, "Value Quantization", quants, default_idx=3
        )
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

        return (
            "llama-server",
            {
                "llama_server_path": cfg.llama_server_path,
                "model_path": model_path,
                "key_quant": key_quant,
                "value_quant": value_quant,
                "port": cfg.port,
                "context_size": ctx_size,
                "gpu_layers": gpu_layers,
                "agent_mode": agent_mode,
            },
        )

    if choice.startswith("favorite: "):
        fav_name = choice[len("favorite: ") :]
        fav_data = dict(cfg.favorites[fav_name])
        fav_data.setdefault("llama_server_path", cfg.llama_server_path)
        return ("favorite", fav_data)

    if choice == "llama-swap":
        return (
            "llama-swap",
            {
                "port": cfg.default_port,
                "llama_swap_path": cfg.llama_swap_path,
                "llama_swap_config": cfg.llama_swap_config,
            },
        )

    return None


def confirm_run(cmd):
    """Ask user to confirm before running the command."""
    print("\n" + "=" * 60)
    print("Command to execute:")
    print("-" * 60)
    print(cmd)
    print("=" * 60)
    try:
        resp = input("\nRun this command? [Y/n]: ").strip().lower()
    except (EOFError, KeyboardInterrupt):
        print()
        return False
    if resp in ("", "y", "yes"):
        return True
    return False


def main():
    parser = argparse.ArgumentParser(
        prog="launch-llama", description="Launch and manage llama-server instances"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose/debug output"
    )
    parser.add_argument(
        "--no-confirm",
        action="store_true",
        help="Skip confirmation prompt before running command",
    )
    args, _ = parser.parse_known_args()

    verbose = args.verbose
    no_confirm = args.no_confirm

    try:
        result = wrapper(lambda stdscr: curses_app(stdscr, verbose=verbose))
    except KeyboardInterrupt:
        return 130

    if result is None:
        return 0

    action, data = result

    if action == "llama-server":
        cmd = builder.build_llama_server_command(**data)
        if not no_confirm and not confirm_run(cmd):
            print("Cancelled.")
            return 0
        print(f"\n{cmd}\n")
        return runner.Runner(verbose=verbose).run(cmd)

    if action == "favorite":
        fav = data
        model_path = fav.get("model", "")
        cmd = builder.build_llama_server_command(
            llama_server_path=fav["llama_server_path"],
            model_path=model_path,
            key_quant=str(fav.get("key_quant", "q8_0")),
            value_quant=str(fav.get("value_quant", "q8_0")),
            port=int(fav.get("port", 8080)),
            context_size=int(fav.get("context_size", fav.get("context", 4096))),
            gpu_layers=int(fav.get("gpu_layers", 20)),
            agent_mode=fav.get("agent_mode", False),
        )
        if not no_confirm and not confirm_run(cmd):
            print("Cancelled.")
            return 0
        print(f"\n{cmd}\n")
        return runner.Runner(verbose=verbose).run(cmd)

    if action == "llama-swap":
        cmd = builder.build_llama_swap_command(**data)
        if not no_confirm and not confirm_run(cmd):
            print("Cancelled.")
            return 0
        print(f"\n{cmd}\n")
        return runner.Runner(verbose=verbose).run(cmd)

    return 1


if __name__ == "__main__":
    sys.exit(main())
