# Design Document - Launch-Llama

## Overview

A TUI application written in C++ that launches llama-swap or llama-server with configurable options.

## Architecture

```
launch-llama/
├── src/
│   ├── main.cpp
│   ├── launcher.cpp / launcher.hpp
│   ├── screens/
│   │   ├── model_list.cpp / model_list.hpp
│   │   ├── kv_quant.cpp / kv_quant.hpp
│   │   ├── gpu_layers.cpp / gpu_layers.hpp
│   │   ├── agent_mode.cpp / agent_mode.hpp
│   │   └── run_cmd.cpp / run_cmd.hpp
│   ├── parser.cpp / parser.hpp
│   ├── curses_util.cpp / curses_util.hpp
│   └── config_loader.cpp / config_loader.hpp
├── include/
│   └── config.yaml
├── build/
│   ├── Makefile
│   └── libtclap.a
└── tests/
    └── test_configs/
```

### Components

| Component | Responsibility |
|-----------|----------------|
| `launcher` | Core event loop, screen transitions, state machine |
| `model_list` | Lists LLM-Models/LLM/, search via `/`, highlights current |
| `kv_quant` | Selects key/value quantization (f32, f16, bf16, q8_0, q4_0, q4_1, iq4_nl, q5_0, q5_1) |
| `gpu_layers` | Range 0-40, up/down arrows adjust, default 20 |
| `agent_mode` | Yes/No selection |
| `run_cmd` | Spawns command in child, captures stdout/stderr, keeps focus |
| `parser` | Handles CLI args: binary, `--config <file>`, etc. |
| `curses_util` | Color definitions, box drawing, centered widgets |
| `config_loader` | Parses color.yaml and favorites.yaml |

## Dependencies

| Library | Purpose |
|---------|---------|
| `notcurses` | Terminal UI |
| `tclap` | CLI argument parsing |
| `curl` | Fetch model metadata |
| `yaml` (pyyaml) | Config file parsing |
| `libev` | Non-blocking I/O for model listing |

## Build

```bash
mkdir build && cd build
make
```

Makefile:
- `CXXFLAGS` includes `-I/usr/include/tclap`
- `LDFLAGS` links `-ltclap -lyaml -lnotcurses -lcurl -lstdc++`
- Generates `launch-llama` binary

## Configuration

### color.yaml
```yaml
foreground: default
background: default
selected: cyan
highlight: yellow
```

### favorites.yaml
```yaml
llama-swap:
  model: Llama-3-70B-Instruct
  kv_key: q8_0
  kv_value: q4_0
  gpu_layers: 20
  agent: false
llama-server:
  model: Mistral-7B-Instruct-v0.2
  kv_key: f16
  kv_value: f32
  gpu_layers: 32
  agent: true
```

## CLI Interface

```bash
launch-llama [options]

Options:
  --config <file>    Path to yaml config (skips interactive)
  --binary <name>    llama-swap | llama-server
  --port <num>       Port number
  --model <path>     Model path
  --context <num>    Context size (powers of 2)
  --kv-key <type>    Key quantization
  --kv-value <type>  Value quantization
  --gpu-layers <num> GPU layers (0-40)
  --agent            Enable agent mode
  --no-agent         Disable agent mode
```

## Screen Flow

1. **Main Menu**
   - Text box with two options: llama-swap, llama-server
   - If `--binary` given, auto-select that option
   - If `--config` given, skip to run command screen

2. **Model List** (llama-server only)
   - Vertical list of models from `/home/procastoh/AI/LLM-Models/LLM/`
   - Highlight first model by default
   - `/` enables search; type model name substring
   - Press `Enter` to select

3. **KV Quantization**
   - Two horizontal boxes: Key Quant, Value Quant
   - Same options for both
   - Press `Enter` to proceed

4. **GPU Layers**
   - Range slider visual (text-based)
   - Up arrow increments, down decrements
   - Default: 20
   - Enter to proceed

5. **Agent Mode**
   - Two options: Yes, No
   - Enter to select, Esc to go back

6. **Run Command**
   - Displays full command in child window
   - Spawns with `popen()` or fork+exec
   - Logs output in real-time
   - Press Enter to close and return to menu

## Error Handling

- Invalid path → print error, exit
- Missing binary → print error, exit
- YAML parse failure → print error, exit
- Any other issue → exit gracefully with message
