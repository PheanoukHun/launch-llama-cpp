import subprocess
import signal
import sys
import os
import time
import threading
import shutil


class Spinner:
    def __init__(self, message="Running", stream=sys.stderr):
        self.message = message
        self.stream = stream
        self._running = False
        self._thread = None

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *args):
        self.stop()

    def start(self):
        self._running = True
        self._thread = threading.Thread(target=self._spin, daemon=True)
        self._thread.start()

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=0.5)
        self.stream.write("\r" + " " * 60 + "\r")
        self.stream.flush()

    def _spin(self):
        chars = "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
        i = 0
        while self._running:
            self.stream.write(f"\r{chars[i % len(chars)]} {self.message}...")
            self.stream.flush()
            time.sleep(0.1)
            i += 1


class Runner:
    def __init__(self, verbose=False):
        self._child = None
        self.verbose = verbose

    def _check_command(self, cmd):
        """Check if the first command exists in PATH."""
        first_word = cmd.strip().split()[0]
        found = shutil.which(first_word)
        if not found:
            print(f"Error: '{first_word}' not found in PATH.")
            print(f"Please install {first_word} and ensure it's accessible.")
            if self.verbose:
                print(f"  PATH={os.environ.get('PATH', '')}")
            return False
        if self.verbose:
            print(f"[runner] Found '{first_word}' at: {found}")
        return True

    def run(self, cmd):
        """Execute command and stream output.

        Args:
            cmd: Command string to execute

        Returns:
            Exit code (0 on success, 130 on Ctrl+C)
        """
        if not self._check_command(cmd):
            return 127

        if self.verbose:
            print(f"[runner] Starting: {cmd}")

        self._child = subprocess.Popen(
            cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        try:
            with Spinner("Running") as spinner:
                for line in self._child.stdout:
                    if line:
                        spinner.stop()
                        print(line, end='')
                        spinner.start()
            self._child.wait()
            if self.verbose:
                print(f"[runner] Exit code: {self._child.returncode}")
            return self._child.returncode
        except KeyboardInterrupt:
            print("\nReceived Ctrl+C, terminating child process...")
            self._child.terminate()
            try:
                self._child.wait(timeout=3)
            except subprocess.TimeoutExpired:
                print("Force killing child process...")
                self._child.kill()
                self._child.wait()
            return 130
