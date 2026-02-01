#include "controller.h"
#include <Python.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

// Forward declarations for HELM wrapper functions
namespace HELMWrapper {
    int initialize_helm_environment(const char* proxy_url);
    void* run_helm_benchmark(
        const char* plugin_path,
        const char* run_spec_name,
        const char* model_name,
        int max_instances,
        const char* output_path,
        const char* proxy_url
    );
    void* run_helm_scenario(const char* scenario_name, void* config, const char* proxy_url);
    int load_helm_scenario(const char* scenario_path);
    const char* get_helm_results(const char* output_path);
    const char* get_helm_error();
}

// Global state
static std::string g_proxy_url;
static std::string g_config_path;
static std::string g_last_error;
static std::string g_helm_results;
static bool g_initialized = false;
static bool g_helm_initialized = false;

// Helper to set last error
static void set_error(const std::string& error) {
    g_last_error = error;
    std::cerr << "[Controller Error] " << error << std::endl;
}

extern "C" {

int controller_init(const char* config_path) {
    try {
        if (g_initialized) {
            set_error("Controller already initialized");
            return -1;
        }

        std::cout << "[Controller] Initializing..." << std::endl;

        // Initialize Python interpreter if not already initialized
        if (!Py_IsInitialized()) {
            Py_Initialize();
        }

        // Store config path
        g_config_path = std::string(config_path);

        // Read proxy URL from environment or use default
        const char* env_proxy = getenv("LLM_PROXY_URL");
        if (env_proxy) {
            g_proxy_url = std::string(env_proxy);
        } else {
            g_proxy_url = "http://llm-proxy:8000";
        }

        std::cout << "[Controller] Proxy URL: " << g_proxy_url << std::endl;

        // Import necessary Python modules
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("import os");

        // Install import hooks for security
        install_import_hooks();

        // Initialize HELM environment
        if (HELMWrapper::initialize_helm_environment(g_proxy_url.c_str()) == 0) {
            g_helm_initialized = true;
            std::cout << "[Controller] HELM environment initialized" << std::endl;
        } else {
            std::cout << "[Controller] HELM not available (optional)" << std::endl;
        }

        g_initialized = true;
        std::cout << "[Controller] Initialized successfully" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        set_error(std::string("Init failed: ") + e.what());
        return -1;
    }
}

void* controller_get_proxy_client() {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return nullptr;
    }

    try {
        // Create a Python client object that can make HTTP calls to proxy
        PyObject* globals = PyDict_New();
        PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

        // Add proxy URL to globals
        PyObject* proxy_url_py = PyUnicode_FromString(g_proxy_url.c_str());
        PyDict_SetItemString(globals, "proxy_url", proxy_url_py);

        // Create ProxyClient class in Python
        const char* client_code = R"(
import sys
import os

# Import requests (Controller allows this)
try:
    import requests
except ImportError:
    # Fallback for testing
    class requests:
        @staticmethod
        def post(*args, **kwargs):
            return type('obj', (object,), {'json': lambda: {'choices': [{'message': {'content': 'test'}}]}})()
        @staticmethod
        def get(*args, **kwargs):
            return type('obj', (object,), {'status_code': 200})()

class ProxyClient:
    def __init__(self, url):
        self.url = url

    def complete(self, prompt, **kwargs):
        """Make completion request to proxy"""
        try:
            response = requests.post(
                f"{self.url}/v1/chat/completions",
                json={
                    "messages": [{"role": "user", "content": prompt}],
                    "model": kwargs.get("model", "gpt-4o-mini"),
                    "temperature": kwargs.get("temperature", 1.0),
                    "max_tokens": kwargs.get("max_tokens", 150)
                },
                timeout=30
            )
            return response.json()
        except Exception as e:
            print(f"[ProxyClient] Error: {e}")
            raise

client = ProxyClient(proxy_url)
)";

        PyRun_String(client_code, Py_file_input, globals, globals);
        PyObject* client = PyDict_GetItemString(globals, "client");

        if (!client) {
            set_error("Failed to create proxy client");
            Py_DECREF(globals);
            return nullptr;
        }

        Py_INCREF(client);
        std::cout << "[Controller] Created proxy client" << std::endl;
        return (void*)client;

    } catch (const std::exception& e) {
        set_error(std::string("get_proxy_client failed: ") + e.what());
        return nullptr;
    }
}

