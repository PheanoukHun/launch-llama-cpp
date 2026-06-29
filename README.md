# Launch-Llama

## Overview

Launch-Llama is a TUI application written in **C++20** that launches either `llama-swap` or `llama-server` with configurable options. It uses the following third-party libraries:

- **notcurses** – terminal UI
- **tclap** – command-line argument parsing
- **yaml-cpp** – YAML configuration parsing

The project is organized into a single source tree and can be built on any Linux distribution that provides the required dependencies.

## Installing Dependencies

### Debian/Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libnotcurses-dev libtclap-dev libyaml-cpp-dev
```

### Fedora / CentOS

```bash
sudo dnf install -y gcc-c++ cmake notcurses-devel tclap-devel yaml-cpp-devel
```

### Arch Linux

```bash
sudo pacman -Syu --needed base-devel notcurses tclap yaml-cpp
```

> **Note**: If a package for a library is not available, you can build it from source. The README in the library's repository contains build instructions.

## Building the Project

```bash
make
```

The `Makefile` in the repository is configured to use `CXXFLAGS` for C++20 and links against the required libraries. The resulting binary `launch-llama` is placed in the `build` directory.

### Custom Build Options

If you installed libraries in a non-standard location, adjust the Makefile:

```make
CXXFLAGS += -I/opt/notcurses/include
LDFLAGS  += -L/opt/notcurses/lib
```

Alternatively, export `CXXFLAGS` and `LDFLAGS` before invoking `make`.

## Running the Application

```bash
./build/launch-llama [options]
```

### Common Options

| Option | Description |
|--------|-------------|
| `--bin <name>` | Specify `llama-swap` or `llama-server` to skip the binary selection menu. |
| `--config <file>` | Path to a YAML configuration file to bypass the interactive UI. |
| `--port <num>` | Port number for the server binary. |
| `--model <path>` | Path to the model directory. |
| `--context <num>` | Context size (must be a power of 2). |
| `--kv-key <type>` | Key quantization type (f32, f16, bf16, q8_0, q4_0, q4_1, iq4_nl, q5_0, q5_1). |
| `--kv-value <type>` | Value quantization type (same options). |
| `--gpu-layers <num>` | Number of GPU layers (0-40). |
| `--agent` / `--no-agent` | Enable or disable agent mode. |

### Interactive Flow

1. **Select Binary** – Choose between `llama-server`, `llama-swap`, or a saved favorite.
2. **Select Model** – Pick a model from the detected list.
3. **Context Size** – Set the context window (power of 2).
4. **KV Key Quant** – Choose key quantization format.
5. **KV Value Quant** – Choose value quantization format.
6. **GPU Layers** – Set number of GPU layers (0-40).
7. **Agent Mode** – Enable or disable agent mode.
8. **Run** – The command is built and executed.

When `llama-swap` is selected, steps 2-7 are skipped and the command runs immediately.

### Bypass TUI

Specify both `--bin` and `--config` to build and run the command directly without the interactive TUI:

```bash
./build/launch-llama --bin llama-server --config my-config.yaml
```

## Configuration Files

Two YAML files in the `include/` directory provide default settings:

- `include/config.yaml` – Terminal color configuration.
- `include/favorites.yaml` – Pre-configured launch options (favorites appear in the main menu).

You can modify these files or point to a custom config file using the `--config` option.

### include/favorites.yaml format

```yaml
favorites:
  my-preset:
    binary: llama-server
    model: my-model
    context: 8192
    kv_key: f16
    kv_value: f16
    gpu_layers: 35
    agent: false
    port: 8080
```

The `binary` field selects which binary to launch. Selecting a favorite from the menu applies all its settings and runs the command.

## Tests

```bash
make test
```

A simple integration test is provided in `tests/integration_test.cpp`.

## License

MIT
