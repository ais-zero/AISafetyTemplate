For our eval, bench, & red-teaming investigation data (public external links are okay for this, e.g. Google Sheets).
We need to compile and analyze data on which environments, languages, libraries, tools & use cases (what model/system/framework they apply to).

## Pre-Analysis based on investigation so far:
After getting in to this investigation and analyzing Eval repos & resources, we are re-considering existing projects as the Template core.

#### Initial strong reasons for not doing:
- sovereignty concerns, international adoption & trust politics
- having a stronger environment, e.g. using a formal, typed-checked, & compiled language
- simplier interop with the high-level initiative's platform, which won't use Python

It was known that it would be an enormous endeavor even if incremental (once scoped beyond this sprint) - after deeper investigation, comparing this to the countering endeavor's need to reproduce/translate existing safety work, it is imposes much more community effort. In addtion, it is less fitting after settling on a Python environment (due to it's popular use in AI safety, along with Python's ML ecosystem and tooling).

#### EleutherAI's LM-Evaluation-Harness & Stanford's HELM are top contenders based on:
- being open-source non-sovereignty/national/commercial projects
- current adoption & existing project-specific safety work
- project's leadership, purpose & vision
- AI model interface flexibility
- conflict of interests

##### LM-Evaluation-Harness:
- community developed
- industry adoption (NVIDIA, Cohere, Mistral) & more widely used by safety community
- no/less support for red-teaming, CCBRN, jailbreak detection (must be extended)
- better performance
- good ease of use for creating safety work
- more metrics over reporting
- 60+ benchmarks, large amount of variants

##### HELM:
- academically developed with supporting papers
- industry adoption & use (IBM, MMLU-Pro and GPQA, some lab use for pre-release safety checks)
- steeper ease-of-use for creating safety work
- better existing coverage on CCBRN & jailbreaking with automated judges (e.g., GPT-4o) 
- support for red-teaming
- less performant but has lightweight versions for focused domains
- more comprehensive multi-metric reporting
- 42+ scenarios, multi-metric per scenario

Due to more critical security targeting, usability for red-teaming, and multi-metric scenario reporting vs merely benchmarks - it is leaning towards HELM.
