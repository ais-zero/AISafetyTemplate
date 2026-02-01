#include <Python.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

/**
 * HELM Wrapper Module
 *
 * Provides full integration with Stanford HELM (crfm-helm) framework.
 * Wraps HELM's API and exposes it to UserComponents via the Controller.
 *
 * HELM Flow:
 * 1. Load user-provided scenario and run_spec files
 * 2. Configure HELM with Controller-provided model client (proxy)
 * 3. Execute HELM benchmark via helm-run
 * 4. Parse and return results
 */

namespace HELMWrapper {

// Static storage for results
static std::string g_helm_results_json;
static std::string g_helm_error;

// Helper to set error
static void set_helm_error(const std::string& error) {
    g_helm_error = error;
    std::cerr << "[HELM] Error: " << error << std::endl;
}

// Helper to check Python errors and log them
static bool check_python_error() {
    if (PyErr_Occurred()) {
        PyObject *type, *value, *traceback;
        PyErr_Fetch(&type, &value, &traceback);
        PyErr_NormalizeException(&type, &value, &traceback);

        if (value) {
            PyObject* str_exc = PyObject_Str(value);
            if (str_exc) {
                const char* c_str = PyUnicode_AsUTF8(str_exc);
                if (c_str) {
                    set_helm_error(std::string("Python error: ") + c_str);
                }
                Py_DECREF(str_exc);
            }
        }

        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(traceback);
        return true;
    }
    return false;
}

/**
 * Initialize HELM environment
 * Sets up Python paths and imports required modules
 */
int initialize_helm_environment(const char* proxy_url) {
    std::cout << "[HELM] Initializing HELM environment..." << std::endl;

    // Build initialization code
    std::ostringstream init_code;
    init_code << R"(
import sys
import os

# Ensure HELM modules are importable
# HELM is installed via pip (crfm-helm package)

# Set environment variables for HELM
os.environ['HELM_PROXY_URL'] = ')" << proxy_url << R"('

# Import core HELM modules to verify installation
try:
    from helm.benchmark.run import run_benchmarking
    from helm.benchmark.runner import Runner
    from helm.common.authentication import Authentication
    print("[HELM] Core modules imported successfully")
    _helm_available = True
except ImportError as e:
    print(f"[HELM] Warning: Could not import HELM modules: {e}")
    print("[HELM] Make sure crfm-helm is installed: pip install crfm-helm")
    _helm_available = False

# Store availability flag
helm_available = _helm_available
)";

    PyRun_SimpleString(init_code.str().c_str());

    if (check_python_error()) {
        return -1;
    }

    std::cout << "[HELM] Environment initialized" << std::endl;
    return 0;
}

/**
 * Run HELM benchmark with user-provided configuration
 *
 * This executes HELM programmatically rather than via CLI
 */
