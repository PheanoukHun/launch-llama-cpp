# Program Flow

## Entry Point

```
launch-llama [command] [args]
```

## Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         MAIN                                    │
├─────────────────────────────────────────────────────────────────┤
│  1. Load config.yaml → port                                     │
│  2. Load favorites.yaml → favorites list                        │
│  3. Discover models/ → model list                               │
│  4. Parse CLI args                                              │
└─────────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼
  ┌───────────┐      ┌─────────────┐    ┌──────────────┐
  │Interactive│      │ Favorites   │    │ llama-swap   │
  │   Menu    │      │    Flow     │    │   Flow       │
  └─────┬─────┘      └─────────────┘    └──────────────┘
        │                   │                   │
        │                   │                   │
        ▼                   ▼                   ▼
  ┌───────────┐      ┌─────────────┐    ┌──────────────┐
  │Select     │      │Load from    │    │Build         │
  │llama-     │      │favorites    │    │llama-swap    │
  │server     │      │yaml         │    │command       │
  └─────┬─────┘      └──────┬──────┘    └──────────────┘
        │                   │ 
        ▼                   ▼
  ┌───────────┐      ┌─────────────┐
  │Select     │      │Build        │
  │model      │      │command      │
  └─────┬─────┘      └─────────────┘
        │
        ▼
  ┌───────────┐
  │Context    │
  │size       │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Quant      │
  │key        │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Quant      │
  │value      │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │GPU        │
  │layers     │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Agent      │
  │mode       │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Build      │
  │command    │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Print      │
  │command    │
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │Run        │
  │command    │
  └───────────┘
```

## Detailed Steps

### Interactive Mode (no args)

1. Show menu with options:
   - `llama-server` → go to model selection
   - `favorite_1` → load from favorites
   - `favorite_2` → load from favorites
   - `llama-swap` → go to llama-swap flow

2. If `llama-server` selected:
   - Display model list from `models/`
   - User selects model → store path
   - Prompt context size (default: 4096)
   - Prompt key quantization (default: q8_0)
   - Prompt value quantization (default: q8_0)
   - Prompt GPU layers (default: 20, range 0-40)
   - Prompt agent mode (yes/no)
   - Build and run command

3. If favorite selected:
   - Load all values from favorites.yaml
   - Build and run command
   - (No prompting)

4. If `llama-swap` selected:
   - Get port (or use default from config)
   - Build llama-swap command
   - Print and run

### Favorites Flow (arg: run <name>)

1. Look up name in favorites.yaml
2. If not found → error
3. Extract all values
4. Build command
5. Print and run

### llama-swap Flow (arg: run llama-swap)

1. Get port (or use default)
2. Build command: `llama-swap -config config/llama-swap-config.yaml -listen localhost:<port>`
3. Print and run

## Command Formats

### llama-server

```
llama-server --model <path> -ctk <key> -ctv <value> -fa on --port <port> -c <ctx> -ngl <layers> [--agent|--no-agent]
```

### llama-swap

```
llama-swap -config config/llama-swap-config.yaml -listen localhost:<port>
```

## Error Handling

- Missing config.yaml → create default
- Missing favorites.yaml → treat as empty
- Missing models/ → error
- Invalid port → warning, use default
- Invalid YAML → clear error
- Command not found → error

## Signals

- `SIGINT` (Ctrl+C) → terminate child, exit with 130
