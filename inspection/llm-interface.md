## Core Distinction
- **vLLM**: An inference engine optimized for serving LLMs with high throughput.
- **llama.cpp**: lightweight C/C++ local inference runtime.
- **GGUF**: A model file format designed for efficient storage and quantization.

## Architectural + operational contrast 

<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/4a56d87e-23af-4de9-a8ab-215b26bf1318" />

## Applicability

| Criteria | vLLM | llama.cpp / GGUF | Why It Matters for Evals |
|-----------|----------------|---------------------|----------------------------|
| Reproducibility | Good | Excellent | Fixed quantization ensures consistent outputs |
| Batch Processing | Excellent | Good | vLLM optimized for high-throughput batching |
| Direct Access | Python API | C++ library + bindings | Both allow non-HTTP, in-process access |
| Multi-Model Testing | Native support | Multiple instances | Evals compare several models simultaneously |
| Research Budget | Requires GPU | CPU-friendly | Hardware cost and accessibility |

---

## Fit for AISafetyTemplate

### Workload
- Post-compute safety evals (batch, not real-time)
- Expected concurrency: 1–10 sequential evaluations
- High compute per task
- Reproducibility is critical

### Infrastructure
- TEE compatibility required
- Minimal dependencies for security hardening
- Must run offline / air-gapped
- Hardened containerization

### Operational
- Simple for researchers (not DevOps heavy)
- Verifiable execution
- Deterministic behavior
- Direct model access (no external APIs)

---

## Model Compatibility & Ecosystem

| Aspect | vLLM | llama.cpp / GGUF | Why It Matters |
|---------------------------|---------------------------------------------|---------------------------------------------|---------------------------------------------|
| Hugging Face Integration | Native compatibility, direct loading from Hub | Requires conversion (`convert_hf_to_gguf.py`) | Direct loading reduces setup time and errors |
| New Model Adoption Speed | Days (official/fast releases) | Weeks (community-driven support) | Faster adoption enables quicker experimentation |
| Model Conversion Friction | None (PyTorch/Safetensors native) | Manual conversion + quantization | Extra steps add maintenance overhead |
| Eval Stability Trade-off | Rapid updates may change behavior | Conversion creates fixed artifacts | Fixed binaries improve reproducibility |
| Supported Architectures | Llama, Mistral, Mixtral, Qwen, DeepSeek, Phi, Gemma | Same core set | Broad parity for mainstream models |
| Architecture Adoption | Typically first to support new designs | Slower, after community adds kernels | Matters for cutting-edge research |
| Quantization Options | Basic/standard | Wide range (Q2_K → Q8_0) | More quantization enables smaller/edge deployments |
| Multimodal Support | vLLM-Omni (text/image/video/audio) | libmtmd (basic vision/audio) | Determines suitability for multimodal agents |
| Ecosystem Style | Production/server-focused | Local/edge/hobbyist-friendly | Influences deployment strategy |

## Agent Systems & Router Architectures

#### Tool Calling Support

| Capability | vLLM | llama.cpp / GGUF | Why It Matters |
|--------------|-------------------------------|--------------------------------|-------------------------------|
| Native Function Calling | Production-grade, OpenAI-compatible | Limited native support | Reduces custom glue code and improves reliability |
| Parallel Calls | Supported | Not natively supported | Enables multi-step and concurrent agent actions |
| Streaming | Supported | Limited / manual | Improves responsiveness and UX for long generations |
| Control Mechanism | Built-in function schema handling | Grammar-based constrained generation | Structured outputs vs prompt engineering hacks |
| Integration Effort | Low | Medium–High (manual prompts/parsers) | Faster development and fewer edge cases |
| Best Fit | Complex multi-agent systems | Simple agent workflows | Match tooling to system complexity |


#### Architecture Pattern
```
Controller (Rust/C)
    ├─> Model Under Test (llama-server instance)
    └─> Python UserComponent
        ├─> Attack Agent 1 (jailbreak)
        ├─> Attack Agent 2 (toxicity)
        └─> Attack Agent 3 (capability elicitation)
```

### Router Models (Multi-Model Orchestration)
#### Multi-Model Concurrency Analysis

| Approach | vLLM | llama.cpp |
|----------|------|-----------|
| **Many small models** | Inefficient (PagedAttention overhead) | Efficient (low per-instance cost) |
| **One large model** | Excellent (tensor parallelism) | Limited (no native multi-GPU) |
| **Model isolation** | Complex to verify | Simple (separate processes) |
| **For safety evals** | Overkill | Better fit |