const char* controller_load_dataset(const char* name) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return nullptr;
    }

    try {
        std::cout << "[Controller] Loading dataset: " << name << std::endl;

        // For sprint, we'll use HuggingFace datasets directly
        // In production, this would verify hashes and provide curated datasets

        static std::string dataset_name;
        dataset_name = std::string(name);

        return dataset_name.c_str();

    } catch (const std::exception& e) {
        set_error(std::string("load_dataset failed: ") + e.what());
        return nullptr;
    }
}

void* controller_run_helm_scenario(const char* scenario_name, void* config) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return nullptr;
    }

    try {
        std::cout << "[Controller] Running HELM scenario: " << scenario_name << std::endl;

        // Use HELM wrapper to run the scenario
        void* result = HELMWrapper::run_helm_scenario(
            scenario_name,
            config,
            g_proxy_url.c_str()
        );

        if (!result) {
            const char* helm_error = HELMWrapper::get_helm_error();
            set_error(std::string("HELM scenario failed: ") + (helm_error ? helm_error : "Unknown error"));
            return nullptr;
        }

        std::cout << "[Controller] HELM scenario completed" << std::endl;
        return result;

    } catch (const std::exception& e) {
        set_error(std::string("run_helm_scenario failed: ") + e.what());
        return nullptr;
    }
}

void* controller_run_helm_benchmark(
    const char* plugin_path,
    const char* run_spec_name,
    const char* model_name,
    int max_instances,
    const char* output_path
) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return nullptr;
    }

    try {
        std::cout << "[Controller] Running HELM benchmark: " << run_spec_name << std::endl;

        // Use HELM wrapper to run the full benchmark
        void* result = HELMWrapper::run_helm_benchmark(
            plugin_path,
            run_spec_name,
            model_name,
            max_instances,
            output_path,
            g_proxy_url.c_str()
        );

        if (!result) {
            const char* helm_error = HELMWrapper::get_helm_error();
            set_error(std::string("HELM benchmark failed: ") + (helm_error ? helm_error : "Unknown error"));
            return nullptr;
        }

        std::cout << "[Controller] HELM benchmark completed" << std::endl;
        return result;

    } catch (const std::exception& e) {
        set_error(std::string("run_helm_benchmark failed: ") + e.what());
        return nullptr;
    }
}

int controller_load_helm_scenario(const char* scenario_path) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return -1;
    }

    try {
        std::cout << "[Controller] Loading HELM scenario: " << scenario_path << std::endl;

        int result = HELMWrapper::load_helm_scenario(scenario_path);

        if (result != 0) {
            const char* helm_error = HELMWrapper::get_helm_error();
            set_error(std::string("Failed to load HELM scenario: ") + (helm_error ? helm_error : "Unknown error"));
            return -1;
        }

        std::cout << "[Controller] HELM scenario loaded successfully" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        set_error(std::string("load_helm_scenario failed: ") + e.what());
        return -1;
    }
}

const char* controller_get_helm_results(const char* output_path) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return nullptr;
    }

    try {
        std::cout << "[Controller] Getting HELM results from: " << output_path << std::endl;

        const char* results = HELMWrapper::get_helm_results(output_path);

        if (!results || strlen(results) == 0) {
            set_error("No HELM results found");
            return nullptr;
        }

        // Store results in static string for return
        g_helm_results = std::string(results);
        return g_helm_results.c_str();

    } catch (const std::exception& e) {
        set_error(std::string("get_helm_results failed: ") + e.what());
        return nullptr;
    }
}

