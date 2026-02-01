#!/bin/bash
#
# Download datasets for offline use in the eval container
#
# The eval container has NO INTERNET ACCESS, so all datasets must be
# pre-downloaded and included in the container image.
#
# Usage:
#   ./download_datasets.sh [dataset_name]
#
# Examples:
#   ./download_datasets.sh                              # Download all default datasets
#   ./download_datasets.sh JailbreakBench/JBB-Behaviors # Download specific dataset
#
# This script requires:
#   - Python 3.8+
#   - pip install datasets huggingface_hub

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OFFLINE_DIR="${SCRIPT_DIR}/offline_datasets"

# Default datasets to download
DEFAULT_DATASETS=(
    "JailbreakBench/JBB-Behaviors"
)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required Python packages are installed
check_requirements() {
    log_info "Checking requirements..."

    if ! python3 -c "import datasets" 2>/dev/null; then
        log_error "Python 'datasets' package not installed."
        log_info "Install with: pip install datasets huggingface_hub"
        exit 1
    fi

    log_info "Requirements satisfied"
}

# Download a single dataset
download_dataset() {
    local dataset_name="$1"

    # Parse owner/name format
    local owner=""
    local name=""
    if [[ "$dataset_name" == *"/"* ]]; then
        owner="${dataset_name%/*}"
        name="${dataset_name#*/}"
    else
        owner="local"
        name="$dataset_name"
    fi

    local output_dir="${OFFLINE_DIR}/${owner}/${name}"

    log_info "Downloading: ${dataset_name}"
    log_info "Output: ${output_dir}"

    # Create output directory
    mkdir -p "${output_dir}"

    # Use Python to download and save the dataset
    python3 << PYTHON_SCRIPT
import sys
import json
import os

try:
    from datasets import load_dataset
    from huggingface_hub import hf_hub_download
except ImportError as e:
    print(f"Error: {e}")
    print("Install with: pip install datasets huggingface_hub")
    sys.exit(1)

dataset_name = "${dataset_name}"
output_dir = "${output_dir}"

print(f"Loading dataset: {dataset_name}")

try:
    # JBB-Behaviors requires specifying the 'behaviors' config
    if "JBB-Behaviors" in dataset_name:
        dataset = load_dataset(dataset_name, "behaviors")
    else:
        dataset = load_dataset(dataset_name)

    # Get all splits
    splits = list(dataset.keys())
    print(f"Found splits: {splits}")

    # Save each split
    for split in splits:
        split_data = dataset[split]
        output_file = os.path.join(output_dir, f"{split}.jsonl")

        print(f"Saving {split} split ({len(split_data)} records) to {output_file}")

        with open(output_file, 'w', encoding='utf-8') as f:
            for item in split_data:
                # Convert to dict and handle special types
                item_dict = dict(item)
                for key, value in item_dict.items():
                    if hasattr(value, 'tolist'):  # numpy arrays
                        item_dict[key] = value.tolist()
                f.write(json.dumps(item_dict, ensure_ascii=False) + '\n')

        print(f"Saved: {output_file}")

    # Also save a combined file if multiple splits
    if len(splits) == 1:
        # Rename single split to data.jsonl for consistency
        single_file = os.path.join(output_dir, f"{splits[0]}.jsonl")
        data_file = os.path.join(output_dir, "data.jsonl")
        if os.path.exists(single_file) and single_file != data_file:
            import shutil
            shutil.copy(single_file, data_file)
            print(f"Copied to: {data_file}")

    # Save metadata
    metadata = {
        "dataset_name": dataset_name,
        "splits": splits,
        "total_records": sum(len(dataset[s]) for s in splits),
        "features": {s: list(dataset[s].features.keys()) for s in splits}
    }

    meta_file = os.path.join(output_dir, "metadata.json")
    with open(meta_file, 'w') as f:
        json.dump(metadata, f, indent=2)
    print(f"Saved metadata: {meta_file}")

    print(f"Successfully downloaded: {dataset_name}")

except Exception as e:
    print(f"Error downloading {dataset_name}: {e}")

    # Try alternative download for HarmBench specifically
    if "JBB-Behaviors" in dataset_name or "harmbench" in dataset_name.lower():
        print("Attempting alternative download for JailbreakBench...")
        try:
            # Try direct file download
            behaviors_file = hf_hub_download(
                repo_id="JailbreakBench/JBB-Behaviors",
                filename="behaviors.csv",
                repo_type="dataset"
            )

            import csv
            records = []
            with open(behaviors_file, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    records.append(row)

            output_file = os.path.join(output_dir, "behaviors.jsonl")
            with open(output_file, 'w', encoding='utf-8') as f:
                for record in records:
                    f.write(json.dumps(record, ensure_ascii=False) + '\n')

            print(f"Downloaded {len(records)} behaviors to {output_file}")

        except Exception as e2:
            print(f"Alternative download also failed: {e2}")
            sys.exit(1)
    else:
        sys.exit(1)
PYTHON_SCRIPT

    if [ $? -eq 0 ]; then
        log_info "Successfully downloaded: ${dataset_name}"
    else
        log_error "Failed to download: ${dataset_name}"
        return 1
    fi
}

# Main function
main() {
    log_info "=== Dataset Downloader for Offline Use ==="
    log_info "Output directory: ${OFFLINE_DIR}"

    # Create offline directory
    mkdir -p "${OFFLINE_DIR}"

    # Check requirements
    check_requirements

    # Determine which datasets to download
    local datasets=()
    if [ $# -gt 0 ]; then
        datasets=("$@")
    else
        datasets=("${DEFAULT_DATASETS[@]}")
    fi

    log_info "Datasets to download: ${datasets[*]}"
    echo ""

    # Download each dataset
    local failed=0
    for dataset in "${datasets[@]}"; do
        if ! download_dataset "$dataset"; then
            failed=$((failed + 1))
        fi
        echo ""
    done

    # Summary
    echo ""
    log_info "=== Download Complete ==="
    log_info "Downloaded to: ${OFFLINE_DIR}"

    if [ $failed -gt 0 ]; then
        log_warn "Failed downloads: ${failed}"
        exit 1
    fi

    # List downloaded datasets
    log_info "Available datasets:"
    find "${OFFLINE_DIR}" -name "*.jsonl" -o -name "*.json" | while read -r f; do
        local size=$(du -h "$f" | cut -f1)
        echo "  - $f ($size)"
    done

    echo ""
    log_info "These datasets will be available in the eval container at:"
    log_info "  /app/offline_datasets/"
}

# Run main with all arguments
main "$@"
