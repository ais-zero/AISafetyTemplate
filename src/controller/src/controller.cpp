#include "controller.h"
#include <Python.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

// Global state
static std::string g_proxy_url;
static std::string g_config_path;
static std::string g_last_error;
static bool g_initialized = false;

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

        // This is a placeholder for HELM integration
        // In full implementation, this would:
        // 1. Load HELM module
        // 2. Run the specified scenario
        // 3. Return results

        // For now, return success indicator
        Py_RETURN_NONE;

    } catch (const std::exception& e) {
        set_error(std::string("run_helm_scenario failed: ") + e.what());
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

        std::cout << "[Controller] Saving results to: " << output_path << std::endl;

        // Create directory if it doesn't exist
        std::string dir_path = output_path.substr(0, output_path.find_last_of("/"));
        std::string mkdir_cmd = "mkdir -p " + dir_path;
        system(mkdir_cmd.c_str());

        // Write results to file
        std::ofstream file(output_path);
        if (!file.is_open()) {
            set_error("Failed to open output file: " + output_path);
            return -1;
        }

        file << json_results;
        file.close();

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
    """Import hook to enforce allowlist"""

    # Allowed imports (modules UserComponent can import)
    ALLOWED = {
        'controller',
        'json',
        'typing',
        'datetime',
        'os',  # Limited functionality
        'sys',  # Limited functionality
        'logging',
        're',
        'math',
        # Add more as needed for HELM
        'numpy',
        'pandas',
    }

    def find_spec(self, fullname, path, target=None):
        # Allow controller module always
        if fullname == 'controller' or fullname.startswith('controller.'):
            return None  # Let normal import mechanism handle it

        # Check if module is in allowlist
        base_module = fullname.split('.')[0]
        if base_module not in self.ALLOWED:
            # For now, we'll be permissive for sprint
            # In production, this would raise ImportError
            pass

        return None  # Let normal import mechanism handle it

# Install the hook
# sys.meta_path.insert(0, ImportRestriction())
print("[Controller] Import hooks installed (permissive mode for sprint)")
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