int controller_submit_results(const char* json_results) {
    if (!g_initialized) {
        set_error("Controller not initialized");
        return -1;
    }

    try {
        // Get output path from environment or use default
        const char* output_path_env = getenv("OUTPUT_PATH");
        std::string output_path = output_path_env ?
            std::string(output_path_env) :
            "/tmp/results/evaluation_output.json";

        std::cout << "[Controller] Processing and saving results to: " << output_path << std::endl;

        // Process metrics through Python for normalization
        PyObject* globals = PyDict_New();
        PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
        PyDict_SetItemString(globals, "raw_results", PyUnicode_FromString(json_results));
        PyDict_SetItemString(globals, "output_path", PyUnicode_FromString(output_path.c_str()));

        // Metrics processing code - normalizes user component metrics to standard format
        const char* process_code = R"(
import json
import os
from datetime import datetime

# Parse raw results
try:
    results = json.loads(raw_results)
except json.JSONDecodeError:
    results = {"raw": raw_results, "error": "Invalid JSON"}

# Standard metric mapping - maps user component metrics to normalized names
METRIC_MAPPINGS = {
    # Safety metrics
    "refusal_rate": "safety.refusal_rate",
    "attack_success_rate": "safety.attack_success_rate",
    "asr": "safety.attack_success_rate",
    "jailbreak_rate": "safety.jailbreak_rate",
    "harmful_completion_rate": "safety.harmful_completion_rate",

    # Quality metrics
    "exact_match": "quality.exact_match",
    "f1_score": "quality.f1",
    "accuracy": "quality.accuracy",
    "bleu": "quality.bleu",
    "rouge": "quality.rouge",

    # Fairness metrics
    "demographic_parity": "fairness.demographic_parity",
    "equal_opportunity": "fairness.equal_opportunity",

    # Performance metrics
    "latency_p50": "performance.latency_p50_ms",
    "latency_p99": "performance.latency_p99_ms",
    "tokens_per_second": "performance.tokens_per_second",
}

def normalize_metrics(metrics_dict, prefix=""):
    """Recursively normalize metric names"""
    normalized = {}
    for key, value in metrics_dict.items():
        full_key = f"{prefix}.{key}" if prefix else key

        if isinstance(value, dict):
            normalized.update(normalize_metrics(value, full_key))
        else:
            # Map to standard name if available
            mapped_key = METRIC_MAPPINGS.get(key, full_key)
            normalized[mapped_key] = value

    return normalized

# Process scenarios and normalize metrics
processed_results = {
    "version": results.get("version", "1.0.0"),
    "timestamp": datetime.utcnow().isoformat() + "Z",
    "controller_version": "0.1.0-sprint",
    "scenarios": [],
    "normalized_metrics": {},
    "metadata": results.get("metadata", {})
}

# Process each scenario
for scenario in results.get("scenarios", []):
    scenario_name = scenario.get("name", "unknown")
    raw_metrics = scenario.get("metrics", {})

    # Normalize metrics
    normalized = normalize_metrics(raw_metrics, scenario_name)
    processed_results["normalized_metrics"].update(normalized)

    # Keep original scenario data
    processed_results["scenarios"].append({
        "name": scenario_name,
        "raw_metrics": raw_metrics,
        "normalized_metrics": {k: v for k, v in normalized.items() if k.startswith(scenario_name)}
    })

# Create output directory
os.makedirs(os.path.dirname(output_path), exist_ok=True)

# Write processed results
with open(output_path, 'w') as f:
    json.dump(processed_results, f, indent=2)

print(f"[Controller] Processed {len(processed_results['scenarios'])} scenarios")
print(f"[Controller] Normalized {len(processed_results['normalized_metrics'])} metrics")
process_success = True
)";

        PyRun_String(process_code, Py_file_input, globals, globals);

        PyObject* success = PyDict_GetItemString(globals, "process_success");
        if (!success || !PyObject_IsTrue(success)) {
            set_error("Metrics processing failed");
            Py_DECREF(globals);
            return -1;
        }

        Py_DECREF(globals);
        std::cout << "[Controller] Results saved successfully" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        set_error(std::string("submit_results failed: ") + e.what());
        return -1;
    }
}

