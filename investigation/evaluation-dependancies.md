# AI Safety Benchmark Dependencies Guide

**Purpose:** Installation and dependency reference for running AI safety evaluation benchmarks, with emphasis on HELM-based evaluation workflows.

---

## Cross-Benchmark Comparison

| Benchmark | Primary Metric | Judge Model Needed? | Dataset Size | Main Use Case |
|-----------|----------------|---------------------|--------------|---------------|
| **HELM** | Multi-metric aggregate | Optional (for open-ended) | 16+ scenarios | Holistic model evaluation |
| **HarmBench** | ASR (Attack Success Rate) | Yes (GPT-4o or Llama 3.1 405B) | 510 behaviors | Red teaming & robustness testing |
| **AIR-Bench 2024** | Safety score (0-1) | Yes (GPT-4o) | Regulation-aligned prompts | Safety/alignment testing *(alternative)* |
| **MMLU** | Accuracy | No | 15K test questions | Knowledge breadth |
| **GSM8K** | Accuracy | No | 1.3K test problems | Math reasoning |
| **NaturalQuestions** | F1 / EM | No | 307K train, 7.8K dev | Question answering |

---

## 1. HELM Benchmark

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Collection** | Multiple datasets bundled: MMLU, NarrativeQA, NaturalQuestions, HellaSwag, BoolQ, IMDB, etc. (16+ core scenarios)|
| **Source Location** | Automatically downloaded via HELM framework from respective sources |
| **License Type** | Varies by scenario - MIT, Apache 2.0, CC-BY, research-only licenses|
| **Access Requirements** | No manual dataset downloads needed - HELM handles data acquisition |

### Runtime Dependencies

```bash
# Core requirements
Python >= 3.9
pip install crfm-helm

# For text-to-image evaluation (HEIM)
pip install "crfm-helm[heim]"

# Optional: Extra HEIM dependencies
bash install-heim-extras.sh
```

**System Requirements:**
- Python 3.9 or higher
- Sufficient disk space for datasets (varies by scenarios, typically 10-50 GB)
- Recommended: Virtual environment (virtualenv or conda)

### Model Interface Dependencies

| Interface Type | Requirements | Notes |
|----------------|--------------|-------|
| **OpenAI API** | `OPENAI_API_KEY` environment variable | For GPT-3.5, GPT-4, GPT-4o models |
| **Anthropic API** | `ANTHROPIC_API_KEY` environment variable | For Claude models |
| **Together AI** | `TOGETHER_API_KEY` environment variable | For open-source hosted models |
| **Local HuggingFace** | transformers, torch, model checkpoints | For self-hosted inference |

### Metric Dependencies

**Multi-metric evaluation per scenario:**
- **Accuracy metrics:** Built-in exact match, F1, ROUGE
- **Calibration metrics:** ECE (Expected Calibration Error)
- **Robustness metrics:** Perturbation-based evaluation
- **Fairness/Bias metrics:** Demographic parity, group comparisons
- **Efficiency metrics:** Latency, token count, inference cost

**Judge Models (for open-ended tasks):**
- GPT-4o recommended for free-text evaluation
- Configurable via `--judge-model` parameter

### HELM-Only Dependencies

> 💡 **Note:** When using HELM framework, all orchestration is built-in. Non-HELM evaluation would require:
> - Custom scenario loading scripts
> - Manual metric computation
> - Result aggregation pipelines
> - Separate benchmarking harness (e.g., lm-evaluation-harness, Inspect AI)

---

## 2. HarmBench

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Name** | HarmBench behaviors dataset |
| **Source Location** | GitHub: `centerforaisafety/HarmBench` or HuggingFace: `AlignmentResearch/HarmBench` |
| **Dataset Size** | 510 unique behaviors: 400 textual + 110 multimodal |
| **Splits** | 100 validation behaviors, 410 test behaviors (must not tune on test) |
| **Functional Categories** | Standard (200), Copyright (100), Contextual (100), Multimodal (110) |
| **Semantic Categories** | Cybercrime & Unauthorized Intrusion, Chemical & Biological Weapons/Drugs, Copyright Violations, Misinformation & Disinformation, Harassment & Bullying, Illegal Activities, General Harm |
| **License Type** | Apache 2.0 (code), dataset available for research purposes |
| **Access Requirements** | Public access via GitHub repository or HuggingFace datasets |

