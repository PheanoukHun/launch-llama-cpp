import os
import re

class Config:
    def __init__(self):
        self.port = 8080
        self.favorites = {}
        self.default_port = 8081

    def _parse_port(self, port_str):
        """Parse a port string, handling YAML-style 'port: 8080'."""
        match = re.search(r'^\s*port\s*:\s*(\d+)\s*$', port_str, re.IGNORECASE)
        if match:
            return int(match.group(1))
        return None

    def load(self, config_path, favorites_path, defaults_path):
        """Load configuration files using pure Python.
        
        Args:
            config_path: Path to config.yaml
            favorites_path: Path to favorites.yaml
            defaults_path: Path to include/default.yaml
        """
        if os.path.exists(config_path):
            with open(config_path, 'r') as f:
                content = f.read()
                port = self._parse_port(content)
                if port is not None and 1 <= port <= 65535:
                    self.port = port
                else:
                    print(f"Warning: Invalid port in {config_path}, using default 8080")
        else:
            print(f"Warning: {config_path} not found, using default port 8080")

        if os.path.exists(favorites_path):
            with open(favorites_path, 'r') as f:
                content = f.read()
                self.favorites = self._parse_favorites(content)
        else:
            print(f"Info: {favorites_path} not found, using empty favorites")

        if os.path.exists(defaults_path):
            with open(defaults_path, 'r') as f:
                content = f.read()
                port = self._parse_port(content)
                if port is not None and 1 <= port <= 65535:
                    self.default_port = port

    def _parse_favorites(self, content):
        """Simple YAML parser for favorites."""
        favorites = {}
        current = None
        for line in content.split('\n'):
            stripped = line.strip()
            if stripped.endswith(':') and not stripped.startswith('#'):
                current = stripped[:-1].strip()
                favorites[current] = {}
            elif ':' in line and current:
                key, val = line.split(':', 1)
                favorites[current][key.strip()] = val.strip()
        return favorites
