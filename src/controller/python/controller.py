"""
Python wrapper for controller.so DLL

This is the ONLY module UserComponents should import.
Do NOT import any other modules directly!
"""
import ctypes
import os
import json
from typing import Any, Dict, List, Optional


class Controller:
    """
    High-level Python interface to C++ Controller DLL

    UserComponents should ONLY use this class.
    Do NOT import any other modules directly!
    """

    def __init__(self):
        # Load the compiled DLL
        lib_path = self._find_library()

        try:
            self._lib = ctypes.CDLL(lib_path)
            self._setup_signatures()
            self._initialized = False
        except OSError as e:
            raise RuntimeError(
                f"Failed to load controller.so from {lib_path}: {e}\n"
                "Make sure the Controller DLL is built and available."
            )

    def _find_library(self) -> str:
        """Find the controller.so library"""
        # Try multiple locations
        locations = [
            # Development build location
            os.path.join(os.path.dirname(__file__), '../../build/controller.so'),
            # Installed location (Docker)
            '/opt/controller/libcontroller.so',
            # Current directory
            './controller.so',
            # Build directory
            '../build/controller.so',
        ]

        for path in locations:
            abs_path = os.path.abspath(path)
            if os.path.exists(abs_path):
                print(f"[Controller] Found library at: {abs_path}")
                return abs_path

        raise FileNotFoundError(
            f"controller.so not found in any of these locations:\n" +
            "\n".join(f"  - {os.path.abspath(p)}" for p in locations) +
            "\n\nPlease build the Controller DLL first:\n"
            "  cd controller && mkdir -p build && cd build && cmake .. && make"
        )

    def _setup_signatures(self):
        """Setup C function signatures"""
        # controller_init
        self._lib.controller_init.argtypes = [ctypes.c_char_p]
        self._lib.controller_init.restype = ctypes.c_int

        # controller_shutdown
        self._lib.controller_shutdown.argtypes = []
        self._lib.controller_shutdown.restype = None

        # controller_get_proxy_client
        self._lib.controller_get_proxy_client.argtypes = []
        self._lib.controller_get_proxy_client.restype = ctypes.py_object

        # controller_load_dataset
        self._lib.controller_load_dataset.argtypes = [ctypes.c_char_p]
        self._lib.controller_load_dataset.restype = ctypes.c_char_p

        # controller_submit_results
        self._lib.controller_submit_results.argtypes = [ctypes.c_char_p]
        self._lib.controller_submit_results.restype = ctypes.c_int

        # controller_get_version
        self._lib.controller_get_version.argtypes = []
        self._lib.controller_get_version.restype = ctypes.c_char_p

        # controller_get_last_error
        self._lib.controller_get_last_error.argtypes = []
        self._lib.controller_get_last_error.restype = ctypes.c_char_p

    def init(self, config_path: str = '/app/config.yaml') -> None:
        """
        Initialize Controller with config file

        Args:
            config_path: Path to YAML config file
        """
        result = self._lib.controller_init(config_path.encode('utf-8'))
        if result != 0:
            error = self.get_last_error()
            raise RuntimeError(f"Controller initialization failed: {error}")

        self._initialized = True
        print(f"[Controller] Initialized with config: {config_path}")

    def get_model_client(self):
        """
        Get model client for making completions

        Returns:
            ProxyClient object for making model requests
        """
        if not self._initialized:
            raise RuntimeError("Controller not initialized. Call init() first.")

        client = self._lib.controller_get_proxy_client()
        if client is None:
            error = self.get_last_error()
            raise RuntimeError(f"Failed to create model client: {error}")

        return client

    def load_dataset(self, dataset_name: str) -> str:
        """
        Load a dataset

        Args:
            dataset_name: Name of dataset (e.g., "JailbreakBench/JBB-Behaviors")

        Returns:
            Dataset name/identifier that can be used with datasets library
        """
        if not self._initialized:
            raise RuntimeError("Controller not initialized. Call init() first.")

        result = self._lib.controller_load_dataset(dataset_name.encode('utf-8'))
        if result is None:
            error = self.get_last_error()
            raise RuntimeError(f"Failed to load dataset {dataset_name}: {error}")

        return result.decode('utf-8')

    def submit_results(self, results: Dict[str, Any]) -> None:
        """
        Submit final evaluation results

        Args:
            results: Dictionary containing evaluation results
        """
        if not self._initialized:
            raise RuntimeError("Controller not initialized. Call init() first.")

        # Convert results to JSON
        json_str = json.dumps(results, indent=2)

        result = self._lib.controller_submit_results(json_str.encode('utf-8'))
        if result != 0:
            error = self.get_last_error()
            raise RuntimeError(f"Failed to submit results: {error}")

        print("[Controller] Results submitted successfully")

    def shutdown(self) -> None:
        """Cleanup Controller resources"""
        if self._initialized:
            self._lib.controller_shutdown()
            self._initialized = False
            print("[Controller] Shutdown complete")

    def get_version(self) -> str:
        """Get Controller version"""
        version = self._lib.controller_get_version()
        return version.decode('utf-8')

    def get_last_error(self) -> str:
        """Get last error message"""
        error = self._lib.controller_get_last_error()
        return error.decode('utf-8') if error else "Unknown error"

    def __enter__(self):
        """Context manager entry"""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.shutdown()
        return False
