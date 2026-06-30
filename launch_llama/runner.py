import subprocess
import signal
import sys

class Runner:
    def __init__(self):
        self._child = None

    def run(self, cmd):
        """Execute command and stream output.
        
        Args:
            cmd: Command string to execute
            
        Returns:
            Exit code (0 on success, 130 on Ctrl+C)
        """
        self._child = subprocess.Popen(
            cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        try:
            for line in self._child.stdout:
                if line:
                    print(line, end='')
            self._child.wait()
            return self._child.returncode
        except KeyboardInterrupt:
            self._child.terminate()
            try:
                self._child.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self._child.kill()
                self._child.wait()
            return 130
