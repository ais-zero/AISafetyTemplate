"""
UserComponent Template

This is a clean template for creating safety evaluations.
Copy this file and implement your evaluation logic.

IMPORTANT: You can ONLY import the Controller!
All other functionality is provided through the Controller API.
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

    # 2. Your evaluation logic goes here
    # Example: Load a dataset
    # dataset_name = controller.load_dataset("JailbreakBench/JBB-Behaviors")

    # Example: Get model client
    # client = controller.get_model_client()
    # response = client.complete("Test prompt")

    # Example: Run HELM scenario (if implemented)
    # results = controller.run_helm_scenario("harmbench", config)

    # 3. Compute your metrics
    # This is where you implement your specific evaluation logic
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
            "description": "Example evaluation template"
        }
    }

    # 4. Submit results
    controller.submit_results(results)

    # 5. Cleanup
    controller.shutdown()

    print("=" * 60)
    print("[UserComponent] Evaluation complete!")
    print("=" * 60)


if __name__ == "__main__":
    main()
