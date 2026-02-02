# AISafetyTemplate - A prototype for creating standardized AI safety evaluations that run in a hardened & private way


## Goal for this sprint
To develop a working prototype for creating a steamlined, standardized AI safety evalulations, benchmarks & automated red-teaming with a private, secure & isolated environment to run them.

## Purpose:
Primarily, to make post-compute AI safety work more collaborative, universal & multilaterally trusted. Secondarily, to help secure such safety work so it can run against models while preserving IP privacy.

It targets a subset of AI safety work, those that apply to **foundational international AI safety** over, e.g., (supra-)national regulation or NatSec specific concerns.

Not to create the safety work itself rather the engineering to enable standardized collaboration internationally.
Not to replace existing safety, governance, or policy initiatives but to united them with common infrastructure that is international, non-sovereign, secure, verifiable, distributed, private where needed & transparent as can be.

## Deliverable:
A way to develop post-compute, non-inference-time AI safety work with a streamlined template, containerized and ready to be run inside a Trusted Execution Environment (TEE).
![Visual overview of components](https://github.com/davidg-apart/AISafetyTemplate/blob/aca0e653e092e36f35b6aed5b0e22478a2f53acc/template-idea.png)
The Controller serves as the only external dependancy for the UserComponent from which evulations are derived. It orchestrates the environment, restricts access, handles isolated model interfacing & containerization.
Using Cython/C++ to add security, memory isolation (not implemented for this sprint), performance & precision, imported by the UserComponent as a dll.
For this sprint, the AI Assembly (model/system/framework) will be mocked with a proxied interface to a hosted inference model. In this version it does not contain blockchain or TEE components.

#### Why target Python Environments?
Good question?! Python is a lower-entry, dynamically-typed scripting language with an often wild, lesser-maintained ecosystem. We *should* be using a formal, compiled language like Rust or Haskell. The reality is 90% of post-compute/non-inference-time AI safety work (at least in OSS) uses Python environments, and Python ML ecosystem itself is popular with good maintenance.

#### Why not just put existing Safety Eval code in a docker container and call it a day?
It needs results that are comparable, reproducible, and trusted across organizations and governments.

- Standardization:
  Different evalulation frameworks use slightly different task definitions, metrics, and formats - often from sources with potential conflicts of interest used at the international level.

- Reproducibility:
  We need comparable metrics, comprehensive templates, vetted datasets, solid dependencies, and controlled execution rules.

- Trust & governance
  Participants can’t rely on opaque, centrally built images. The system must be transparent, auditable, and neutral so everyone can verify what’s being run.
    
- Security
  Containers share the host kernel and aren’t strong isolation boundaries. Evaluating untrusted or risky workloads may require stronger sandboxing.
    
- Verifiability
  API-only access can hide behavior behind routing or safeguards. Safety evaluation often requires running directly on the actual models and systems.
 
## Who benefits?

This is apart of an open-source, non-profit initiative. The streamlined development flow benefits new AI safety researchers and groups, while making the product of their research go further as it can be used more broadly.

Smaller AI labs benefit from secure, privacy-respecting solutions for giving access to IP to 3rd-party safety eval orgs. While 3rd-party safety eval orgs benefit from verifiable, trusted direct access to the AI Assembly.

To understand its greater benefit, it must be seen in the higher-level context: to enable verifiable, standardized AI safety collaboration globally while providing an open, trusted solution for tooling and infrastructure.

## Architectual notes
We orignally sought to develop a proprietary evaluation template in a more formal language. During our investigation stage we determined to base the core off one of two great, existing popular OSS frameworks: EleutherAI's LM-Evalutation-Harness or Stanford's The Holistic Evaluation of Language Models (HELM). They did not have conflict of interests and were most likely to be internationally accepted. We selected HELM as it was more comprehensive, including support for red-teaming, despite being a little less popular, it had a good academic backing. We choose Cython/C++ over Rust merely due to team skill set in the scope of this sprint.

## Instructions
Please see the readmes in the src directory for how to use this project.



## The higher-level initiative
For deeper understanding on how this project fits in to the overall design, see FrameworkZero.org.
![What we are doing in this sprint in the context of the overall initiative](https://github.com/davidg-apart/AISafetyTemplate/blob/1e45d683df7e2a1684b7858915241301f13182f1/flow-hackathon.png)