### Batching Behavior Impact on Agents

**vLLM**: Continuous batching = variable latency per agent
- Good: High throughput for many agents
- Bad: Unpredictable timing for sequential attacks

**llama.cpp**: Single-stream processing = consistent latency
- Good: Predictable, reproducible agent behavior
- Bad: Doesn't scale to 100+ concurrent agents
- **For your use case (1-10 agents)**: Perfectly adequate

---
## Ecosystem Maturity & Adoption
### Usage Patterns

| Pattern | vLLM | llama.cpp |
|---------|------|-----------|
| **Enterprise production** | Stripe, Meta, Cohere, IBM | - |
| **Consumer apps** | - | Ollama, LM Studio, GPT4All, Jan |
| **Research/academia** | Growing | Dominant |
| **Safety eval tools** | Emerging | Established (HarmBench, etc.) |

### Release Cadence
- **vLLM**: Monthly releases (4-6 weeks), professional release management
- **llama.cpp**: Daily/weekly commits, continuous development
- **Both**: Very active, neither is stagnant

### Corporate Backing
- **vLLM**: Red Hat, Google Cloud, IBM Research, NVIDIA
- **llama.cpp**: Community-driven (sustainability concern but 85k stars = resilient)

### Safety Evaluation Ecosystem
- **vLLM**: Growing (newer in research space)
- **llama.cpp**: More established safety eval tools built on GGUF
- **For international collaboration**: GGUF's simpler format aids reproducibility

## Evaluation Execution Patterns

   ####  Sequential Testing
```
Safety Eval → Model A (proprietary) → Results
           → Model B (open-source) → Results  
           → Model C (baseline)    → Results
```
- **llama.cpp**: Run 3 instances (different ports), simple
- **vLLM**: Can serve multiple models in one instance (more complex)

| Aspect | vLLM | llama.cpp / GGUF | Why It Matters |
|-------------------|---------------------------|---------------------------|--------------------------------|
| Setup | Single multi-model server | Multiple small instances | Simplicity vs isolation |
| Hardware | GPU required | CPU friendly | Cost difference |
| Throughput | High | Medium/Low | Speed of evals |
| Isolation | Shared runtime | Separate processes | Reproducibility |
| Best Fit | Large centralized runs | Small/local testing | Choose based on infra |


####  Concurrent Evaluation
```
Eval Suite dispatches to:
    ├─> Model A server
    ├─> Model B server  
    └─> Model C server
(Results aggregated by Controller)
```
| Aspect | vLLM | llama.cpp / GGUF | Why It Matters |
|-------------------|---------------------------|---------------------------|--------------------------------|
| Concurrency | Native batching | Manual processes | Scaling complexity |
| Multi-model | Built-in | Separate servers | Ops overhead |
| Scaling style | Vertical (big GPU) | Horizontal (many CPUs) | Infra design |
| Latency | Lower under load | Higher under load | Runtime speed |
| Best Fit | Production eval farms | Distributed/lightweight | Deployment choice |

## Final Recommendation

| Scenario | Preferred Choice | Reason |
|-------------------------------|-------------------|----------------------------------------------|
| Safety evaluations | llama.cpp / GGUF | Deterministic quantization, reproducible |
| Benchmarks | llama.cpp / GGUF | Low resource, easy local runs |
| Offline / air-gapped | llama.cpp / GGUF | No cloud or GPU dependency |
| TEE / security-sensitive | llama.cpp / GGUF | Minimal dependencies, single binary |
| High concurrency (>10 users) | vLLM | Continuous batching, GPU throughput |
| Production APIs | vLLM | OpenAI-compatible serving stack |
| Complex agents / tool calling | vLLM | Native function calling + streaming |
| Large-scale orchestration | vLLM | Multi-model + high throughput |

## Hybrid Strategy

| Layer | Tool | Responsibility |
|----------|--------------|-----------------------------|
| Core | llama.cpp / GGUF | Safety evals, benchmarks, reproducible baselines |
| Optional | vLLM | Complex agents, orchestration, high-throughput serving |

**Conclusion:**  
1.  llama.cpp/GGUF as the default for safety and reproducible evaluation workflows, 
2.  vLLM only when high concurrency, agent tooling, or production-scale serving is required.
