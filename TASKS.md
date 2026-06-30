# Implementation Tasks — `launch-llama`

## Phase 1: Project Scaffolding & Configuration

- [ ] **Task 1.1: Initialize Python project**
  - Create `pyproject.toml` with project metadata (name, version, dependencies)
  - Set up virtual environment (`venv` or `uv`)
  - Create entry point script (`launch-llama`) pointing to a `main()` function
  - Create initial directory structure:
    ```
    launch-llama/
    ├── pyproject.toml
    ├── launch_llama/
    │   ├── __init__.py
    │   ├── main.py              # entry point
    │   ├── cli.py               # argument parsing
    │   ├── config.py            # config file loading
    │   ├── models.py            # model discovery
    │   ├── prompts.py           # interactive TUI prompts
    │   ├── builder.py           # command builder
    │   └── runner.py            # command execution
    ├── config.yaml              # global config (port number, etc.)
    ├── favorites.yaml           # saved favorites
    ├── models/                  # model files directory
    ├── include/
    │   ├── default.yaml         # defaults (port for llama-swap)
    │   └── llama-swap-config.yaml
    └── PROGRAM_FLOW.md
    ```

- [ ] **Task 1.2: Configuration file loading (`config.py`)**
  - Implement loading of `config.yaml` — extract `port` number (and any future global settings)
  - Implement loading of `favorites.yaml` — parse named favorite entries with full parameter sets
  - Implement loading of `include/default.yaml` — extract port for llama-swap mode
  - Handle missing files gracefully (provide sensible defaults or error messages)
  - Validate port numbers are integers in the valid range

## Phase 2: Core Logic

- [ ] **Task 2.1: CLI argument parsing (`cli.py`)**
  - Parse subcommands:
    - No arguments → enter interactive mode (show menu)
    - `run llama-server` → enter interactive model selection flow
    - `run <favorite_name>` → look up in favorites.yaml, build command directly
    - `run llama-swap` → build and execute the predefined llama-swap command
  - Use `argparse` (stdlib) or a library like `click`/`typer`
  - Show help text when `--help` is passed

- [ ] **Task 2.2: Model discovery (`models.py`)**
  - Scan `models/` directory for model files (filter by common extensions: `.gguf`, `.bin`, `.safetensors`, etc.)
  - Return sorted list of filenames (or full paths)
  - Handle empty models directory (show helpful error)
  - Support models in subdirectories (optional)

- [ ] **Task 2.3: Launcher menu screen (no args mode)**
  - Display a selection list:
    1. `llama-server` — proceeds to full interactive flow
    2. `favorite_1` — loads from favorites.yaml
    3. `favorite_2` — loads from favorites.yaml
    4. ... (dynamically list all favorites)
    5. `llama-swap` — runs directly
  - Use a TUI library: `questionary`, `pick`, `inquirer`, or build with `rich`

- [ ] **Task 2.4: Model selection screen**
  - Display all discovered models as a list
  - User picks one → store selected model path

- [ ] **Task 2.5: Context size prompt**
  - Ask user for context size (numeric input)
  - Default value: `4096` (or configurable)
  - Validate input is a positive integer

- [ ] **Task 2.6: Key quantization selection**
  - Present a list of options: `f32`, `f16`, `bf16`, `q8_0`, `q4_0`, `q4_1`, `iq4_nl`, `q5_0`, `q5_1`
  - Default: `q8_0` (or configurable)
  - Store selected value as `-ctk <value>`

- [ ] **Task 2.7: Value quantization selection**
  - Same options as key quantization
  - Default: `q8_0` (or configurable)
  - Store selected value as `-ctv <value>`

- [ ] **Task 2.8: GPU layers prompt**
  - Ask for number of GPU layers (integer 0–40)
  - Default: `20` (or configurable)
  - Validate range

- [ ] **Task 2.9: Agent mode toggle**
  - Yes/No question
  - If yes → append `--agent` flag
  - If no → append `--no-agent` flag

- [ ] **Task 2.10: Command builder (`builder.py`)**
  - Assemble the final command string:
    ```
    llama-server --model <model_path> -ctk <key_quant> -ctv <value_quant> -fa on --port <port> -c <context_size> [--agent|--no-agent]
    ```
  - The port number comes from `config.yaml`
  - For favorites: all values pulled from the YAML entry, no prompting needed
  - For llama-swap: build `llama-swap -config include/llama-swap-config.yaml -listen localhost:<port>`

- [ ] **Task 2.11: Command execution (`runner.py`)**
  - Use `subprocess.Popen` (or `asyncio.create_subprocess_exec`) to run the command
  - Stream stdout/stderr in real-time to the terminal
  - Handle `Ctrl+C` gracefully (kill child process, clean exit)
  - Return the exit code

## Phase 3: Integration & Polish

- [ ] **Task 3.1: Wire everything together in `main.py`**
  - Load config on startup
  - Parse CLI args → dispatch to correct flow
  - Interactive flow: chain prompts in order, then build & run
  - Favorites flow: extract from YAML, build & run
  - llama-swap flow: build & run directly

- [ ] **Task 3.2: Error handling**
  - Handle missing `config.yaml` (create a default one)
  - Handle missing `favorites.yaml` (treat as empty)
  - Handle model not found / empty models directory
  - Handle command not found (llama-server/llama-swap not in PATH)
  - Handle invalid YAML with clear error messages
  - Graceful shutdown on `KeyboardInterrupt`

- [ ] **Task 3.3: Configuration file generation**
  - On first run, generate default `config.yaml` with sensible defaults
  - Generate default `favorites.yaml` with example entries
  - Generate default `include/default.yaml` if missing

- [ ] **Task 3.4: Polish & UX improvements**
  - Colorized output (using `rich` or ANSI codes)
  - Loading spinners for long operations
  - Show the final command before executing (user confirmation?)
  - Log file or verbose mode for debugging

## Phase 4: Packaging & Distribution

- [ ] **Task 4.1: Package the project**
  - Configure `pyproject.toml` with proper entry point
  - Make `launch-llama` installable via `pip install .`
  - List all dependencies (PyYAML, questionary/rich, etc.)

- [ ] **Task 4.2: Documentation**
  - Write README with usage examples
  - Document the YAML config file schemas
  - Document how to set up favorites
