# AI Benchmark Datasets

A collection of benchmark datasets for evaluating large language models across safety and capability dimensions.

---

## Usage Summary

| Evaluation Goal | Benchmark | Size |
|-----------------|-----------|------|
| Safety & Compliance | AIR-Bench 2024 | 5,694 prompts |
| Red Teaming & Robustness | HarmBench | 400+ behaviors |
| Knowledge & Reasoning | MMLU | 15,908 questions |
| Mathematical Reasoning | GSM8K | 8,500 problems |
| Question Answering | NaturalQuestions | 307K+ queries |

---

## Safety & Red Teaming

### HarmBench

> Evaluates model behavior on harmful or disallowed prompts, testing robust refusal and alignment in adversarial red-teaming scenarios.

| Attribute | Details |
|-----------|---------|
| **Size** | 400+ harmful behaviors |
| **Categories** | 6 risk categories |
| **Developed by** | UC Berkeley, Google DeepMind, Center for AI Safety |

**Risk Categories:**
- ⚠️ Chemical/biological threats
- ⚠️ Illegal activities
- ⚠️ Misinformation
- ⚠️ Harassment/hate speech
- ⚠️ Cybercrime
- ⚠️ Copyright violations

---

### AIR-Bench 2024

> AI safety benchmark aligned with regulatory risk categories, using diverse risky prompts to assess model compliance and safe response behavior.

| Attribute | Details |
|-----------|---------|
| **Size** | 5,694 risky instruction prompts |
| **Categories** | 314 granular risk categories |
| **Sources** | 8 government regulations, 16 corporate policies |

**Key Features:**
- ✅ Tests model refusal capabilities
- ✅ Aligned with EU AI Act and other regulatory frameworks
- ✅ Real-world policy compliance evaluation

---

## Capability & Knowledge

### MMLU (Massive Multitask Language Understanding)

> Massive multitask benchmark examining broad factual knowledge and reasoning, used to gauge a model's general knowledge proficiency and robustness across domains.

| Attribute | Details |
|-----------|---------|
| **Size** | 15,908 multiple-choice questions |
| **Scope** | 57 academic subjects |
| **Human Baseline** | ~89.8% accuracy |

**Domains Covered:**
- 🎓 STEM fields
- 📚 Humanities
- 🌍 Social sciences
- 💼 Professional subjects

---

### GSM8K (Grade School Math 8K)

> Grade-school math word problems focusing on multi-step arithmetic reasoning, used to evaluate models' chain-of-thought problem-solving and numerical robustness.

| Attribute | Details |
|-----------|---------|
| **Size** | 8,500 word problems |
| **Complexity** | 2-8 reasoning steps |
| **Operations** | Elementary arithmetic |

**Key Features:**
- ⭐ Non-templated, diverse problem scenarios
- ⭐ Requires natural language explanations
- ⭐ Tests step-by-step reasoning ability

---

### NaturalQuestions

> Real-world question-answering dataset assessing models' ability to retrieve factual information and provide accurate, grounded answers.

| Attribute | Details |
|-----------|---------|
| **Size** | 307,373+ queries |
| **Source** | Real anonymized Google searches |
| **Answers** | Wikipedia pages |

**Answer Types:**
- 🔹 **Long answers:** Paragraph-level context
- 🔹 **Short answers:** Specific entities
- 🔹 **Null:** When no answer exists

---
