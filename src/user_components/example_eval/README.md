# Example Q&A Eval

A minimal HELM evaluation component with hardcoded multiple-choice questions. No external dataset downloads required.

## What it does

Runs 10 multiple-choice Q&A pairs (general knowledge) against a model via HELM. The first 5 questions serve as few-shot examples; the remaining 8 are scored with exact-match metrics.

## Files

- **`example_scenario.py`** — `ExampleQAScenario`: HELM `Scenario` subclass with inline Q&A data.
- **`helm_example_run_specs.py`** — `@run_spec_function("example_qa")`: wires the scenario, adapter, and metrics into a HELM `RunSpec`.
- **`requirements.txt`** — empty; no dependencies beyond `crfm-helm`.

## How to run

```bash
cd src/sandbox
docker compose up --build
```

The eval sandbox connects to a LiteLLM proxy (which routes to Qwen/Qwen3-1.7B on HuggingFace) and writes results to the `eval_output` Docker volume at `/app/benchmark_output`.

## Network isolation

- `eval_sandbox` can only reach `llm_proxy` on the internal network.
- `llm_proxy` has external internet access to reach the HuggingFace inference API.
- `eval_sandbox` cannot reach the internet directly.

## Sandbox Output
- Retrieve all run artifacts (from `src/sandbox`): `docker compose cp eval_sandbox:/app/benchmark_output ./benchmark_output`