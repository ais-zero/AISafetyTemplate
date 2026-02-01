# Troubleshooting Log

## Issue 1: C++ Compilation Error in Controller
**File:** `src/controller/src/controller.cpp`
**Error:** `missing terminating " character`
**Cause:** A raw string literal `R"(...)"` contains `)"` inside the Python code, causing premature termination of the string.
**Fix:** Change the raw string delimiter to `R"HOOK(...)HOOK"`.

## Issue 2: OpenAI Library Incompatibility
**File:** `src/llm_proxy/requirements.txt`
**Error:** `TypeError: Client.__init__() got an unexpected keyword argument 'proxies'` (Anticipated Runtime Error)
**Cause:** `openai==1.12.0` has compatibility issues with newer `httpx` versions.
**Fix:** Upgrade `openai` to `>=1.40.0`.

## Issue 3: Import Ambiguity
**File:** `src/controller/python/controller.py` and `src/Dockerfile`
**Error:** `ImportError: dynamic module does not define module export function (PyInit_controller)`
**Cause:** `controller.py` and `controller.so` are in the same directory (`/opt/controller/`). Python tries to load `controller.so` as a module instead of `controller.py` when importing `controller`.
**Fix:** Rename `controller.so` to `libcontroller.so` in `src/Dockerfile` and update the search path in `src/controller/python/controller.py`.

## Issue 4: Segmentation Fault (SIGSEGV)
**File:** `src/controller/src/controller.cpp`
**Error:** `user-component exited with code 139`
**Cause:** Python C API functions are called from C++ without holding the Global Interpreter Lock (GIL).
**Fix:** Implement a `GILGuard` class and use it in all exported C functions that interact with Python.
