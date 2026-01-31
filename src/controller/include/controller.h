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

// HELM wrapper
void* controller_run_helm_scenario(const char* scenario_name, void* config);

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
