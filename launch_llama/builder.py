def build_llama_server_command(
    model_path,
    key_quant,
    value_quant,
    port,
    context_size,
    gpu_layers,
    agent_mode
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
        
    Returns:
        Formatted command string
    """
    cmd = [
        "llama-server",
        f"--model {model_path}",
        f"-ctk {key_quant}",
        f"-ctv {value_quant}",
        "-fa on",
        f"--port {port}",
        f"-c {context_size}",
        f"-ngl {gpu_layers}"
    ]
    if agent_mode:
        cmd.append("--agent")
    else:
        cmd.append("--no-agent")
    return " ".join(cmd)

def build_llama_swap_command(port):
    """Build llama-swap command string.
    
    Args:
        port: Port number to listen on
        
    Returns:
        Formatted command string
    """
    return f"llama-swap -config include/llama-swap-config.yaml -listen localhost:{port}"