void* run_helm_benchmark(
    const char* plugin_path,
    const char* run_spec_name,
    const char* model_name,
    int max_instances,
    const char* output_path,
    const char* proxy_url
) {
    std::cout << "[HELM] Running benchmark:" << std::endl;
    std::cout << "[HELM]   Plugin: " << plugin_path << std::endl;
    std::cout << "[HELM]   RunSpec: " << run_spec_name << std::endl;
    std::cout << "[HELM]   Model: " << model_name << std::endl;
    std::cout << "[HELM]   MaxInstances: " << max_instances << std::endl;
    std::cout << "[HELM]   Output: " << output_path << std::endl;

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    // Add parameters to globals
    PyDict_SetItemString(globals, "plugin_path", PyUnicode_FromString(plugin_path));
    PyDict_SetItemString(globals, "run_spec_name", PyUnicode_FromString(run_spec_name));
    PyDict_SetItemString(globals, "model_name", PyUnicode_FromString(model_name));
    PyDict_SetItemString(globals, "max_instances", PyLong_FromLong(max_instances));
    PyDict_SetItemString(globals, "output_path", PyUnicode_FromString(output_path));
    PyDict_SetItemString(globals, "proxy_url", PyUnicode_FromString(proxy_url));

    // HELM execution code
    const char* helm_code = R"(
import sys
import os
import json
import importlib.util
from pathlib import Path

result = {
    'status': 'error',
    'scenario': run_spec_name,
    'model': model_name,
    'metrics': {},
    'instances_evaluated': 0,
    'error': None
}

try:
    # Set up proxy URL for HELM's OpenAI client
    os.environ['OPENAI_API_BASE'] = proxy_url + '/v1'
    os.environ['OPENAI_API_KEY'] = 'dummy-key-for-proxy'

    # Try to import HELM
    try:
        from helm.benchmark.run import run_benchmarking
        from helm.benchmark.presentation.run_entry import RunEntry
        from helm.common.general import ensure_directory_exists
        helm_available = True
    except ImportError as e:
        helm_available = False
        result['error'] = f"HELM not installed: {e}"
        result['status'] = 'helm_not_available'

    if helm_available:
        # Load the user's plugin file to register run specs
        if plugin_path and os.path.exists(plugin_path):
            print(f"[HELM] Loading plugin from: {plugin_path}")

            # Add plugin directory to path
            plugin_dir = os.path.dirname(os.path.abspath(plugin_path))
            if plugin_dir not in sys.path:
                sys.path.insert(0, plugin_dir)

            # Import the plugin module
            spec = importlib.util.spec_from_file_location("user_plugin", plugin_path)
            if spec and spec.loader:
                user_module = importlib.util.module_from_spec(spec)
                sys.modules["user_plugin"] = user_module
                spec.loader.exec_module(user_module)
                print(f"[HELM] Plugin loaded successfully")

        # Create output directory
        ensure_directory_exists(output_path)

        # Build run entry
        run_description = f"{run_spec_name}:model={model_name}"
        run_entries = [RunEntry(description=run_description, priority=1)]

        print(f"[HELM] Running: {run_description}")

        # Execute HELM benchmark
        # Note: In production, we would use more sophisticated configuration
        try:
            from helm.benchmark.runner import Runner
            from helm.benchmark.run_spec import RunSpec
            from helm.benchmark.run_specs.run_spec_factory import get_run_spec_function

            # Get the run spec function
            run_spec_func = get_run_spec_function(run_spec_name)
            if run_spec_func:
                run_spec = run_spec_func()
                print(f"[HELM] Got RunSpec: {run_spec.name}")

                # Create runner and execute
                runner = Runner(
                    output_path=output_path,
                    dry_run=False,
                    skip_instances=False,
                    max_eval_instances=max_instances if max_instances > 0 else None
                )

                # Run the benchmark
                runner.run_one(run_spec)

                result['status'] = 'completed'
                result['instances_evaluated'] = max_instances if max_instances > 0 else -1

                # Try to read results
                results_file = os.path.join(output_path, 'runs', run_spec.name, 'stats.json')
                if os.path.exists(results_file):
                    with open(results_file, 'r') as f:
                        stats = json.load(f)
                        result['metrics'] = stats
            else:
                result['error'] = f"Run spec function '{run_spec_name}' not found"
                result['status'] = 'run_spec_not_found'

        except Exception as e:
            print(f"[HELM] Runner error: {e}")
            result['error'] = str(e)
            result['status'] = 'execution_error'

except Exception as e:
    import traceback
    error_msg = f"{str(e)}\n{traceback.format_exc()}"
    print(f"[HELM] Error: {error_msg}")
    result['error'] = error_msg
    result['status'] = 'error'

print(f"[HELM] Result status: {result['status']}")
)";

    PyRun_String(helm_code, Py_file_input, globals, globals);

    if (check_python_error()) {
        Py_DECREF(globals);
        return nullptr;
    }

    // Get result
    PyObject* result = PyDict_GetItemString(globals, "result");
    if (result) {
        Py_INCREF(result);
        std::cout << "[HELM] Benchmark completed" << std::endl;
        return (void*)result;
    }

    set_helm_error("Failed to get HELM result");
    Py_DECREF(globals);
    return nullptr;
}

/**
 * Run a simple HELM scenario by name
 * Uses default configuration suitable for quick evaluations
 */
void* run_helm_scenario(const char* scenario_name, void* config, const char* proxy_url) {
    std::cout << "[HELM] Running scenario: " << scenario_name << std::endl;

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    PyDict_SetItemString(globals, "scenario_name", PyUnicode_FromString(scenario_name));
    PyDict_SetItemString(globals, "proxy_url", PyUnicode_FromString(proxy_url));

    // Pass config if provided
    if (config) {
        PyDict_SetItemString(globals, "user_config", (PyObject*)config);
    } else {
        Py_INCREF(Py_None);
        PyDict_SetItemString(globals, "user_config", Py_None);
    }

    const char* scenario_code = R"(
import os
import json

result = {
    'status': 'pending',
    'scenario': scenario_name,
    'metrics': {},
    'instances': [],
    'error': None
}

try:
    # Set up proxy
    os.environ['OPENAI_API_BASE'] = proxy_url + '/v1'
    os.environ['OPENAI_API_KEY'] = 'dummy-key-for-proxy'

    # Try HELM import
    try:
        from helm.benchmark.scenarios.scenario import ScenarioSpec
        from helm.benchmark.scenarios.scenario_registry import get_scenario_class
        helm_available = True
    except ImportError:
        helm_available = False

    if helm_available:
        # Try to get the scenario class
        scenario_class = get_scenario_class(scenario_name)
        if scenario_class:
            scenario = scenario_class()
            print(f"[HELM] Loaded scenario: {scenario.name}")
            print(f"[HELM] Description: {scenario.description}")

            # Get instances
            instances = scenario.get_instances("/tmp/helm_output")
            result['instances'] = [
                {
                    'input': inst.input.text if inst.input else '',
                    'references': [ref.output.text for ref in inst.references] if inst.references else []
                }
                for inst in instances[:10]  # Limit for preview
            ]
            result['total_instances'] = len(instances)
            result['status'] = 'loaded'
        else:
            result['error'] = f"Scenario '{scenario_name}' not found in registry"
            result['status'] = 'not_found'
    else:
        # Fallback: Return scenario info without running
        result['status'] = 'helm_not_available'
        result['error'] = 'HELM not installed - install with: pip install crfm-helm'

except Exception as e:
    import traceback
    result['error'] = f"{str(e)}\n{traceback.format_exc()}"
    result['status'] = 'error'

print(f"[HELM] Scenario result: {result['status']}")
)";

    PyRun_String(scenario_code, Py_file_input, globals, globals);

    if (check_python_error()) {
        Py_DECREF(globals);
        return nullptr;
    }

    PyObject* result = PyDict_GetItemString(globals, "result");
    if (result) {
        Py_INCREF(result);
        return (void*)result;
    }

    Py_DECREF(globals);
    return nullptr;
}