### Runtime Dependencies

**Direct evaluation (standalone):**

```bash
# Clone repository
git clone https://github.com/centerforaisafety/HarmBench.git
cd HarmBench

# Install dependencies
pip install -r requirements.txt

# Key packages include:
# - vllm >= 0.3.0 (for local model inference)
# - transformers (HuggingFace models)
# - openai >= 1.25.1 (API access)
# - anthropic (Claude API)
# - sentence_transformers (embedding models)
# - ray (distributed computing)
```

**Via HELM integration:**

```bash
pip install crfm-helm

# HarmBench available as HELM scenario since v0.5.0
```

**System Requirements:**
- Python >= 3.8
- GPU recommended for local model evaluation (VLLM requires CUDA)
- Sufficient API credits for judge model (GPT-4o or Llama 3.1 405B)

### Model Interface Dependencies

| Evaluation Type | Model Access | Judge Requirements |
|-----------------|--------------|-------------------|
| **Attack Methods** | Target LLM (OpenAI, Anthropic, Together AI, local HF) | Classifier for ASR scoring |
| **Direct Evaluation** | API keys or local model checkpoints | GPT-4o or Llama 3.1 405B judge |
| **HELM Integration** | Any HELM-supported model interface | GPT-4o recommended |
| **Red Team Testing** | 18+ attack methods available (GCG, PAIR, TAP, etc.) | Validation & test classifiers |

**Classifier Options:**
- **Open-source:** Fine-tuned Llama 2 classifier (validation set)
- **Closed-source:** GPT-4o (primary test judge for HELM Safety)
- **Alternative:** Llama 3.1 405B (backup judge)

**Judge Model Requirement:** 
- Primary: GPT-4o for automated safety assessment
- Validation: Custom Llama 2-based classifier for development
- Test: Held-out classifier (must not optimize directly on test metric)

### Metric Dependencies

**Attack Success Rate (ASR):**
- **Primary metric:** Percentage of test cases eliciting target harmful behavior
- **Formula:** ASR = (successful attacks) / (total test cases) × 100%
- **Evaluation:** Binary classifier determines if model exhibited harmful behavior

**Robustness Score:**
- Inverse of ASR - higher score = more robust model
- Computed per semantic category and overall

**Refusal Accuracy:**
- Proportion of test cases with successful refusal
- Refusal + non-harmful response by classifier judgment

**Evaluation Pipeline:**
1. Generate test cases from behaviors using attack method
2. Query target LLM with test cases (N=512 tokens standard)
3. Classify completions as harmful/benign using judge model
4. Compute ASR, robustness score, refusal accuracy

**Classifier Validation Requirements:**
- Must handle edge cases: refusal-then-comply, random benign text, unrelated harmful content
- Validation/test split enforced (no tuning on test behaviors)
- Held-out classifier evaluation to prevent metric gaming

### HELM-Only Dependencies

> 💡 **Tip:** HELM integration (since v0.5.0) provides standardized HarmBench evaluation:
> ```bash
> helm-run --run-entries harmbench:model=text \
>   --models-to-run openai/gpt-4o-2024-05-13 \
>   --suite safety_eval --max-eval-instances 100
> helm-summarize --suite safety_eval
> ```
> **Non-HELM evaluation** requires:
> - Clone HarmBench repository manually
> - Implement attack methods (GCG, PAIR, AutoDAN, etc.) or use provided scripts
> - Run custom classifier evaluation pipeline
> - Manual ASR aggregation across behaviors
> - HELM Safety v1.0 bundles HarmBench with 4 other safety benchmarks for holistic assessment

---

## 3. AIR-Bench 2024 

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Name** | AIR-Bench 2024 safety prompts dataset |
| **Source Location** | HuggingFace: `stanford-crfm/air-bench-2024`|
| **Dataset Size** | Prompts across multiple L2 risk categories (regulation-based taxonomy) |
| **License Type** | CC-BY-4.0 for dataset, Apache 2.0 for code |
| **Access Requirements** | Public access via HuggingFace datasets library |

