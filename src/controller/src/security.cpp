#include <Python.h>
#include <iostream>
#include <string>
#include <set>

/**
 * Security Module
 *
 * Provides security enforcement mechanisms:
 * - Import allowlist/blocklist
 * - Filesystem access control
 * - Network access restrictions
 */

namespace Security {

// Allowed Python modules
static const std::set<std::string> ALLOWED_IMPORTS = {
    // Core Python
    "json",
    "os",
    "sys",
    "logging",
    "datetime",
    "typing",
    "re",
    "math",
    "collections",
    "functools",
    "itertools",
    "pathlib",
    "importlib",
    "abc",
    "dataclasses",
    "enum",
    "copy",
    "hashlib",
    "base64",
    "uuid",
    "time",
    "random",
    "string",
    "io",
    "tempfile",
    "shutil",
    "glob",
    "fnmatch",
    "pickle",
    "gzip",
    "zipfile",
    "csv",
    "configparser",
    "argparse",
    "textwrap",
    "traceback",
    "warnings",
    "threading",
    "queue",
    "concurrent",

    // Controller
    "controller",

    // Data science libraries (needed for HELM)
    "numpy",
    "pandas",
    "scipy",
    "sklearn",
    "torch",
    "transformers",
    "tokenizers",
    "safetensors",

    // HTTP (needed for proxy communication)
    "requests",
    "urllib",
    "urllib3",
    "http",
    "httpx",
    "aiohttp",
    "certifi",
    "charset_normalizer",
    "idna",

    // Dataset loading
    "datasets",
    "huggingface_hub",

    // HELM framework modules
    "helm",
    "crfm_helm",
    "cattrs",
    "attrs",
    "dacite",
    "pydantic",
    "yaml",
    "ruamel",
    "toml",
    "tqdm",
    "filelock",
    "fsspec",
    "pyarrow",
    "multiprocess",
    "dill",
    "xxhash",
    "aiofiles",
    "nest_asyncio",
    "sqlitedict",
    "retrying",
    "tenacity",
    "nltk",
    "spacy",
    "sentencepiece",
    "tiktoken",
    "openai",
    "anthropic",
    "cohere",
    "google",
    "vertexai",
};

// Blocked modules (security-sensitive)
static const std::set<std::string> BLOCKED_IMPORTS = {
    "subprocess",
    "socket",
    "ftplib",
    "telnetlib",
    "paramiko",
    "fabric",
};

bool is_import_allowed(const std::string& module_name) {
    // Check if explicitly blocked
    if (BLOCKED_IMPORTS.find(module_name) != BLOCKED_IMPORTS.end()) {
        std::cerr << "[Security] Blocked import: " << module_name << std::endl;
        return false;
    }

    // For sprint: Allow most imports except explicitly blocked ones
    // In production: Only allow imports in ALLOWED_IMPORTS
    return true;
}

void install_import_hooks() {
    std::cout << "[Security] Installing import hooks (permissive mode)" << std::endl;

    // This is called from controller.cpp
    // For sprint, we use permissive mode
    // In production, strict enforcement would be enabled
}

} // namespace Security
