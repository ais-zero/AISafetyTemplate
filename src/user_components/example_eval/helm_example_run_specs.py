"""RunSpec function for the example Q&A eval."""

from helm.benchmark.adaptation.adapter_spec import AdapterSpec
from helm.benchmark.adaptation.common_adapter_specs import (
    get_multiple_choice_adapter_spec,
)
from helm.benchmark.metrics.common_metric_specs import get_exact_match_metric_specs
from helm.benchmark.run_spec import RunSpec, run_spec_function
from helm.benchmark.scenarios.scenario import ScenarioSpec


@run_spec_function("example_qa")
def get_example_qa_run_spec() -> RunSpec:
    scenario_spec = ScenarioSpec(
        class_name="example_scenario.ExampleQAScenario",
    )

    adapter_spec: AdapterSpec = get_multiple_choice_adapter_spec(
        method="multiple_choice_joint",
        instructions="Answer the following multiple-choice question by selecting the correct option letter.",
        input_noun="Question",
        output_noun="Answer",
    )

    metric_specs = get_exact_match_metric_specs()

    return RunSpec(
        name="example_qa",
        scenario_spec=scenario_spec,
        adapter_spec=adapter_spec,
        metric_specs=metric_specs,
        groups=["example_qa"],
    )