/**
 * Load and validate a user-provided HELM scenario file
 */
int load_helm_scenario(const char* scenario_path) {
    std::cout << "[HELM] Loading scenario from: " << scenario_path << std::endl;

    // Check if file exists
    std::ifstream file(scenario_path);
    if (!file.good()) {
        set_helm_error(std::string("Scenario file not found: ") + scenario_path);
        return -1;
    }
    file.close();

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    PyDict_SetItemString(globals, "scenario_path", PyUnicode_FromString(scenario_path));

    const char* load_code = R"(
import sys
import os
import importlib.util

load_success = False
load_error = None

try:
    # Add scenario directory to path
    scenario_dir = os.path.dirname(os.path.abspath(scenario_path))
    if scenario_dir not in sys.path:
        sys.path.insert(0, scenario_dir)

    # Load the scenario module
    spec = importlib.util.spec_from_file_location("user_scenario", scenario_path)
    if spec and spec.loader:
        user_module = importlib.util.module_from_spec(spec)
        sys.modules["user_scenario"] = user_module
        spec.loader.exec_module(user_module)

        # Verify it contains a Scenario subclass
        from helm.benchmark.scenarios.scenario import Scenario

        found_scenario = False
        for name in dir(user_module):
            obj = getattr(user_module, name)
            if isinstance(obj, type) and issubclass(obj, Scenario) and obj != Scenario:
                print(f"[HELM] Found scenario class: {name}")
                found_scenario = True

        if found_scenario:
            load_success = True
            print("[HELM] Scenario loaded successfully")
        else:
            load_error = "No Scenario subclass found in file"
    else:
        load_error = "Could not create module spec"

except Exception as e:
    import traceback
    load_error = f"{str(e)}\n{traceback.format_exc()}"

if load_error:
    print(f"[HELM] Load error: {load_error}")
)";

    PyRun_String(load_code, Py_file_input, globals, globals);

    if (check_python_error()) {
        Py_DECREF(globals);
        return -1;
    }

    PyObject* success = PyDict_GetItemString(globals, "load_success");
    bool loaded = success && PyObject_IsTrue(success);

    if (!loaded) {
        PyObject* error = PyDict_GetItemString(globals, "load_error");
        if (error && PyUnicode_Check(error)) {
            set_helm_error(PyUnicode_AsUTF8(error));
        }
    }

    Py_DECREF(globals);
    return loaded ? 0 : -1;
}

/**
 * Get HELM results from output directory as JSON
 */
const char* get_helm_results(const char* output_path) {
    std::cout << "[HELM] Reading results from: " << output_path << std::endl;

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    PyDict_SetItemString(globals, "output_path", PyUnicode_FromString(output_path));

    const char* results_code = R"(
import os
import json
from pathlib import Path

results_json = "{}"

try:
    output_dir = Path(output_path)

    # Collect all results
    all_results = {
        'runs': [],
        'summary': {}
    }

    runs_dir = output_dir / 'runs'
    if runs_dir.exists():
        for run_dir in runs_dir.iterdir():
            if run_dir.is_dir():
                run_data = {'name': run_dir.name}

                # Read stats.json if exists
                stats_file = run_dir / 'stats.json'
                if stats_file.exists():
                    with open(stats_file) as f:
                        run_data['stats'] = json.load(f)

                # Read run_spec.json if exists
                spec_file = run_dir / 'run_spec.json'
                if spec_file.exists():
                    with open(spec_file) as f:
                        run_data['run_spec'] = json.load(f)

                all_results['runs'].append(run_data)

    # Read benchmark summary if exists
    summary_file = output_dir / 'benchmark_output' / 'summary.json'
    if summary_file.exists():
        with open(summary_file) as f:
            all_results['summary'] = json.load(f)

    results_json = json.dumps(all_results, indent=2)
    print(f"[HELM] Found {len(all_results['runs'])} runs")

except Exception as e:
    results_json = json.dumps({'error': str(e)})
    print(f"[HELM] Error reading results: {e}")
)";

    PyRun_String(results_code, Py_file_input, globals, globals);

    PyObject* json_result = PyDict_GetItemString(globals, "results_json");
    if (json_result && PyUnicode_Check(json_result)) {
        g_helm_results_json = PyUnicode_AsUTF8(json_result);
    }

    Py_DECREF(globals);
    return g_helm_results_json.c_str();
}

/**
 * Get the last HELM error message
 */
const char* get_helm_error() {
    return g_helm_error.c_str();
}

} // namespace HELMWrapper
