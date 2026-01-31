# Inspect AI Controller Integration Guide

This document describes the input/output maps and execution interfaces for the `inspect_ai` framework, treating it as a black-box component for integration into a Controller system.

## 1. High-Level Flow

The `inspect_ai` framework operates on a pipeline model:

1.  **Task Loading:** A `Task` is defined (typically via a decorated Python function) which constructs a `Dataset`.
2.  **Dataset Iteration:** The framework iterates over `Sample` objects from the `Dataset`.
3.  **Solver Execution:** Each `Sample` is processed by a chain of `Solvers` (agents, prompt engineering, etc.). Solvers interact with a `Model` to generate an `Output`.
4.  **Scoring:** The `Output` and `Target` are passed to a `Scorer` to produce a `Score`.
5.  **Logging:** All inputs, outputs, interactions (events), and scores are aggregated into an `EvalLog` object and persisted (typically as a JSON or binary `.eval` file).

## 2. Input Map 

The primary unit of work is the **Sample**. A Dataset is simply a sequence of Samples.

### Data Structures

#### `inspect_ai.dataset.Sample`
The core data structure representing a single evaluation case.

| Field | Type | Description |
| :--- | :--- | :--- |
| `input` | `str` \| `list[ChatMessage]` | The input prompt or message history to be submitted to the model. |
| `target` | `str` \| `list[str]` | The ideal target output (ground truth). Used by scorers. |
| `id` | `int` \| `str` | Unique identifier for the sample within the dataset. |
| `metadata` | `dict[str, Any]` | Arbitrary metadata (e.g., source urls, difficulty labels). |
| `choices` | `list[str]` | Optional list of answer choices (for multiple-choice tasks). |
| `sandbox` | `SandboxEnvironmentSpec` | Optional sandbox environment config (e.g., Docker container). |
| `files` | `dict[str, str]` | Files to be copied to the sandbox environment. |

#### `inspect_ai.dataset.Dataset`
An abstract sequence of `Sample` objects. It must implement `__getitem__` and `__len__`.

## 3. Execution Interface

There are two primary ways to invoke the framework: via the Python API or the CLI.

### Python API (Recommended for Controller)

**Entry Point:** `inspect_ai.eval`

```python
from inspect_ai import eval

logs = eval(
    tasks="path/to/task.py",  # or Task object
    model="openai/gpt-4",
    limit=10,
    log_dir="./logs",
    log_format="json"
)
```

**Key Arguments:**

| Argument | Type | Description |
| :--- | :--- | :--- |
| `tasks` | `str`, `Task`, `list` | The task(s) to run. Can be a file path, directory, or `Task` object. |
| `model` | `str` \| `Model` | The model to evaluate (e.g., `openai/gpt-4`, `anthropic/claude-3-opus`). |
| `model_args` | `dict` | Configuration for the model (temperature, max_tokens, etc.). |
| `task_args` | `dict` | Arguments passed to the task function itself (for dynamic task configuration). |
| `limit` | `int` | Limit the number of samples to evaluate (useful for testing). |
| `epochs` | `int` | Number of times to repeat the dataset. |
| `log_dir` | `str` | Directory where output logs will be saved. |
| `log_format` | `"eval"`, `"json"` | Output format. `json` is recommended for programmatic consumption. |

### CLI Interface

**Entry Point:** `inspect eval`

```bash
inspect eval <task_path> --model <model_name> [options]
```

**Key Flags:**
*   `--limit <n>`: Run only N samples.
*   `--epochs <n>`: Run dataset N times.
*   `--model-args <k=v>`: Pass arguments to the model.
*   `--task-args <k=v>`: Pass arguments to the task.
*   `--log-format json`: Output logs in JSON format.

## 4. Output Map 

The result of an execution is an `EvalLog` object. When serialized to JSON, it follows a consistent schema.

### Results Location
*   **Python:** Returned as a list of `EvalLog` objects from `eval()`.
*   **File System:** Saved to `log_dir` (default: `./logs`) with a timestamped filename.

### JSON Output Schema (Simplified)

```json
{
  "status": "success",
  "eval": {
    "task": "task_name",
    "task_id": "unique_task_id",
    "model": "openai/gpt-4",
    "dataset": { "name": "dataset_name", "samples": 100 },
    "config": { ... }
  },
  "results": {
    "total_samples": 100,
    "completed_samples": 100,
    "scores": [
      {
        "name": "accuracy",
        "metrics": {
          "accuracy": { "name": "accuracy", "value": 0.85 }
        }
      }
    ]
  },
  "samples": [
    {
      "id": 1,
      "input": "Question text...",
      "target": "Answer text",
      "output": {
        "text": "Model generated answer",
        "stop_reason": "stop"
      },
      "scores": {
        "accuracy": {
          "value": 1.0,
          "explanation": "Correct answer."
        }
      },
      "error": null
    }
  ],
  "stats": {
    "started_at": "2023-10-27T10:00:00+00:00",
    "completed_at": "2023-10-27T10:05:00+00:00",
    "model_usage": { ... }
  }
}
```

### Key Fields for Parsing
*   `status`: Check this first. Should be `"success"`.
*   `results.scores`: Contains aggregate metrics (e.g., overall accuracy).
*   `samples`: List of detailed results for each input.
    *   `samples[i].input`: The input prompt.
    *   `samples[i].output.text`: The model's raw response.
    *   `samples[i].scores`: The score assigned to this specific sample.

## 5. Standardization Analysis

*   **Strictly Standard (Static):**
    *   The `eval()` function signature and CLI arguments are stable.
    *   The `EvalLog` / JSON output structure is strictly typed (Pydantic models) and consistent.
    *   The `Sample` object structure (input/target/id) is consistent across all tasks.

*   **Dynamic (Variable):**
    *   **Task Arguments:** Every task can accept arbitrary arguments (`task_args`). The Controller needs to know these per-task.
    *   **Metadata:** The contents of `metadata` fields in Samples and Logs are unstructured dictionaries.
    *   **Scoring Logic:** While the *structure* of a score is standard (value/explanation), the *meaning* (0-1 vs 0-100) depends on the specific scorer used.