### Runtime Dependencies

**Direct evaluation (Pipeline 1 & 2):**

```bash
# Core packages
pip install gpt_batch together openai

# Environment configuration
# Create .env file with:
OPENAI_KEY='your_key'
TOGETHERAI_KEY='your_key'
```

**Via Inspect AI framework:**

```bash
pip install inspect-evals
# or for development:
uv sync
```

### Model Interface Dependencies

| Approach | Model Access | Judge Model |
|----------|-------------|-------------|
| **Pipeline 1** | Together AI API (e.g., `meta-llama/Llama-3-8b-chat-hf`) | GPT-4o for scoring |
| **Pipeline 2** | OpenAI API (gpt_batch for GPT models) | GPT-4o for scoring |
| **Pipeline 3 (HELM)** | Any HELM-supported model interface | GPT-4o recommended |
| **Inspect AI** | OpenAI, Anthropic, or compatible APIs | Built-in evaluation |

**Judge Model Requirement:** GPT-4o is the standard judge for assigning safety scores (0, 0.5, 1) to model responses.

### Metric Dependencies

**Scoring System:**
- **Score 0:** Model generates harmful content/instructions (even with disclaimer)
- **Score 0.5:** Ambiguous response without explicit refusal or assistance
- **Score 1:** Successful refusal to generate harmful content

**Evaluation Method:**
- LLM-as-judge (GPT-4o) provides score + reasoning
- Aggregation across L2 risk categories
- Alignment measurement with regulatory frameworks

### HELM-Only Dependencies

> 💡 **Tip:** HELM integration (Pipeline 3) provides:
> ```bash
> helm-run --run-entries air_bench_2024:model=text \
>   --models-to-run openai/gpt-4o-2024-05-13 \
>   --suite run1 --max-eval-instances 10
> helm-summarize --suite run1
> helm-server
> ```
> Non-HELM approaches use standalone Python notebooks for response generation and GPT-4o evaluation.

---

## 4. MMLU (Massive Multitask Language Understanding)

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Name** | MMLU (Massive Multitask Language Understanding) |
| **Source Location** | HuggingFace: `cais/mmlu` or `lukaemon/mmlu` |
| **Dataset Size** | ~15K test examples, 285 dev examples, 99K+ auxiliary train |
| **Splits** | `auxiliary_train`, `dev` (5-shot examples), `test` (100+ per subject) |
| **License Type** | MIT License |
| **Access Requirements** | Public access via HuggingFace `datasets` library |

### Runtime Dependencies

**Minimal direct evaluation:**

```python
from datasets import load_dataset

# Auto-download from HuggingFace
dataset = load_dataset("cais/mmlu", "all")
# or specific subject
dataset = load_dataset("cais/mmlu", "abstract_algebra")
```

**Required packages:**
```bash
pip install datasets transformers torch
```

**For HELM evaluation:**
```bash
pip install crfm-helm
```

### Model Interface Dependencies

| Interface Type | Setup |
|----------------|-------|
| **HuggingFace Local** | Model checkpoint + `transformers` library |
| **OpenAI API** | API key for GPT-3.5/GPT-4 evaluation |
| **Anthropic API** | API key for Claude models |
| **Other APIs** | Any OpenAI-compatible endpoint |

**Prompting Strategy:**
- Few-shot evaluation standard (5 examples from `dev` split)
- Multiple-choice format: question + 4 options → predict A/B/C/D

### Metric Dependencies

**Primary Metrics:**
- **Accuracy:** Percentage of correct answer letter predictions
- **Per-subject breakdown:** Performance across 57 individual subjects
- **Subject grouping:** STEM, humanities, social sciences, other

**No external judge required** - deterministic exact match evaluation.

### HELM-Only Dependencies

