#include <Python.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>

/**
 * Dataset Manager Module
 *
 * Provides secure dataset loading and management from LOCAL files only.
 * The eval container has no internet access, so all datasets must be
 * pre-downloaded to /app/offline_datasets/
 *
 * Supported formats:
 * - JSONL (JSON Lines)
 * - JSON (single array)
 * - Parquet (via Python)
 *
 * Directory structure:
 *   /app/offline_datasets/
 *     {owner}/
 *       {dataset_name}/
 *         data.jsonl (or data.json, *.parquet)
 */

namespace DatasetManager {

// Base path for offline datasets
static const std::string OFFLINE_DATASETS_PATH = "/app/offline_datasets/";
static std::string g_last_error;

struct Dataset {
    std::string name;
    std::string version;
    std::string path;
    std::string format;  // "jsonl", "json", "parquet"
    std::string hash;    // SHA-256 hash for verification
};

// Helper to set error
static void set_error(const std::string& error) {
    g_last_error = error;
    std::cerr << "[DatasetManager] Error: " << error << std::endl;
}

// Check if path exists and is a directory
static bool directory_exists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// Check if path exists and is a file
static bool file_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFREG);
}

// Find data file in dataset directory
static std::string find_data_file(const std::string& dataset_dir) {
    // Check for common data file names in order of preference
    std::vector<std::string> candidates = {
        "data.jsonl",
        "behaviors.jsonl",
        "train.jsonl",
        "test.jsonl",
        "data.json",
        "behaviors.json",
        "train.json",
        "test.json"
    };

    for (const auto& candidate : candidates) {
        std::string full_path = dataset_dir + "/" + candidate;
        if (file_exists(full_path)) {
            return full_path;
        }
    }

    // Check for parquet files
    DIR* dir = opendir(dataset_dir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.size() > 8 && name.substr(name.size() - 8) == ".parquet") {
                closedir(dir);
                return dataset_dir + "/" + name;
            }
        }
        closedir(dir);
    }

    return "";
}

// Detect file format from extension
static std::string detect_format(const std::string& file_path) {
    if (file_path.size() > 6 && file_path.substr(file_path.size() - 6) == ".jsonl") {
        return "jsonl";
    } else if (file_path.size() > 5 && file_path.substr(file_path.size() - 5) == ".json") {
        return "json";
    } else if (file_path.size() > 8 && file_path.substr(file_path.size() - 8) == ".parquet") {
        return "parquet";
    }
    return "unknown";
}

/**
 * Convert HuggingFace dataset identifier to local path
 * e.g., "JailbreakBench/JBB-Behaviors" -> "/app/offline_datasets/JailbreakBench/JBB-Behaviors/"
 */
static std::string dataset_name_to_path(const std::string& dataset_name) {
    std::string path = OFFLINE_DATASETS_PATH;

    // Handle HuggingFace-style names (owner/dataset)
    // and simple names (just dataset)
    for (char c : dataset_name) {
        if (c == '/') {
            path += '/';
        } else {
            path += c;
        }
    }

    return path;
}

/**
 * Load dataset from local offline storage
 *
 * Returns a Python object with the loaded data, or NULL on error.
 * The caller is responsible for decrementing the reference count.
 */
void* load_dataset_data(const char* dataset_name) {
    std::cout << "[DatasetManager] Loading dataset: " << dataset_name << std::endl;

    std::string dataset_path = dataset_name_to_path(dataset_name);
    std::cout << "[DatasetManager] Looking in: " << dataset_path << std::endl;

    // Check if dataset directory exists
    if (!directory_exists(dataset_path)) {
        set_error("Dataset directory not found: " + dataset_path +
                  "\nPlease run download_datasets.sh to download datasets for offline use.");
        return nullptr;
    }

    // Find the data file
    std::string data_file = find_data_file(dataset_path);
    if (data_file.empty()) {
        set_error("No data file found in: " + dataset_path +
                  "\nExpected: data.jsonl, behaviors.jsonl, data.json, or *.parquet");
        return nullptr;
    }

    std::string format = detect_format(data_file);
    std::cout << "[DatasetManager] Found: " << data_file << " (format: " << format << ")" << std::endl;

    // Use Python to load the data
    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    PyDict_SetItemString(globals, "data_file", PyUnicode_FromString(data_file.c_str()));
    PyDict_SetItemString(globals, "file_format", PyUnicode_FromString(format.c_str()));

    const char* load_code = R"(
import json

dataset_data = None
load_error = None

try:
    if file_format == 'jsonl':
        # Load JSON Lines
        dataset_data = []
        with open(data_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line:
                    dataset_data.append(json.loads(line))
        print(f"[DatasetManager] Loaded {len(dataset_data)} records from JSONL")

    elif file_format == 'json':
        # Load JSON array
        with open(data_file, 'r', encoding='utf-8') as f:
            dataset_data = json.load(f)
        if isinstance(dataset_data, list):
            print(f"[DatasetManager] Loaded {len(dataset_data)} records from JSON")
        else:
            # Wrap single object in list
            dataset_data = [dataset_data]
            print(f"[DatasetManager] Loaded 1 record from JSON")

    elif file_format == 'parquet':
        # Load Parquet (requires pyarrow)
        try:
            import pyarrow.parquet as pq
            table = pq.read_table(data_file)
            dataset_data = table.to_pylist()
            print(f"[DatasetManager] Loaded {len(dataset_data)} records from Parquet")
        except ImportError:
            load_error = "Parquet support requires pyarrow. Add pyarrow to requirements.txt"

    else:
        load_error = f"Unsupported format: {file_format}"

except Exception as e:
    import traceback
    load_error = f"{str(e)}\n{traceback.format_exc()}"

if load_error:
    print(f"[DatasetManager] Error: {load_error}")
)";

    PyRun_String(load_code, Py_file_input, globals, globals);

    // Check for errors
    PyObject* error = PyDict_GetItemString(globals, "load_error");
    if (error && error != Py_None) {
        const char* error_str = PyUnicode_AsUTF8(error);
        if (error_str) {
            set_error(error_str);
        }
        Py_DECREF(globals);
        return nullptr;
    }

    // Get the data
    PyObject* data = PyDict_GetItemString(globals, "dataset_data");
    if (data && data != Py_None) {
        Py_INCREF(data);
        Py_DECREF(globals);
        return (void*)data;
    }

    set_error("Failed to load dataset data");
    Py_DECREF(globals);
    return nullptr;
}

