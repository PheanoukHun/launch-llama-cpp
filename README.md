# launch-llama

Launch and manage `llama-server` instances with an interactive curses TUI.

## Quick Start

```bash
./launch-llama
```

Requires Python 3.10+ and a `llama-server` or `llama-swap` binary in your `PATH`.

## Features

- **Top-level menu**: Choose between `llama-server` (interactive setup), saved favorites, or `llama-swap`
- **Arrow-key navigation**: Navigate selection lists with ↑↓, confirm with Enter
- **Interactive setup**: Select a model, configure context size, quantization, GPU layers, and agent mode
- **Favorites**: Save and load model presets from `favorites.yaml`
- **Intel oneAPI**: Automatically sources `/opt/intel/oneapi/setvars.sh` on launch

## Usage

| Key | Action |
|---|---|
| ↑ ↓ | Navigate |
| Enter | Confirm |
| q / Esc | Quit |

```bash
./launch-llama              # interactive TUI
```

## Configuration

- `config.yaml` — global port setting
- `favorites.yaml` — named model presets
- `include/default.yaml` — llama-swap defaults
- `include/llama-swap-config.yaml` — llama-swap tool config

Models go in the `models/` directory (`.gguf`, `.bin`, `.safetensors`, `.ggml`, `.onnx`).

## Install

```bash
pip install .
```
