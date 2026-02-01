### AI USE: Grok generated based on Kazuki's inspectAI.md
# HELM Controller Integration Guide

This document describes the input/output maps and execution interfaces for the **HELM** framework (Holistic Evaluation of Language Models) from Stanford CRFM, treating it as a black-box component for integration into a Controller system.

## 1. High-Level Flow

HELM operates on a scenario-based evaluation pipeline:

1. **Run Specification:** A `RunSpec` defines which scenarios, models, and metrics to evaluate.
2. **Instance Generation:** Each scenario produces a list of `Instance` objects (evaluation cases).
3. **Model Execution:** For each instance, the framework queries the specified model (or model deployment) to produce a `Completion` or sequence of tokens.
4. **Metric Computation:** Model outputs are scored by one or more `Metric` objects → produce `Stat` values (numbers + metadata).
5. **Aggregation & Reporting:** Statistics are aggregated across instances → final metrics per scenario/model. Results are written to JSON files and can be visualized via a web server.

## 2. Input Map

The primary unit of work is the **Instance**.

### Data Structures

#### `helm.core.instance.Instance`
A single evaluation example.

| Field          | Type                  | Description |
|----------------|-----------------------|-------------|
| `input`        | `InstanceInput`       | Usually contains `references` and prompt text/context. |
| `references`   | `list[Reference]`     | Ground truth(s): correct answer(s), ideal completion(s), etc. |
| `id`           | `str`                 | Unique identifier within the scenario. |
| `split`        | `str`                 | Usually `"train"`, `"valid"`, `"test"`. |
| `metrics`      | `list[str]`           | Names of metrics that should be computed for this instance (optional). |
| `metadata`     | `dict[str, Any]`      | Arbitrary metadata (source, tags, difficulty, etc.). |

#### `helm.core.scenario.Scenario`
Generates `Instance`s. Configured via name + parameters (e.g. `mmlu:subject=college_biology`).

## 3. Execution Interface

There are two primary ways to invoke HELM: via the CLI (`helm-run`) or Python API.

### Python API (Recommended for Controller)

**Entry Point:** `helm.benchmark.run.run_benchmark`

```python
from helm.benchmark.run import run_benchmark
from helm.benchmark.presentation.summarize import summarize_benchmark

# Example: run a small evaluation
run_benchmark(
    run_specs=["mmlu:subject=philosophy,model=openai/gpt-4o-mini"],
    suite="my-controller-suite",
    max_eval_instances=20,
    num_threads=4,
    mongo_uri=None,               # optional
    url=None,
    local_path="runs/my-run",
    exit_on_error=False
)
