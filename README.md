# Launch‑Llama

## Overview

Launch‑Llama is a TUI application written in **C++20** that launches either `llama‑swap` or `llama‑server` with configurable options. It uses the following third‑party libraries:

- **termOX** – terminal UI
- **tclap** – command‑line argument parsing
- **libev** – non‑blocking I/O for model listing
- **curl** – fetch model metadata
- **yaml** – configuration parsing (pyyaml)

The project is organized into a single source tree and can be built on any Linux distribution that provides the required dependencies.

## Cloning the Repository

```bash
git clone https://github.com/your-org/launch-llama-cpp.git
cd launch-llama-cpp
```

## Installing Dependencies

The following commands install the necessary libraries on common distributions. Adjust as needed for your package manager.

### Debian/Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libnotcurses-dev libtclap-dev libev-dev libcurl4-openssl-dev libyaml-dev
```

### Fedora / CentOS

```bash
sudo dnf install -y gcc-c++ cmake notcurses-devel tclap-devel libev-devel curl-devel yaml-devel
```

### Arch Linux

```bash
sudo pacman -Syu --needed base-devel notcurses tclap libev curl yaml
```

> **Note**: If a package for a library is not available, you can build it from source. The README in the library’s repository contains build instructions.

## Building the Project

```bash
mkdir build && cd build
make
```

The `Makefile` in the repository is configured to use `CXXFLAGS` for C++20 and links against the required libraries. The resulting binary `launch-llama` is placed in the `build` directory.

### Custom Build Options

If you installed libraries in a non‑standard location, adjust the Makefile:

```make
CXXFLAGS += -I/opt/notcurses/include
LDFLAGS  += -L/opt/notcurses/lib
```

Alternatively, export `CXXFLAGS` and `LDFLAGS` before invoking `make`.

## Running the Application

```bash
./launch-llama [options]
```

### Common Options

| Option | Description |
|--------|-------------|
| `--binary <name>` | Specify `llama-swap` or `llama-server` to skip the binary selection menu.
| `--config <file>` | Path to a YAML configuration file to bypass the interactive UI.
| `--port <num>` | Port number for the server binary.
| `--model <path>` | Path to the model directory.
| `--context <num>` | Context size (must be a power of 2).
| `--kv-key <type>` | Key quantization type.
| `--kv-value <type>` | Value quantization type.
| `--gpu-layers <num>` | Number of GPU layers (0‑40).
| `--agent` / `--no-agent` | Enable or disable agent mode.

## Configuration Files

Two YAML files are used for default settings:

- `include/config.yaml` – Terminal color configuration.
- `include/favorites.yaml` – Pre‑configured launch options for `llama-swap` and `llama-server`.

You can modify these files or point to a custom config file using the `--config` option.

## Tests

A simple integration test is provided in `tests/integration_test.cpp`. Run it with `make test` after building.

## License

MIT

---

For any questions or issues, open an issue on the repository or contact the maintainer.