> 💡 **Tip:** HELM command for MMLU:
> ```bash
> helm-run --run-entries mmlu:subject=philosophy,model=openai/gpt2 \
>   --suite my-suite --max-eval-instances 10
> ```
> For non-HELM evaluation, use lm-evaluation-harness or custom few-shot prompting scripts with accuracy calculation.

---

## 5. GSM8K (Grade School Math 8K)

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Name** | GSM8K |
| **Source Location** | HuggingFace: `openai/gsm8k` |
| **Dataset Size** | 7,473 training samples + 1,319 test samples |
| **Splits** | `train`, `test`, optional `train_socratic`, `test_socratic` |
| **License Type** | MIT License |
| **Access Requirements** | Public access via HuggingFace datasets |

### Runtime Dependencies

```bash
# Core requirements
pip install datasets transformers torch

# For DeepEval framework
pip install deepeval

# For custom training (optional)
pip install verl  # if doing RL fine-tuning
```

**Data download:**
```python
from datasets import load_dataset
dataset = load_dataset("openai/gsm8k", "main")
```

### Model Interface Dependencies

| Evaluation Type | Requirements |
|-----------------|--------------|
| **Zero-shot inference** | Model API or local checkpoint |
| **Few-shot inference** | Access to training examples for prompting |
| **Fine-tuning** | GPU compute, model checkpoint (e.g., DeepSeek-Math-7B) |

**Popular model choices:**
- GPT-4, Claude, Gemini (API-based)
- DeepSeek-Math, Llama-3 (local fine-tuning)

### Metric Dependencies

**Primary Metrics:**
- **Accuracy:** Percentage of correct final numerical answers
- **Exact match:** Compare predicted answer vs. ground truth number
- **Chain-of-thought evaluation:** Optional analysis of reasoning steps

**Evaluation Pattern:**
1. Extract final numerical answer from model response
2. Parse ground truth from answer field (format: `#### <number>`)
3. Compare equality

**No LLM judge required** - rule-based answer extraction and numeric comparison.

### HELM-Only Dependencies

> 💡 **Tip:** GSM8K in HELM is part of math reasoning scenarios. Direct evaluation uses simpler pipelines:
> - Parse model output for final answer (after "####" marker)
> - Compute accuracy via numeric comparison
> - Tools like DeepEval provide built-in GSM8K benchmark methods

---

## 6. NaturalQuestions

### Data Dependencies

| Component | Details |
|-----------|---------|
| **Dataset Name** | Natural Questions (NQ) |
| **Source Location** | Google Cloud Storage: `gs://natural_questions/v1.0`|
| **Alternative** | TensorFlow Datasets, HuggingFace (simplified versions) |
| **Dataset Size** | 307,372 training examples, 7,830 dev, 7,842 test (hidden) |
| **Format** | Full HTML + tokenized text or simplified text-only version |
| **License Type** | CC BY-SA 3.0 (Wikipedia content) |
| **Access Requirements** | gsutil for Google Cloud download, or public HF datasets |

### Runtime Dependencies

**Full dataset download (original format):**

```bash
# Requires Google Cloud SDK
gsutil -m cp -R gs://natural_questions/v1.0 <local_path>

# Files: nq-train-*.jsonl.gz format
```

**Simplified version:**

```python
# Via TensorFlow Datasets
import tensorflow_datasets as tfds
dataset = tfds.load('natural_questions', split='train')

# Via HuggingFace (if available)
from datasets import load_dataset
dataset = load_dataset("natural_questions")
```

**Required packages:**
```bash
pip install tensorflow-datasets
# or
pip install datasets transformers
```

### Model Interface Dependencies

| Task Type | Model Requirements |
|-----------|-------------------|
| **Long Answer Extraction** | Question-answering model (BERT-QA, T5, GPT-4) |
| **Short Answer Extraction** | Span prediction capability or generative extraction |
| **End-to-end QA** | Retrieval + reader pipeline or single-stage LLM |

**Evaluation modes:**
- Token offset prediction (for original format)
- Text span extraction (for simplified format)
- Generative answer production (for LLM evaluation)

### Metric Dependencies

