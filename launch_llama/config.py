import os
import re
import sys

try:
    import yaml
    HAS_YAML = True
except ImportError:
    HAS_YAML = False

CONFIG_TEMPLATE = """port: 8080
models_dir: models
llama_server_path: llama-server
llama_swap_path: llama-swap
llama_swap_config: config/llama-swap-config.yaml
favorites: config/favorites.yaml
default_port_config: config/default.yaml
"""

FAVORITES_TEMPLATE = """# Favorites for launch-llama
# Each entry contains: model path, key quant, value quant, port, context size, GPU layers, agent mode

default:
  model: models/Qwen2.5-1.5B-Instruct-Q4_0.gguf
  key_quant: q4_0
  value_quant: q4_0
  port: 8080
  context_size: 2048
  gpu_layers: 16
  agent_mode: false

chatbot:
  model: models/Mistral-7B-Instruct-v0.3-Q8_0.gguf
  key_quant: q8_0
  value_quant: q8_0
  port: 8081
  context_size: 8192
  gpu_layers: 32
  agent_mode: true
"""

DEFAULT_YAML_TEMPLATE = """# Default configuration for llama-swap
# This is used when llama-swap mode is selected

port: 8081
"""


class ConfigError(Exception):
    pass


def check_command_exists(name):
    """Check if a command exists in PATH."""
    for path_dir in os.environ.get("PATH", "").split(os.pathsep):
        full = os.path.join(path_dir, name)
        if os.path.isfile(full) and os.access(full, os.X_OK):
            return True
    return False


def _parse_yaml_fields(content):
    """Parse simple 'key: value' lines from a YAML string into a dict."""
    fields = {}
    for line in content.split('\n'):
        stripped = line.strip()
        if ':' in stripped and not stripped.startswith('#'):
            key, _, value = stripped.partition(':')
            fields[key.strip()] = value.strip()
    return fields


def _ensure_fields(filepath, template):
    """Add any missing key-value fields from template to an existing file."""
    if not os.path.exists(filepath):
        return False
    with open(filepath, 'r') as f:
        existing = f.read()
    existing_fields = _parse_yaml_fields(existing)
    default_fields = _parse_yaml_fields(template)
    missing = {k: v for k, v in default_fields.items() if k not in existing_fields}
    if not missing:
        return False
    with open(filepath, 'a') as f:
        f.write('\n')
        for key, value in missing.items():
            f.write(f'{key}: {value}\n')
    return True


def generate_defaults(config_path):
    """Ensure config.yaml exists and has all default fields,
    adding any that are missing."""
    generated = []

    if os.path.exists(config_path):
        if _ensure_fields(config_path, CONFIG_TEMPLATE):
            generated.append(config_path)
            print(f"Added missing fields to config: {config_path}")
    else:
        with open(config_path, 'w') as f:
            f.write(CONFIG_TEMPLATE)
        generated.append(config_path)
        print(f"Created default config: {config_path}")

    return generated


