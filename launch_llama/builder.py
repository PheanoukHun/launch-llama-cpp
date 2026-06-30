def parse_agent_mode(val):
    """Parse agent_mode from various types (bool, str, int) to bool."""
    if isinstance(val, bool):
        return val
    return str(val).lower() in ('true', 'yes', '1')


def build_llama_server_command(
    model_path,
    key_quant,
    value_quant,
    port,
    context_size,
    gpu_layers,
    agent_mode,
    llama_server_path="llama-server"
):
    """Build llama-server command string.
    
    Args:
        model_path: Path to model file
        key_quant: Key quantization type
        value_quant: Value quantization type
        port: Port number
        context_size: Context window size
        gpu_layers: Number of GPU layers
        agent_mode: True for --agent, False for --no-agent
        llama_server_path: Path to llama-server binary
        
    Returns:
        Formatted command string
    """
    cmd = [
        llama_server_path,
        f"--model {model_path}",
        f"-ctk {key_quant}",
        f"-ctv {value_quant}",
        "-fa on",
        f"--port {port}",
        f"-c {context_size}",
        f"-ngl {gpu_layers}"
    ]
    if parse_agent_mode(agent_mode):
        cmd.append("--agent")
    else:
        cmd.append("--no-agent")
    return " ".join(cmd)

def build_llama_swap_command(port, llama_swap_path="llama-swap", llama_swap_config="include/llama-swap-config.yaml"):
    """Build llama-swap command string.
    
    Args:
        port: Port number to listen on
        llama_swap_path: Path to llama-swap binary
        llama_swap_config: Path to llama-swap config file
        
    Returns:
        Formatted command string
    """
    return f"{llama_swap_path} -config {llama_swap_config} -listen localhost:{port}"
