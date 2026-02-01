# AI Safety Template - HarmBench Evaluation

A containerized AI safety evaluation framework that runs HarmBench safety assessments using a secure two-container architecture with LLM proxy pattern.

## Architecture

```
┌─────────────────────────────────────┐
│ Eval Sandbox Container              │
│  - Runs HELM evaluations            │
│  - User component execution         │
│  - NO internet access               │
│  - Connects only to LLM proxy       │
└──────────────┬──────────────────────┘
               │ HTTP (OpenAI-compatible API)
               ↓
┌─────────────────────────────────────┐
│ LLM Proxy Container                 │
│  - OpenAI-compatible API interface  │
│  - Proxies to HuggingFace Inference │
└──────────────┬──────────────────────┘
               │ HTTPS
               ↓
       [HuggingFace Inference API]
```

## Features

- **Two-Container Architecture**: Separation of concerns between evaluation logic and model access
- **Security Enforcement**: Network isolation, import restrictions, and filesystem restrictions
- **HarmBench Evaluation**: Tests model safety against harmful behaviors using JailbreakBench dataset
- **Proxy Pattern**: Easy to swap different LLM backends without changing evaluation code
- **HELM Integration**: Full support for HELM (Holistic Evaluation of Language Models) benchmarks
- **Offline Mode**: Datasets pre-downloaded for air-gapped evaluation
- **Docker Compose**: Simple orchestration and networking
- **Standardized Output**: JSON results with metrics (refusal rate, attack success rate)

## Quick Start

### Prerequisites

- Docker and Docker Compose installed
- HuggingFace API key

### Setup

1. Clone the repository (or navigate to the project directory)

2. Create environment file:
   ```bash
   cp .env.example .env
   ```

3. Edit `.env` and add your HuggingFace API key:
   ```
   HF_API_KEY=hf_your-actual-api-key-here
   ```

### Run the Evaluation

```bash
# Option A: Use the helper script (recommended)
./run.sh

# Option B: Use docker-compose directly
docker-compose up --build

# Or run in detached mode
docker-compose up --build -d

# View logs
docker-compose logs -f eval_sandbox
```

### View Results

Results are saved to the `eval_output` Docker volume. To access:

```bash
# View results from volume
docker run --rm -v ai-safety-template_eval_output:/data alpine cat /data/evaluation_results.json
```

Expected output format:
```json
{
  "version": "0.1.0",
  "timestamp": "2025-01-31T17:00:00Z",
  "scenarios": [
    {
      "name": "harmbench",
      "metrics": {
        "refusal_rate": 0.92,
        "attack_success_rate": 0.08,
        "num_behaviors": 50
      }
    }
  ],
  "metadata": {
    "model": "smollm3-3b",
    "evaluation_type": "direct_request"
  }
}
```

## Project Structure

```
ai-safety-template/
├── llm_proxy/
│   ├── server.py              # Proxy server (HuggingFace backend)
│   ├── Dockerfile             # Proxy container definition
│   └── requirements.txt       # Proxy dependencies
├── controller/
│   ├── src/                   # C++ Controller source
│   ├── include/               # C++ headers
│   ├── python/
│   │   └── controller.py      # Python wrapper for Controller DLL
│   └── CMakeLists.txt         # Build configuration
├── eval_container/
│   ├── Dockerfile             # Eval sandbox container definition
│   ├── requirements.txt       # Eval dependencies
│   ├── download_datasets.sh   # Script to pre-download datasets
│   ├── offline_datasets/      # Pre-downloaded datasets
│   └── SECURITY.md            # Security documentation
├── user_component/
│   ├── boilerplate/
│   │   └── my_eval.py         # Template for custom evaluations
│   └── examples/
│       └── harmbench_eval/
│           └── harmbench_eval.py  # HarmBench evaluation implementation
├── helm_config/
│   ├── model_deployments.yaml # HELM model configuration
│   ├── model_metadata.yaml    # HELM model metadata
│   └── run_entries.conf       # HELM run configuration
├── config.yaml                # Main configuration file
├── Dockerfile                 # Controller build container
├── docker-compose.yml         # Container orchestration
├── requirements.txt           # Controller Python dependencies
├── .env.example               # Environment variables template
├── run.sh                     # Quick start script
├── test_proxy.sh              # Proxy testing script
├── build_controller.sh        # Controller build script
├── USAGE.md                   # Detailed usage guide
└── README.md                  # This file
```

## Configuration

Edit [`config.yaml`](config.yaml) to customize:

- **Model settings**: proxy URL, model name, temperature
- **Security settings**: allowed imports, filesystem paths, network hosts
- **Evaluation settings**: number of samples, dataset source

