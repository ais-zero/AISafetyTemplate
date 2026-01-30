# AISafetyTemplate

Welcome to the Hackathon repo!
It will serve to coordinate development and organize our findings.

## Goal
To develop a working prototype for standardized software template(s) for AI Safety Evals, Benches and/or Automated Red-Teaming.
![What we are doing in this sprint in the context of the overall initiative](https://github.com/davidg-apart/AISafetyTemplate/blob/1e45d683df7e2a1684b7858915241301f13182f1/flow-hackathon.png)

## Purpose:
Primarily, to make post-compute AI safety work more collaborative with verifiable standards. Second, to serve as strong boilerplate or SDK to streamline safety work. Additionally, to help secure such safety work by giving it a more hardened environment, and... with collaborative work also comes adversarial work.

This is targets a subset of AI safety work, those that apply to **foundational international AI safety** over, e.g., (supra-)national regulation or NatSec specific concerns. This means not all safety work is specifically relevant to this Template project.

We are not creating the safety work itself rather the engineering to enable standardized collaboration internationally.
Not to replace existing safety, governance, or policy initiatives but to united them with common infrastructure that is international, non-sovereign secure, verifiable, distributed, private where needed & transparent as can be. This is the greater conceptual mission, which will help orient you as we collaborate in this sprint.

## Deliverable:
A verifiable software template(s) for post-compute, non-inference-time AI safety work, containerized and ready to be run inside a Trusted Execution Environment.
Something along the lines of:
![template idea](https://github.com/davidg-apart/AISafetyTemplate/blob/aca0e653e092e36f35b6aed5b0e22478a2f53acc/template-idea.png)
The Controller serves as the only external dependancy for the UserComponent. It orchestrates the environment, verifies UserComponent code, handles external interfacing & containerization.
Using C/Rust for it adds security, performance & precision, referenced from the UserComponent (a Python package) as a dll/binary.
For this sprint, the AI Assembly (model/system/framework) will be mocked and there will be no blockchain or TEE components. None of this is set in stone, let's discuss.

#### Why target Python Environments?
Good question?! Python is a low-entry, dynamically-typed scripting language with an often wild, lesser-maintained ecosystem. We *should* be using a formal, compiled language like Rust or Haskell. The reality is 90% of post-compute/non-inference-time AI safety work (at least in OSS) uses Python environments, and Python ML libraries & tooling are popular with good maintenance.

#### Why not just put Safety Eval code in a docker container and call it a day?
Docker is great for packaging software so it runs the same way everywhere. But international safety evaluation needs more than “it runs.” It needs results that are comparable, reproducible, and trusted across organizations and governments. Containers alone don’t provide those guarantees.

    Standardization
    Different eval frameworks use slightly different task definitions, metrics, and formats. Docker packages them, but it doesn’t make their results consistent or comparable.

    Reproducibility
    We need shared templates, fixed datasets, versioned benchmarks, and controlled execution rules. Docker freezes dependencies, but it doesn’t enforce these constraints.

    Trust & governance
    Participants can’t rely on opaque, centrally built images. The system must be transparent, auditable, and neutral so everyone can verify what’s being run.
    
    Security
    Containers share the host kernel and aren’t strong isolation boundaries. Evaluating untrusted or risky workloads may require stronger sandboxing.
    
    Verifiability
    API-only access can hide behavior behind routing or safeguards. Safety evaluation often requires running directly on the actual models and systems.
 
## Who benefits?

This is an open-source, non-profit initiative. The streamlined development flow benefits new AI safety researchers and groups, while making the product of their research go further as it can be used more broadly.

Smaller AI labs benefit from secure, privacy-respecting solutions for giving access to IP to 3rd-party safety eval orgs. While 3rd-party safety eval orgs benefit from verifiable, trusted direct access to the AI Assembly. The sprint scope focuses on serving these groups where the value is most immediate.

To understand its greater benefit, it must be seen in the higher-level context: to enable verifiable, standardized AI safety collaboration globally while providing an open, trusted solution for tooling and infrastructure

## Proposed agenda:
- Agree on the technical base (env/langs/tools/etc)
- Gather open-source eval/bench/auto-red-teaming
- Analyze for common properties & abstract/factor them out
- Divide them in to work packages, work in parallel
- Sync, review, refactor
- Integrate in to a unified template
- Test with real or mocked UserComponent eval/bench/auto-red-teaming
- Cycle back as needed
- Write up [Hackathon report](https://docs.google.com/document/d/1Nu8GhsgtLPiT25-3ijmp4UCQGgly3n2l5K2EqN5gE90/edit?usp=sharing)
