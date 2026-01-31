# Harness Controller Integration Guide

This document describes the input/output maps and execution interfaces for the [EleutherAI LM Evaluation Harness](https://github.com/EleutherAI/lm-evaluation-harness), treating it as a black-box component for integration into a Controller system.

## 1. High-Level Flow

The harness operates as a modular evaluation pipeline:

1.  **Task Registry**: It resolves task names (strings) into `Task` objects via a `TaskManager`. This handles benchmarking logic, dataset downloading, and prompt formatting.
2.  **Model Adapter**: It wraps various backends (HuggingFace, OpenAI, vLLM, GGUF) into a unified `LM` (Language Model) interface.
3.  **Evaluator**: The core loop (`simple_evaluate`) orchestrates the process:
    *   Batches inputs from selected tasks.
    *   Queries the `LM` for log-likelihoods or text generation.
    *   Computes metrics (Accuracy, Perplexity, Exact Match, etc.).
4.  **Aggregation**: Results are aggregated per task and (optional) group, producing a final dictionary of metrics and metadata.

## 2. Input Map

The primary inputs are the **Task Specification**, **Model Definition**, and **Runtime Parameters**.

### Task Specification
Tasks are identified by string keys in the registry.

*   **Standard Tasks**: Simple strings, e.g., `"hellaswag"`, `"arc_challenge"`, `"gsm8k"`.
*   **Grouped Tasks**: Benchmarks that aggregate multiple sub-tasks, e.g., `"mmlu"`.
*   **Configurable Tasks**: Some tasks accept parameters, though standard usage often relies on the registry defaults.

### Model Arguments
The harness uses a provider string and an arguments string to instantiate models.

*   **Model Provider** (`model`): A string identifying the backend family.
    *   `"hf"`: HuggingFace (local or Hub).
    *   `"openai-chat"` / `"openai-completions"`: OpenAI API.
    *   `"vllm"`: vLLM inference server.
    *   `"gguf"`: Local GGUF/llama.cpp models.
*   **Model Arguments** (`model_args`): A comma-separated string or dictionary of key-value pairs.
    *   Example: `"pretrained=meta-llama/Llama-2-7b-hf,dtype=bfloat16,trust_remote_code=True"`

### Key Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `tasks` | `list[str]` | List of task names to evaluate. |
| `num_fewshot` | `int` | Number of few-shot examples to prepend to the prompt. |
| `batch_size` | `int` or `"auto"` | Batch size for inference. "auto" attempts to maximize GPU usage. |
| `limit` | `int` or `float` | Limit the number of examples per task (useful for debugging). |
| `gen_kwargs` | `dict` | Generation parameters (temperature, top_p) for generative tasks. |
| `apply_chat_template` | `bool` | Whether to format prompts using the model's chat template. |

## 3. Execution Interface

There are two primary ways to invoke the harness: via the CLI or the Python API.

### Python API (Primary)
The main entry point is `lm_eval.simple_evaluate`. This function handles the entire pipeline and returns a result dictionary.

```python
import lm_eval
from lm_eval.models.huggingface import HFLM

# 1. Define Model
# Option A: Initialize via string (Lazy loading)
model_name = "hf"
model_args = "pretrained=gpt2,dtype=float"

# Option B: Pre-initialize object (for advanced control)
# lm = HFLM(pretrained="gpt2", batch_size="auto")

# 2. Run Evaluation
results = lm_eval.simple_evaluate(
    model=model_name,
    model_args=model_args,
    tasks=["hellaswag", "arc_easy"],
    num_fewshot=0,
    batch_size="auto",
    limit=10,  # Run only 10 examples per task for testing
    log_samples=False,
)

# 3. Process Results
print(results["results"])
```

### CLI
The standard command-line interface matches the Python API arguments.

```bash
lm_eval --model hf \
    --model_args pretrained=gpt2,dtype=float \
    --tasks hellaswag,arc_easy \
    --device cuda:0 \
    --batch_size auto
```

## 4. Output Map

The evaluator returns a dictionary containing metrics, configuration, and optional per-sample logs.

### Structure

The output dictionary has the following top-level keys:

*   **`results`**: The core metrics. A dictionary where keys are task names and values are dictionaries of metrics.
*   **`groups`**: (Optional) Aggregated metrics for task groups (e.g., MMLU categories).
*   **`configs`**: The resolved configuration for each task run (useful for reproducibility).
*   **`samples`**: (If `log_samples=True`) A list of every individual input/output pair and its score.
*   **`n-shot`**: Mapping of task names to the number of few-shot examples used.
*   **`versions`**: Version strings for the tasks.

### Output JSON Schema (Simplified)

```json
{
  "results": {
    "hellaswag": {
      "acc,none": 0.25,
      "acc_norm,none": 0.28,
      "alias": "hellaswag"
    },
    "arc_easy": {
      "acc,none": 0.42,
      "acc_stderr,none": 0.01
    }
  },
  "configs": {
    "hellaswag": {
      "task": "hellaswag",
      "group": "multiple_choice",
      "dataset_path": "hellaswag",
      "training_split": "train",
      "validation_split": "validation",
      "test_split": "test",
      "num_fewshot": 0
    }
  },
  "n-shot": {
    "hellaswag": 0,
    "arc_easy": 0
  },
  "versions": {
    "hellaswag": "1.0",
    "arc_easy": "1.0"
  },
  "config": {
    "model": "gpt2",
    "model_args": "pretrained=gpt2",
    "batch_size": "auto",
    "device": "cuda"
  },
  "date": 1706688000.0
}
```