/**
 * Load dataset and return identifier for legacy compatibility
 * This wraps load_dataset_data but returns just the path for backwards compatibility
 */
const char* load_dataset(const char* dataset_name) {
    std::cout << "[DatasetManager] Loading dataset (legacy): " << dataset_name << std::endl;

    static std::string dataset_path;
    dataset_path = dataset_name_to_path(dataset_name);

    // Verify the dataset exists
    if (!directory_exists(dataset_path)) {
        std::cout << "[DatasetManager] WARNING: Dataset not found at " << dataset_path << std::endl;
        std::cout << "[DatasetManager] Please download datasets using download_datasets.sh" << std::endl;
        // Return the expected path anyway - caller will handle the error
    }

    // Return the local path
    return dataset_path.c_str();
}

/**
 * List available offline datasets
 */
void* list_available_datasets() {
    std::cout << "[DatasetManager] Listing available datasets..." << std::endl;

    PyObject* result = PyList_New(0);

    // Iterate through offline datasets directory
    DIR* base_dir = opendir(OFFLINE_DATASETS_PATH.c_str());
    if (!base_dir) {
        std::cout << "[DatasetManager] No offline datasets directory found" << std::endl;
        return (void*)result;
    }

    struct dirent* owner_entry;
    while ((owner_entry = readdir(base_dir)) != nullptr) {
        std::string owner_name = owner_entry->d_name;
        if (owner_name == "." || owner_name == "..") continue;

        std::string owner_path = OFFLINE_DATASETS_PATH + owner_name;
        if (!directory_exists(owner_path)) continue;

        // Check for datasets under this owner
        DIR* owner_dir = opendir(owner_path.c_str());
        if (!owner_dir) continue;

        struct dirent* dataset_entry;
        while ((dataset_entry = readdir(owner_dir)) != nullptr) {
            std::string dataset_name = dataset_entry->d_name;
            if (dataset_name == "." || dataset_name == "..") continue;

            std::string dataset_path = owner_path + "/" + dataset_name;
            if (!directory_exists(dataset_path)) continue;

            // This looks like a valid dataset
            std::string full_name = owner_name + "/" + dataset_name;
            PyObject* py_name = PyUnicode_FromString(full_name.c_str());
            PyList_Append(result, py_name);
            Py_DECREF(py_name);

            std::cout << "[DatasetManager] Found: " << full_name << std::endl;
        }
        closedir(owner_dir);
    }
    closedir(base_dir);

    return (void*)result;
}

/**
 * Verify dataset integrity by checking hash
 */
bool verify_dataset_hash(const std::string& path, const std::string& expected_hash) {
    std::cout << "[DatasetManager] Verifying dataset hash for: " << path << std::endl;

    if (expected_hash.empty()) {
        std::cout << "[DatasetManager] No hash provided, skipping verification" << std::endl;
        return true;
    }

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    PyDict_SetItemString(globals, "file_path", PyUnicode_FromString(path.c_str()));
    PyDict_SetItemString(globals, "expected_hash", PyUnicode_FromString(expected_hash.c_str()));

    const char* verify_code = R"(
import hashlib

hash_valid = False
try:
    sha256 = hashlib.sha256()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(8192), b''):
            sha256.update(chunk)
    computed_hash = sha256.hexdigest()
    hash_valid = computed_hash == expected_hash
    if not hash_valid:
        print(f"[DatasetManager] Hash mismatch!")
        print(f"[DatasetManager]   Expected: {expected_hash}")
        print(f"[DatasetManager]   Computed: {computed_hash}")
    else:
        print(f"[DatasetManager] Hash verified: {computed_hash[:16]}...")
except Exception as e:
    print(f"[DatasetManager] Hash verification error: {e}")
    hash_valid = False
)";

    PyRun_String(verify_code, Py_file_input, globals, globals);

    PyObject* valid = PyDict_GetItemString(globals, "hash_valid");
    bool result = valid && PyObject_IsTrue(valid);

    Py_DECREF(globals);
    return result;
}

/**
 * Get the last error message
 */
const char* get_last_error() {
    return g_last_error.c_str();
}

} // namespace DatasetManager
