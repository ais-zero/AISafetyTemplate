#!/bin/bash
# Quick start script for AI Safety Template

set -e

echo "================================"
echo "AI Safety Template - HarmBench"
echo "================================"

# Check for .env file
if [ ! -f .env ]; then
    echo ""
    echo "ERROR: .env file not found!"
    echo "Please create .env from .env.example and add your HuggingFace API key."
    echo ""
    echo "  cp .env.example .env"
    echo "  # Edit .env and add your HF_API_KEY"
    echo ""
    exit 1
fi

# Check for HF_API_KEY
source .env
if [ -z "$HF_API_KEY" ] && [ -z "$LLM_API_KEY" ]; then
    echo ""
    echo "ERROR: HF_API_KEY not set in .env file!"
    echo "Please edit .env and add your HuggingFace API key."
    echo ""
    exit 1
fi

echo ""
echo "Building and starting containers..."
echo ""

# Build and run with docker-compose
docker-compose up --build

echo ""
echo "================================"
echo "Evaluation Complete!"
echo "================================"
echo ""
echo "Results saved to Docker volume: eval_output"
echo ""
echo "To view results:"
echo "  docker run --rm -v ai-safety-template_eval_output:/data alpine cat /data/evaluation_results.json"
echo ""
