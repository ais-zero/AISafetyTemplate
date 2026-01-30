# AISafetyTemplate

Welcome to the Hackathon repo!
It will serve to coordinate development and organize our findings.

## Goal
To develop a working prototype for standardized software template(s) for AI Safety Evals, Benches and/or Automated Red-Teaming.
![alt text](https://github.com/davidg-apart/AISafetyTemplate/blob/aca0e653e092e36f35b6aed5b0e22478a2f53acc/flow-hackathon.png)

## Purpose:
To make post-compute AI safety work more collaborative & standardized. We are not creating the safety work itself rather the engineering to enable standardized collaboration internationally.
Not to replace existing safety, governance, or policy initiatives but to united them with common infrastructure that is international, non-sovereign secure, verifiable, distributed, private where needed & transparent as can be. This is the greater conceptual mission, which will help orient you as we collaborate in this sprint.

## Deliverable:
A verifiable software template(s) for post-compute, non-inference-time AI safety work, containerized and ready to be run inside a Trusted Execution Environment.
Something along the lines of:
![template idea](https://github.com/davidg-apart/AISafetyTemplate/blob/aca0e653e092e36f35b6aed5b0e22478a2f53acc/template-idea.png)
The Controller serves as the only external dependancy for the UserComponent. It orchestrates the environment, verifies UserComponent code, handles external interfacing & containerization.
Using C/Rust for it adds security, performance & precision, referenced from the UserComponent (a Python package) as a dll/binary.
For this sprint, the AI Assembly (model/system/framework) will be mocked and there will be no blockchain or TEE components. None of this is set in stone, let's discuss.

## Proposed agenda:
- Agree on the technical base (env/langs/tools/etc)
- Gather open-source eval/bench/auto-red-teaming
- Analyze for common properties & abstract/factor them out
- Divide them in to work packages, work in parallel
- Sync, review, refactor
- Integrate in to a unified template
- Cycle back as needed
- Write up Hackathon report
