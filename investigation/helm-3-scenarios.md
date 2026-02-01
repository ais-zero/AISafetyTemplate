# HELM 3 Investigation: Scenario Selection

## Evaluation Coverage

| Evaluation Area | What the Template Measures | What It Provides |
|-----------------|---------------------------|-----------------------------|
| Safety | Harmful compliance, jailbreaks, refusal quality | Misuse prevention evidence |
| Knowledge reliability | Factual correctness across domains | Competence baseline |
| Reasoning correctness | Logical consistency, step-by-step accuracy | Logic failure detection |
| Grounded factual answering | Hallucination risk | Trustworthiness metrics |

---

## Selected Scenarios

### Default Scenario 

| Benchmark | Purpose | Metric | Source | Governance Value |
|-----------|---------|--------|--------|------------------|
| **HarmBench** | Safety / misuse | refusal/violation rate | HELM-native | Red-teaming, misuse prevention, audit defensibility |

HarmBench already covers: harmful compliance, unsafe instructions, jailbreak behavior, refusal quality.

---

### 3+ Scenarios

| Benchmark | Purpose | Metric | Source | Governance Value |
|-----------|---------|--------|--------|------------------|
| **MMLU** (subset) | Knowledge reliability | accuracy % | 3rd-party | Reliability baseline, competence measurement, regression detection |
| **GSM8K** | Reasoning integrity | accuracy % | 3rd-party | Catches hallucinated math, shows logical consistency |
| **NaturalQuestions** | Grounded QA / hallucination risk | EM/F1 % | 3rd-party | Factual accuracy, trustworthiness, user-facing reliability |

---

## Why These? 

| Avoid | Reason |
|-------|--------|
| BBQ | Bias-only, too narrow |
| Tool use / agents | Too complex to audit |
| Domain benchmarks (medical/legal) | Only if product is domain-specific |
| Interactive environments | Hard to audit consistently |

| Preference | Why It Matters |
|------------|----------------|
| Simple | Easy to explain to non-technical stakeholders |
| Reproducible | Consistent results across audit runs |
| Deterministic metrics | No subjective judging needed |
| Easy to explain | Defensible in audit reports |

---

## How it _Feeds_  T#3, T#4, T#14

| Task | What It Uses | Why |
|------|--------------|-----|
| Task 3:  Dependencies | MMLU, GSM8K, NaturalQuestions | As an external benchmarks → need to track as dependencies |
| Task 4: Datasets | HarmBench + the 3 above | Define which datasets to acquire/prepare |
| Task 14:  Standardized Results | All metrics normalize to % | Accuracy, EM, refusal rates all become percentages → consistent dashboarding |

---

## Conclusion

**MMLU + GSM8K + NaturalQuestions** — cleanest, most defensible, most audit-friendly trio in HELM.
