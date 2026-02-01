#!/bin/bash
# Build script for Controller DLL

set -e

echo "======================================"
echo "Building Controller DLL"
echo "======================================"

cd controller

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "[1/3] Running CMake..."
cmake ..

# Build
echo "[2/3] Building with make..."
make

# Verify
echo "[3/3] Verifying build..."
if [ -f "controller.so" ]; then
    echo "✅ Build successful!"
    ls -lh controller.so

    # Test Python import
    echo ""
    echo "Testing Python import..."
    cd ..
    export PYTHONPATH="$(pwd)/python:$PYTHONPATH"
    python3 -c "
from controller import Controller
c = Controller()
print('✅ Controller imported successfully!')
print(f'   Version: {c.get_version()}')
"
else
    echo "❌ Build failed - controller.so not found"
    exit 1
fi

echo ""
echo "======================================"
echo "Controller DLL ready!"
echo "======================================"
