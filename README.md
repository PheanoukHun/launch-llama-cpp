# launch-llama

Curses-based TUI launcher for [llama.cpp](https://github.com/ggerganov/llama.cpp) `llama-server` and `llama-swap`.

## Requirements

- Python >= 3.10
- [llama.cpp](https://github.com/ggerganov/llama.cpp) binaries (`llama-server`, optionally `llama-swap`) in PATH or configured via `config.yaml`
- GGUF model files

## Installation

```bash
pip install .
```

Or run directly without installing:

```bash
python3 launch_llama/main.py
```

## Configuration

Edit `config.yaml` in the project root:

```yaml
port: 8080                          # default port for llama-server
models_dir: models                  # path to model files directory
llama_server_path: llama-server     # path to llama-server binary
llama_swap_path: llama-swap         # path to llama-swap binary
llama_swap_config: config/llama-swap-config.yaml  # llama-swap config path
```

Missing fields are auto-appended on first run.

## Usage

### Interactive Mode

```
launch-llama
```

Shows a menu with options:
- **llama-server** — select a model, configure context/quantization/GPU/agent mode, then run
- **favorite: \<name\>** — run a saved preset from `favorites.yaml`
- **llama-swap** — run `llama-swap` with its preconfigured settings

### Quick Run

```
launch-llama run <preset_name>
launch-llama run llama-server
launch-llama run llama-swap
```

### Flags

```
-v, --verbose      Enable debug output
--no-confirm       Skip the confirmation prompt before executing
```

## Favorites

Save model presets in `favorites.yaml`:

```yaml
my-model:
  model: models/Qwen2.5-1.5B-Instruct-Q4_0.gguf
  key_quant: q4_0
  value_quant: q4_0
  port: 8080
  context_size: 2048
  gpu_layers: 16
  agent_mode: false
```

## Project Structure

```
launch-llama/
├── config.yaml                       # global config
├── config/                           # sub-configs (default, favorites, llama-swap)
├── models/                           # model files (ignored by git)
├── config/
│   ├── default.yaml                  # llama-swap port default
│   ├── favorites.yaml                # saved presets
│   └── llama-swap-config.yaml        # llama-swap binary config
├── launch_llama/
│   ├── __init__.py
│   ├── builder.py                    # command string builder
│   ├── cli.py                        # argparse CLI parser
│   ├── config.py                     # config file loading/generation
│   ├── main.py                       # entry point / TUI app
│   ├── models.py                     # model file discovery
│   ├── prompts.py                    # curses prompt helpers
│   └── runner.py                     # subprocess execution
├── launch-llama                      # entry script (Intel oneAPI)
├── launch-llama-default              # entry script (plain)
├── pyproject.toml
└── PROGRAM_FLOW.md
```
