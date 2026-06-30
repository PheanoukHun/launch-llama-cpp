import os

VALID_EXTENSIONS = {'.gguf', '.bin', '.safetensors', '.ggml', '.onnx'}

def discover_models(models_dir):
    """Scan models directory for model files.
    
    Args:
        models_dir: Path to models directory
        
    Returns:
        Sorted list of relative filenames
    """
    if not os.path.isdir(models_dir):
        return []
    
    models = []
    for root, _, files in os.walk(models_dir):
        for f in files:
            ext = os.path.splitext(f)[-1].lower()
            if ext in VALID_EXTENSIONS:
                rel = os.path.relpath(os.path.join(root, f), models_dir)
                models.append(rel)
    return sorted(models)

def validate_model_path(path):
    """Check if model path exists."""
    return os.path.isfile(path)