class Config:
    def __init__(self):
        self.port = 8080
        self.favorites = {}
        self.default_port = 8081
        self.models_dir = "models"
        self.llama_server_path = "llama-server"
        self.llama_swap_path = "llama-swap"
        self.llama_swap_config = "config/llama-swap-config.yaml"
        self.favorites_path = "config/favorites.yaml"
        self.default_port_config_path = "config/default.yaml"
        self._base_dir = None

    def _validate_port(self, port):
        if not isinstance(port, int):
            return False
        return 1 <= port <= 65535

    def _parse_port_yaml(self, content):
        """Extract port from a simple YAML string (fallback when PyYAML unavailable)."""
        match = re.search(r'^\s*port\s*:\s*(\d+)\s*$', content, re.MULTILINE)
        if match:
            return int(match.group(1))
        return None

    def _parse_favorites_manual(self, content):
        """Simple parser for favorites YAML (fallback when PyYAML unavailable)."""
        favorites = {}
        current = None
        for line in content.split('\n'):
            stripped = line.strip()
            if not stripped or stripped.startswith('#'):
                continue
            if stripped.endswith(':') and not stripped.startswith('-'):
                current = stripped[:-1].strip()
                favorites[current] = {}
            elif ':' in line and current:
                key, val = line.split(':', 1)
                favorites[current][key.strip()] = val.strip()
        return favorites

    def _read_config_yaml(self, config_path):
        """Read config.yaml and return parsed data dict, or None on failure."""
        if not os.path.exists(config_path):
            return None
        try:
            with open(config_path, 'r') as f:
                content = f.read()
            if HAS_YAML:
                return yaml.safe_load(content) or {}
            else:
                return _parse_yaml_fields(content)
        except Exception as e:
            print(f"Warning: Error reading {config_path}: {e}")
            return None

    def _apply_config_fields(self, data):
        """Apply config fields from a parsed dict to self."""
        if HAS_YAML:
            self.models_dir = data.get("models_dir", self.models_dir)
            self.llama_server_path = data.get("llama_server_path", self.llama_server_path)
            self.llama_swap_path = data.get("llama_swap_path", self.llama_swap_path)
            self.llama_swap_config = data.get("llama_swap_config", self.llama_swap_config)
            self.favorites_path = data.get("favorites", self.favorites_path)
            self.default_port_config_path = data.get("default_port_config", self.default_port_config_path)
            port = data.get("port", self.port)
        else:
            self.models_dir = data.get("models_dir", self.models_dir)
            self.llama_server_path = data.get("llama_server_path", self.llama_server_path)
            self.llama_swap_path = data.get("llama_swap_path", self.llama_swap_path)
            self.llama_swap_config = data.get("llama_swap_config", self.llama_swap_config)
            self.favorites_path = data.get("favorites", self.favorites_path)
            self.default_port_config_path = data.get("default_port_config", self.default_port_config_path)
            raw = data.get("port", self.port)
            try:
                port = int(raw) if raw else self.port
            except (ValueError, TypeError):
                port = self.port
        if self._validate_port(port):
            self.port = port
        else:
            print(f"Warning: Invalid port in config, using default {self.port}")

    def _ensure_file(self, path, template, label):
        """Create a file from template if it doesn't exist."""
        parent = os.path.dirname(path)
        if parent and not os.path.exists(parent):
            os.makedirs(parent, exist_ok=True)
        if not os.path.exists(path):
            with open(path, 'w') as f:
                f.write(template)
            print(f"Created default {label}: {path}")

    def _resolve_path(self, path):
        """Resolve a relative path against the config base directory."""
        if self._base_dir and not os.path.isabs(path):
            return os.path.join(self._base_dir, path)
        return path

    def load(self, config_path):
        """Load configuration files.

        Config paths (favorites, default_port_config) are read from config.yaml
        itself, so all paths are configurable from a single file.

        Args:
            config_path: Path to config.yaml
        """
        config_path = os.path.abspath(config_path)
        self._base_dir = os.path.dirname(config_path)

        data = self._read_config_yaml(config_path)
        if data is not None:
            self._apply_config_fields(data)
        else:
            print(f"Warning: {config_path} not found, using defaults")

        self.models_dir = self._resolve_path(self.models_dir)
        self.favorites_path = self._resolve_path(self.favorites_path)
        self.default_port_config_path = self._resolve_path(self.default_port_config_path)
        self.llama_swap_config = self._resolve_path(self.llama_swap_config)

        self._ensure_file(self.favorites_path, FAVORITES_TEMPLATE, "favorites")
        self._ensure_file(self.default_port_config_path, DEFAULT_YAML_TEMPLATE, "default port config")

        if os.path.exists(self.favorites_path):
            try:
                with open(self.favorites_path, 'r') as f:
                    content = f.read()
                if HAS_YAML:
                    data = yaml.safe_load(content) or {}
                    self.favorites = data if isinstance(data, dict) else {}
                else:
                    self.favorites = self._parse_favorites_manual(content)
            except Exception as e:
                print(f"Warning: Error reading {self.favorites_path}: {e}")
                print(f"Warning: Using empty favorites")
                self.favorites = {}
        else:
            print(f"Info: {self.favorites_path} not found, using empty favorites")

        if os.path.exists(self.default_port_config_path):
            try:
                with open(self.default_port_config_path, 'r') as f:
                    content = f.read()
                if HAS_YAML:
                    data = yaml.safe_load(content) or {}
                    port = data.get("port", self.default_port)
                else:
                    port = self._parse_port_yaml(content) or self.default_port
                if self._validate_port(port):
                    self.default_port = port
            except Exception as e:
                print(f"Warning: Error reading {self.default_port_config_path}: {e}")
        else:
            print(f"Info: {self.default_port_config_path} not found, using default port {self.default_port}")