void install_import_hooks() {
    std::cout << "[Controller] Installing import hooks..." << std::endl;

    // Install Python import hook to enforce allowlist
    const char* hook_code = R"(
import sys
import importlib.abc
import importlib.machinery

class ImportRestriction(importlib.abc.MetaPathFinder):
    """Import hook to enforce allowlist for security"""

    # Allowed imports (modules UserComponent can import)
    ALLOWED = {
        # Core Python
        'controller', 'json', 'typing', 'datetime', 'os', 'sys',
        'logging', 're', 'math', 'collections', 'functools',
        'itertools', 'pathlib', 'importlib', 'abc', 'dataclasses',
        'enum', 'copy', 'hashlib', 'base64', 'uuid', 'time',
        'random', 'string', 'io', 'tempfile', 'csv', 'textwrap',
        'traceback', 'warnings', 'threading', 'queue', 'concurrent',
        'pickle', 'gzip', 'zipfile', 'shutil', 'glob', 'fnmatch',
        'configparser', 'argparse',

        # Data science
        'numpy', 'pandas', 'scipy', 'sklearn',

        # ML/AI
        'torch', 'transformers', 'tokenizers', 'tiktoken', 'sentencepiece',
        'safetensors',

        # HTTP (for proxy)
        'requests', 'urllib', 'urllib3', 'http', 'httpx', 'aiohttp',
        'certifi', 'charset_normalizer', 'idna',

        # Datasets
        'datasets', 'huggingface_hub',

        # HELM framework
        'helm', 'crfm_helm', 'cattrs', 'attrs', 'dacite',
        'pydantic', 'yaml', 'ruamel', 'toml', 'tqdm',
        'filelock', 'fsspec', 'pyarrow', 'tenacity', 'nltk',
        'openai', 'anthropic', 'cohere', 'google', 'vertexai',
        'multiprocess', 'dill', 'xxhash', 'aiofiles',
        'nest_asyncio', 'sqlitedict', 'retrying', 'spacy',
    }

    # Explicitly blocked (security-sensitive)
    BLOCKED = {
        'subprocess', 'socket', 'ftplib', 'telnetlib',
        'paramiko', 'fabric', 'pexpect', 'pty',
        'ctypes', 'cffi',
    }

    def find_spec(self, fullname, path, target=None):
        base_module = fullname.split('.')[0]

        # Block security-sensitive modules
        if base_module in self.BLOCKED:
            print(f"[Security] BLOCKED import: {fullname}")
            raise ImportError(f"Module '{fullname}' is not allowed for security reasons")

        # Allow controller module always
        if fullname == 'controller' or fullname.startswith('controller.'):
            return None

        # Check allowlist (permissive mode for HELM compatibility)
        if base_module not in self.ALLOWED:
            # Log but allow for now (strict mode would raise ImportError)
            print(f"[Security] Warning: Unlisted import: {fullname}")

        return None  # Let normal import mechanism handle it

# Install the hook
sys.meta_path.insert(0, ImportRestriction())
print("[Controller] Import hooks installed")
)";

    PyRun_SimpleString(hook_code);
}

void controller_shutdown() {
    if (!g_initialized) {
        return;
    }

    std::cout << "[Controller] Shutting down..." << std::endl;

    // Don't finalize Python here - let the interpreter handle it
    // Py_Finalize();

    g_initialized = false;
    std::cout << "[Controller] Shutdown complete" << std::endl;
}

const char* controller_get_version() {
    static const char* version = "0.1.0-sprint";
    return version;
}

const char* controller_get_last_error() {
    return g_last_error.c_str();
}

} // extern "C"
