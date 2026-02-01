#ifndef CONTROLLER_H
#define CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialization
int controller_init(const char* config_path);
void controller_shutdown();

// Model access
void* controller_get_proxy_client();

// Dataset access
const char* controller_load_dataset(const char* name);

// Load local dataset from offline storage (returns PyObject* list)
// Supports JSONL and JSON formats
// Returns: PyObject* containing list of records, or NULL on error
void* controller_load_local_dataset(const char* dataset_path);

// HELM Integration
// Run a HELM scenario with optional configuration
// Returns: PyObject* containing results dict, or NULL on error
void* controller_run_helm_scenario(const char* scenario_name, void* config);

// Run HELM with a user-provided run spec file
// plugin_path: Path to Python file containing @run_spec_function decorators
// run_spec_name: Name of the run spec to execute (e.g., "example_qa")
// model_name: Model identifier (e.g., "openai/gpt-4o-mini")
// max_instances: Maximum number of instances to evaluate (-1 for all)
// output_path: Path for benchmark output files
// Returns: PyObject* containing results dict, or NULL on error
void* controller_run_helm_benchmark(
    const char* plugin_path,
    const char* run_spec_name,
    const char* model_name,
    int max_instances,
    const char* output_path
);

// Load a HELM scenario from a user-provided scenario file
// Returns: 0 on success, -1 on error
int controller_load_helm_scenario(const char* scenario_path);

// Get HELM benchmark results as JSON string
// Returns: JSON string with results, or NULL on error
const char* controller_get_helm_results(const char* output_path);

// Results
int controller_submit_results(const char* json_results);

// Security
void install_import_hooks();

// Utilities
const char* controller_get_version();
const char* controller_get_last_error();

#ifdef __cplusplus
}
#endif

#endif // CONTROLLER_H