## Security Features

The Eval Sandbox container implements security restrictions:

### Network Isolation
- **NO internet access**: Internal network only
- Can only communicate with LLM proxy container
- Cannot download external resources at runtime

### Import Restrictions
Only approved Python modules can be imported:
- Core libraries: `json`, `os`, `logging`, `datetime`
- Required dependencies: `controller` (the main interface)
- Blocks: `subprocess`, `socket`, unauthorized modules

### Filesystem Restrictions
File access is restricted to:
- **Writable**: `/app/output`, `/tmp`
- **Readable**: `/app`, `/app/eval`, `/app/helm_config`

### Container Hardening
- Runs as non-root user
- Read-only filesystem where possible
- No privileged access
- Minimal capabilities (only NET_BIND_SERVICE)
- Resource limits enforced

## Testing Individual Components

### Test LLM Proxy

```bash
# Build and run proxy
docker-compose up llm_proxy

# In another terminal, test the proxy
./test_proxy.sh

# Or manually test
curl http://localhost:8000/health
```

### Test Eval Sandbox

```bash
# Build and run both containers
docker-compose up --build

# View eval sandbox logs
docker-compose logs -f eval_sandbox
```

## Evaluation Details

### HarmBench / JailbreakBench

The evaluation uses the [JailbreakBench dataset](https://huggingface.co/datasets/JailbreakBench/JBB-Behaviors) which contains:
- 440 harmful behaviors across 7 categories
- Categories: Cybercrime, weapons, misinformation, harassment, etc.
- "Direct Request" attack type (simplest baseline)

For the sprint, we test 50 behaviors (configurable in [`config.yaml`](config.yaml)).

### Refusal Detection

Simple keyword-based refusal detection:
- Looks for phrases like "I can't", "I cannot", "I'm not able"
- Classifies responses as "refused" or "complied"
- Computes refusal rate and attack success rate

### Metrics

- **Refusal Rate**: Percentage of harmful requests the model refused
- **Attack Success Rate**: 1 - Refusal Rate (lower is better)
- **Num Behaviors**: Total number of behaviors tested

## Troubleshooting

### "Can't connect to proxy"
- Ensure both containers are running: `docker-compose ps`
- Check proxy health: `docker-compose logs llm_proxy`
- Verify network: `docker network ls`

### "HF_API_KEY not configured"
- Check `.env` file exists and contains valid API key
- Rebuild containers: `docker-compose up --build`

### "Failed to load dataset"
- Datasets must be pre-downloaded for offline mode
- Run `eval_container/download_datasets.sh` before building
- Fallback behaviors are used if dataset unavailable

### "Import not allowed" or "Permission denied"
- Security enforcement is active
- Check [`config.yaml`](config.yaml) for allowed imports/paths

## Development

### Creating Custom Evaluations

1. Copy the boilerplate template:
   ```bash
   cp user_component/boilerplate/my_eval.py user_component/my_custom_eval.py
   ```

2. Implement your evaluation logic using only the Controller API:
   ```python
   from controller import Controller

   def main():
       controller = Controller()
       controller.init('/app/config.yaml')

       # Your evaluation logic here
       client = controller.get_model_client()
       response = client.complete("Your prompt")

       # Submit results
       controller.submit_results(results)
       controller.shutdown()
   ```

3. Update the Dockerfile CMD to run your evaluation

### HELM Integration

The Controller provides HELM integration for running standard benchmarks:

```python
# Run a built-in HELM scenario
result = controller.run_helm_scenario("mmlu")

# Or run with custom run spec
result = controller.run_helm_benchmark(
    plugin_path="/app/eval/my_run_specs.py",
    run_spec_name="my_custom_eval",
    model_name="openai/smollm3-3b",
    max_instances=10,
    output_path="/app/benchmark_output"
)
```

### Changing Model Backend

The proxy pattern makes it easy to swap backends:

1. Edit [`llm_proxy/server.py`](llm_proxy/server.py)
2. Replace HuggingFace client with your preferred provider
3. Keep the `/v1/chat/completions` endpoint interface
4. Rebuild: `docker-compose up --build`

## License

MIT License - see LICENSE file for details

## References

- [HarmBench Paper](https://arxiv.org/abs/2402.04249)
- [JailbreakBench](https://jailbreakbench.github.io/)
- [HELM Safety](https://crfm.stanford.edu/helm/)
- [HuggingFace Inference API](https://huggingface.co/docs/api-inference)

## Support

For issues or questions:
1. Check the Troubleshooting section above
2. Review logs: `docker-compose logs`
3. Open an issue on the repository
