# setup virtual environment with following commands
#  virtualenv venv --python=C:\Python312\python.exe
#  venv\Scripts\activate
#  pip install transformers==4.51.3 torch==2.7.0 onnx==1.17.0
from transformers import AutoTokenizer, AutoModel
from pathlib import Path
import torch
import onnx

def convert_model_to_onnx(model_name, output_dir):
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # Load and save tokenizer and model
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModel.from_pretrained(model_name)
    tokenizer.save_pretrained(output_path)
    model.save_pretrained(output_path)

    # Set model to inference mode
    model.eval()

    # Dummy input for tracing
    noop = tokenizer("This is a dummy input", return_tensors="pt")

    # Export to ONNX
    torch.onnx.export(
        model,
        (noop["input_ids"], noop["attention_mask"]),
        output_path / "model.onnx",
        input_names=["input_ids", "attention_mask"],
        output_names=["token_embeddings", "sentence_embedding"],
        dynamic_axes={
            "input_ids": {0: "batch_size", 1: "sequence_length"},
            "attention_mask": {0: "batch_size", 1: "sequence_length"},
            "token_embeddings": {0: "batch_size", 1: "sequence_length"},
            "sentence_embedding": {0: "batch_size"}
        },
        opset_version=17
    )

    print(f"✅ Converted {model_name} to ONNX and saved to {output_path}")

# Convert multiple models
models = [
    "sentence-transformers/all-MiniLM-L6-v2",
    "sentence-transformers/all-MiniLM-L12-v2",
    "sentence-transformers/paraphrase-MiniLM-L6-v2",
    "BAAI/bge-large-en-v1.5"
]
for model in models:
    output_dir = f"onnx_models/{model.replace('/', '_')}"
    try:
        convert_model_to_onnx(model, output_dir)
    except Exception as e:
        print(f"❌ Failed to convert {model}: {e}")
