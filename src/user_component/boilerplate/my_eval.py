"""
UserComponent Boilerplate

This is boilerplate code for creating safety evaluations.
Copy this file and implement your evaluation logic.

IMPORTANT: You can ONLY import the Controller!
All other functionality is provided through the Controller API.

OFFLINE MODE:
This eval container has NO INTERNET ACCESS. All datasets and dependencies
must be pre-downloaded before building the container:
- Datasets: /app/offline_datasets/ (run download_datasets.sh)
- Python packages: /app/offline_packages/ (run download_offline_packages.sh)

HELM Integration:
The Controller provides full HELM (Holistic Evaluation of Language Models) integration.
You can run HELM benchmarks in three ways:
1. run_helm_scenario() - Run a built-in HELM scenario by name
2. run_helm_benchmark() - Run with your custom run spec and scenario files
3. Manual evaluation using get_model_client() with HELM-style prompts

Dataset Loading:
The Controller provides load_dataset() for loading offline datasets.
Supports JSONL and JSON formats automatically.
"""

from controller import Controller


def main():
    """Main evaluation logic"""

    # 1. Initialize Controller
    controller = Controller()
    controller.init('/app/config.yaml')

    print("=" * 60)
    print(f"[UserComponent] Controller initialized")
    print(f"[UserComponent] Controller version: {controller.get_version()}")
    print("=" * 60)

    # =========================================================================
    # OPTION A: Run a built-in HELM scenario
    # =========================================================================
    # result = controller.run_helm_scenario("mmlu")
    # print(f"[HELM] Scenario result: {result}")

    # =========================================================================
    # OPTION B: Run HELM with your custom run spec file
    # =========================================================================
    # This is the recommended approach for custom evaluations.
    # See helm-example-&-sandboxing/user_components/example_eval/ for examples.
    #
    # result = controller.run_helm_benchmark(
    #     plugin_path="/app/eval/my_run_specs.py",  # Your @run_spec_function file
    #     run_spec_name="my_custom_eval",            # Name from decorator
    #     model_name="openai/smollm3-3b",           # Model to evaluate
    #     max_instances=10,                          # -1 for all
    #     output_path="/app/benchmark_output"
    # )
    # print(f"[HELM] Benchmark result: {result}")
    #
    # # Get detailed results
    # helm_results = controller.get_helm_results("/app/benchmark_output")
    # print(f"[HELM] Detailed results: {helm_results}")

    # =========================================================================
    # OPTION C: Manual evaluation with model client (non-HELM)
    # =========================================================================
    # client = controller.get_model_client()
    # response = client.complete("What is the capital of France?")
    # print(f"Response: {response}")

    # =========================================================================
    # OPTION D: Load dataset and run custom evaluation (OFFLINE)
    # =========================================================================
    # NOTE: Datasets must be pre-downloaded to /app/offline_datasets/
    # Run download_datasets.sh before building the container
    #
    # dataset = controller.load_dataset("JailbreakBench/JBB-Behaviors")
    #
    # if not dataset:
    #     print("[Eval] Dataset not found - using fallback data")
    #     dataset = [{"prompt": "Test prompt 1"}, {"prompt": "Test prompt 2"}]
    #
    # for item in dataset:
    #     prompt = item.get('Goal', item.get('prompt', ''))
    #     # Process each item...
    #     pass

    # 2. Your evaluation logic
    # Replace this section with your actual evaluation code
    results = {
        "version": "0.1.0",
        "scenarios": [
            {
                "name": "my_scenario",
                "metrics": {
                    "metric_1": 0.0,
                    "metric_2": 0.0,
                    "num_samples": 0
                }
            }
        ],
        "metadata": {
            "controller_version": controller.get_version(),
            "description": "Example evaluation template",
            "offline_mode": True
        }
    }

    # 3. Submit results
    controller.submit_results(results)

    # 4. Cleanup
    controller.shutdown()

    print("=" * 60)
    print("[UserComponent] Evaluation complete!")
    print("=" * 60)


def run_helm_example(controller):
    """
    Example: Running HELM with custom scenario

    To use this, create two files:
    1. my_scenario.py - Your Scenario subclass
    2. my_run_specs.py - Your @run_spec_function decorator
    """
    # First, load your custom scenario (validates the file)
    controller.load_helm_scenario("/app/eval/my_scenario.py")

    # Then run the benchmark
    result = controller.run_helm_benchmark(
        plugin_path="/app/eval/my_run_specs.py",
        run_spec_name="my_custom_eval",
        model_name="openai/smollm3-3b",
        max_instances=10,
        output_path="/app/benchmark_output"
    )

    # Check result status
    if result.get('status') == 'completed':
        print(f"[HELM] Success! Evaluated {result.get('instances_evaluated')} instances")
        print(f"[HELM] Metrics: {result.get('metrics')}")
    elif result.get('status') == 'helm_not_available':
        print("[HELM] HELM not installed. Install with: pip install crfm-helm")
    else:
        print(f"[HELM] Error: {result.get('error')}")

    # Get detailed results from output files
    helm_results = controller.get_helm_results("/app/benchmark_output")
    return helm_results


if __name__ == "__main__":
    main()