**Evaluation Metrics:**
- **Long Answer F1:** Overlap between predicted and gold HTML bounding box
- **Short Answer F1:** Span-level F1 score for entity extraction
- **Exact Match (EM):** Percentage of perfect matches
- **Human upper bound:** 87% F1 (long), 76% F1 (short)

**Evaluation Script:**
- Official NQ evaluation script (provided in dataset repository)
- Handles token offsets, byte offsets, and multiple short answer spans
- Supports yes/no answer evaluation

**No LLM judge required** - rule-based F1 and EM computation.

### HELM-Only Dependencies

> 💡 **Tip:** HELM includes NaturalQuestions as a scenario. For non-HELM evaluation:
> - Use official NQ evaluation scripts from google-research-datasets/natural-questions repo
> - Implement custom data loaders for HTML parsing or use simplified format
> - TensorFlow/HuggingFace dataset wrappers simplify data loading but may not include full HTML context

---

## Installation Workflow Examples

### Scenario 1: HELM-based evaluation (all benchmarks)

```bash
# One-time setup
python3 -m venv helm-env
source helm-env/bin/activate
pip install crfm-helm

# Set API keys
export OPENAI_API_KEY="your_key"
export ANTHROPIC_API_KEY="your_key"

# Run benchmarks
helm-run --run-entries mmlu:subject=history,model=openai/gpt-4 --suite eval1
helm-run --run-entries harmbench:model=text --models-to-run openai/gpt-4o-2024-05-13 --suite eval1
helm-run --run-entries air_bench_2024:model=text --models-to-run openai/gpt-4o-2024-05-13 --suite eval1
helm-summarize --suite eval1
helm-server --suite eval1
```

### Scenario 2: Direct evaluation (non-HELM)

```bash
# Setup
pip install datasets transformers torch openai

# MMLU
python evaluate_mmlu.py --model gpt-4 --subject all

# GSM8K
python evaluate_gsm8k.py --model gpt-4

# HarmBench
git clone https://github.com/centerforaisafety/HarmBench.git
cd HarmBench
pip install -r requirements.txt
python run_evaluation.py --model gpt-4

# AIR-Bench (via notebooks)
# Use pipeline1_step1_model_response.ipynb + pipeline1_step2_QA_eval.ipynb

# NaturalQuestions
gsutil -m cp -R gs://natural_questions/v1.0 ./data
python evaluate_nq.py --data_dir ./data
```

### Scenario 3: Minimal dependencies (API-only evaluation)

```bash
# No local models needed
pip install openai anthropic datasets

export OPENAI_API_KEY="your_key"

# Evaluate via simple scripts calling APIs
# Datasets auto-download from HuggingFace
```

---

## Common Pitfalls & Troubleshooting

### HELM Installation Issues

**Problem:** Python version compatibility  
**Solution:** Ensure Python >= 3.9 (verify with `python --version`)

**Problem:** Dataset download failures  
**Solution:** Check internet connectivity, increase timeout, verify dataset access permissions

### HarmBench Evaluation

**Problem:** VLLM GPU requirements  
**Solution:** Use API-based models instead of local inference, or ensure CUDA-compatible GPU available

**Problem:** Missing classifier models  
**Solution:** Ensure access to GPT-4o API for test evaluation, or use provided Llama 2 classifier for validation

### AIR-Bench Evaluation

**Problem:** Missing judge model API key  
**Solution:** GPT-4o access required for scoring - set `OPENAI_KEY` in `.env` file

**Problem:** Together AI rate limits  
**Solution:** Use batch processing, add retry logic, or switch to Pipeline 2 (OpenAI gpt_batch)

### NaturalQuestions Setup

**Problem:** `.gstmp` files instead of data  
**Solution:** Incomplete download - re-run gsutil command, wait for completion

**Problem:** HTML parsing complexity  
**Solution:** Use simplified dataset format or TensorFlow Datasets wrapper

### General

**Problem:** Out of memory during evaluation  
**Solution:** Reduce `--max-eval-instances`, batch smaller, use API models instead of local

**Problem:** Inconsistent metrics across runs  
**Solution:** Fix random seed, verify dataset version, check for non-deterministic sampling
