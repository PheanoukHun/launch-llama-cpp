The program acts as a launcher for different `llama.cpp`-related binaries. Depending on the command-line arguments, it either walks the user through an interactive setup or immediately launches the requested binary with a configuration file.

### 1. Program starts

The user runs the `launch-llama` executable.

There are three possible entry points:

* `launch-llama`
* `launch-llama --bin llama-server`
* `launch-llama --bin llama-swap`

There are also configuration-based shortcuts:

* `launch-llama --bin llama-server --config some-config.yaml`
* `launch-llama --bin llama-swap --config some-other-config.yaml`

---

### 2. Running without `--bin`

If the user simply runs:

```bash
launch-llama
```

the program displays a launch menu containing available binaries:

1. `llama-server`
2. `llama-swap`
3. Any additional favorites defined in `favorites.yaml`

The user then chooses which binary to launch.

---

### 3. If `llama-server` is selected

The launcher starts an interactive configuration wizard.

It asks the user, in order:

1. Which model to load.
2. What context size to use.
3. Which KV cache key quantization format (`-ctk`) to use.
4. Which KV cache value quantization format (`-ctv`) to use.
5. How many layers should be offloaded to the GPU.
6. Whether Agent mode should be enabled (`--agent`) or disabled (`--no-agent`).

After collecting all of these options, the launcher constructs a command similar to:

```bash
llama-server \
  --model <model> \
  -ctk <key_quant> \
  -ctv <value_quant> \
  -fa on \
  --port 8080 \
  -c <context_size> \
  [--agent | --no-agent]
```

The completed command is then executed.

---

### 4. If `llama-swap` is selected

Instead of asking additional questions, the launcher immediately builds the command:

```bash
llama-swap \
  -config llama-swap-config.yaml \
  -listen localhost:9000
```

It then runs the command.

---

### 5. Using `--bin`

If the user specifies a binary on the command line, the initial selection menu is skipped.

For example:

```bash
launch-llama --bin llama-server
```

starts directly at the interactive `llama-server` configuration wizard.

Likewise,

```bash
launch-llama --bin llama-swap
```

immediately constructs and runs the `llama-swap` command.

---

### 6. Using `--config`

If a configuration file is also provided, the launcher skips the interactive prompts entirely.

For example,

```bash
launch-llama --bin llama-server --config some-config.yaml
```

loads the settings from `some-config.yaml`, builds the `llama-server` command automatically, and executes it.

Similarly,

```bash
launch-llama --bin llama-swap --config some-other-config.yaml
```

uses the supplied configuration file to launch `llama-swap` without any user interaction.

---

### Overall flow

In summary, the launcher follows one of three paths:

* **No arguments:** Show a binary selection screen, then either run the interactive `llama-server` setup or launch `llama-swap`.
* **`--bin` specified:** Skip the selection screen and go directly to the chosen binary.
* **`--bin` + `--config`:** Skip all interactive prompts, load settings from the configuration file, construct the appropriate command, and execute it immediately.
