# AI Safety Template - HarmBench Evaluation

A containerized AI safety evaluation framework that runs HarmBench safety assessments using a secure two-container architecture with LLM proxy pattern.

## Architecture

```
┌─────────────────────────────────────┐
│ Controller Container                │
│  - Security enforcement             │
│  - HELM integration                 │
│  - Result collection                │
│  - Communicates with LLM via HTTP   │
└──────────────┬──────────────────────┘
               │ HTTP (OpenAI-compatible API)
               ↓
┌─────────────────────────────────────┐
│ LLM Proxy Container                 │
│  - OpenAI-compatible API interface  │
│  - Proxies to actual model          │
└──────────────┬──────────────────────┘
               │ HTTPS
               ↓
       [OpenAI API - Hosted Model]
```

## Features

- **Two-Container Architecture**: Separation of concerns between evaluation logic and model access
- **Security Enforcement**: Import allowlisting and filesystem restrictions
- **HarmBench Evaluation**: Tests model safety against harmful behaviors using JailbreakBench dataset
- **Proxy Pattern**: Easy to swap different LLM backends without changing evaluation code
- **Docker Compose**: Simple orchestration and networking
- **Standardized Output**: JSON results with metrics (refusal rate, attack success rate)

## Quick Start

### Prerequisites

- Docker and Docker Compose installed
- OpenAI API key

### Setup

1. Clone the repository (or navigate to the project directory)

2. Create environment file:
   ```bash
   cp .env.example .env
   ```

3. Edit `.env` and add your OpenAI API key:
   ```
   OPENAI_API_KEY=sk-your-actual-api-key-here
   ```

### Run the Evaluation

```bash
# Build and run both containers
docker-compose up --build

# Or run in detached mode
docker-compose up --build -d

# View logs
docker-compose logs -f controller
```

### View Results

Results are saved to `./results/evaluation_output.json`:

```bash
cat results/evaluation_output.json
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
    "model": "openai/gpt-4o-mini",
    "proxy": "http://llm-proxy:8000"
  }
}
```

## Project Structure

```
ai-safety-template/
├── llm_proxy/
│   ├── server.py              # Proxy server implementation
│   ├── Dockerfile             # Proxy container definition
│   └── requirements.txt       # Proxy dependencies
├── controller/
│   ├── controller.py          # Main controller and model client
│   └── security.py            # Security enforcement
├── safety_eval.py             # HarmBench evaluation logic
├── config.yaml                # Configuration
├── Dockerfile                 # Controller container definition
├── docker-compose.yml         # Container orchestration
├── requirements.txt           # Controller dependencies
├── .env.example               # Environment variables template
└── README.md                  # This file
```

## Configuration

Edit [`config.yaml`](config.yaml) to customize:

- **Model settings**: proxy URL, model name, temperature
- **Security settings**: allowed imports, filesystem paths, network hosts
- **Evaluation settings**: number of samples, dataset source

## Security Features

The Controller container implements security restrictions:

### Import Allowlist
Only approved Python modules can be imported:
- Core libraries: `json`, `os`, `logging`, `datetime`
- Required dependencies: `requests`, `datasets`, `controller`
- Blocks: `subprocess`, `socket`, unauthorized modules

### Filesystem Restrictions
File access is restricted to:
- **Writable**: `/tmp/results`, `/tmp/helm_cache`
- **Readable**: `/app`, `/tmp`

### Network Restrictions
Network access is limited by Docker networking to:
- LLM Proxy container only (`llm-proxy:8000`)
- HuggingFace for dataset downloads (during setup)

**Note**: Security enforcement is currently disabled by default for initial testing. To enable, uncomment the security initialization lines in [`safety_eval.py`](safety_eval.py:288-291).

## Testing Individual Components

### Test LLM Proxy

```bash
# Build and run proxy
docker build -t llm-proxy -f llm_proxy/Dockerfile .
docker run -p 8000:8000 -e OPENAI_API_KEY=$OPENAI_API_KEY llm-proxy

# In another terminal, test the proxy
curl -X POST http://localhost:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{"messages": [{"role": "user", "content": "Hi"}], "model": "gpt-4o-mini"}'

# Check health
curl http://localhost:8000/health
```

### Test Controller Locally

```bash
# Install dependencies
pip install -r requirements.txt

# Set environment variables
export LLM_PROXY_URL=http://localhost:8000
export OPENAI_API_KEY=sk-your-key

# Run evaluation
python safety_eval.py
```

## Evaluation Details

### HarmBench / JailbreakBench

The evaluation uses the [JailbreakBench dataset](https://huggingface.co/datasets/JailbreakBench/JBB-Behaviors) which contains:
- 440 harmful behaviors across 7 categories
- Categories: Cybercrime, weapons, misinformation, harassment, etc.
- "Direct Request" attack type (simplest baseline)

For the sprint, we test 50 behaviors (configurable in [`config.yaml`](config.yaml:402)).

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
- Check proxy health: `docker-compose logs llm-proxy`
- Verify network: `docker network ls`

### "OPENAI_API_KEY not configured"
- Check `.env` file exists and contains valid API key
- Rebuild containers: `docker-compose up --build`

### "Failed to load JailbreakBench dataset"
- Internet connection required for first run
- Dataset is cached after first download
- Fallback behaviors are used if dataset unavailable

### "Import not allowed" or "Permission denied"
- Security enforcement is active
- Check [`config.yaml`](config.yaml) for allowed imports/paths
- Or disable security for testing (see [`safety_eval.py`](safety_eval.py:288))

## Development

### Adding Security Enforcement

To enable full security enforcement, edit [`safety_eval.py`](safety_eval.py):

```python
def main():
    # Uncomment these lines:
    from controller.security import initialize_security
    security_enforcer = initialize_security('/app/config.yaml')
    logger.info("Security enforcement enabled")

    # ... rest of main()
```

### Changing Model Backend

The proxy pattern makes it easy to swap backends:

1. Edit [`llm_proxy/server.py`](llm_proxy/server.py)
2. Replace OpenAI client with your preferred provider
3. Keep the `/v1/chat/completions` endpoint interface
4. Rebuild: `docker-compose up --build`

### Customizing Evaluation

Edit [`safety_eval.py`](safety_eval.py) to:
- Change number of samples
- Modify refusal detection logic
- Add custom behaviors
- Integrate with HELM scenarios

## License

MIT License - see LICENSE file for details

## References

- [HarmBench Paper](https://arxiv.org/abs/2402.04249)
- [JailbreakBench](https://jailbreakbench.github.io/)
- [HELM Safety](https://crfm.stanford.edu/helm/)
- [OpenAI API](https://platform.openai.com/docs/api-reference)

## Support

For issues or questions:
1. Check the Troubleshooting section above
2. Review logs: `docker-compose logs`
3. Open an issue on the repository
