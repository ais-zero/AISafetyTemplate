#!/bin/bash
# Download Python packages for offline/air-gapped installation
# Run this script on a machine WITH internet access, then copy
# the offline_packages folder to the build environment.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="${SCRIPT_DIR}/offline_packages"

echo "=============================================="
echo "Downloading packages for offline installation"
echo "=============================================="

# Create packages directory
mkdir -p "${PACKAGES_DIR}"

# Download all packages from requirements.txt
echo "Downloading packages from requirements.txt..."
pip download \
    --dest "${PACKAGES_DIR}" \
    --platform manylinux2014_x86_64 \
    --python-version 311 \
    --only-binary=:all: \
    -r "${SCRIPT_DIR}/requirements.txt" || {
    echo "Note: Some packages may not have binary wheels, downloading source..."
    pip download \
        --dest "${PACKAGES_DIR}" \
        -r "${SCRIPT_DIR}/requirements.txt"
}

# Also download additional dependencies that might be needed
echo "Downloading additional HELM dependencies..."
pip download \
    --dest "${PACKAGES_DIR}" \
    --platform manylinux2014_x86_64 \
    --python-version 311 \
    --only-binary=:all: \
    openai \
    anthropic \
    nltk \
    spacy \
    torch \
    transformers \
    2>/dev/null || true

echo ""
echo "=============================================="
echo "Download complete!"
echo "Packages saved to: ${PACKAGES_DIR}"
echo ""
echo "Package count: $(ls -1 ${PACKAGES_DIR}/*.whl 2>/dev/null | wc -l) wheel files"
echo "Total size: $(du -sh ${PACKAGES_DIR} | cut -f1)"
echo "=============================================="
echo ""
echo "To use these packages in an air-gapped environment:"
echo "1. Copy the offline_packages folder to the target machine"
echo "2. Run: pip install --no-index --find-links=./offline_packages -r requirements.txt"
