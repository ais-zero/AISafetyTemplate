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
- Lack of standardization in an international context means more diverse code bases, versioning nightmares, larger bug surface, and inconsistent results & interpretation.
- Templates allow more steamlined development flow, security hardening, restricting potentially unsafe code, syncing on datasets, version-controlled dependencies, and a common environment for easier, better runtime consistency.
- We can enhance multilateral trust by omitting centalized control or sovereignty.
- Docker is not security-hardened enough to realiably contain the code it runs. If the safety work is to be used multilaterally, trusted security is essential.
- Needs to encompass generic AI Assemblies (models/systems/frameworks) while allowing private access to Intellectual Property, directly (not via API) for verifiablility (APIs often use routing, safewalling and safeguards, and allow user targeting, which obscures true safety levels).

## Who benefits from this sprint project?
This is not a commercial product and is apart of an open, non-profit initiative. Neither is it likely to be helpful to existing independent, more nationally-scoped AI safety eval work with proprietary solutions. That isn't it's point.
The streamlined development flow benefits new AI safety researchers and groups more, while making the product of their research go further as it can be used more broadly.

Smaller AI labs benefit from not having to find secure, privacy-respecting solutions for giving access to IP to 3rd-party safety eval orgs. While 3rd-party safety eval orgs benefit from verifiable, trusted direct access to the AI Assembly. Top AI labs are highly likely to be content with their own solutions, so this does not apply at this stage (the sprint scope).

To understand it's greater benefit, it must be seen in the higher-level context - to enable verifiable, standardized AI safety collaboration globally - while providing a open, trusted solution for tooling and infrastructure.

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
