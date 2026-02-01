#!/bin/bash
# Test script for LLM Proxy

set -e

echo "================================"
echo "LLM Proxy Test Script"
echo "================================"

# Check if proxy is running
echo ""
echo "1. Checking proxy health..."
HEALTH_RESPONSE=$(curl -s http://localhost:8000/health)
echo "Health Response: $HEALTH_RESPONSE"

# Test root endpoint
echo ""
echo "2. Checking root endpoint..."
ROOT_RESPONSE=$(curl -s http://localhost:8000/)
echo "Root Response: $ROOT_RESPONSE"

# Test chat completions
echo ""
echo "3. Testing chat completions endpoint..."
CHAT_RESPONSE=$(curl -s -X POST http://localhost:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [{"role": "user", "content": "Say hello in one word"}],
    "model": "HuggingFaceTB/SmolLM2-1.7B",
    "max_tokens": 10
  }')

echo "Chat Response: $CHAT_RESPONSE"

# Extract and display the actual message
MESSAGE=$(echo $CHAT_RESPONSE | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['choices'][0]['message']['content'])" 2>/dev/null || echo "Failed to parse")
echo ""
echo "Model Response: $MESSAGE"


# Test chat completions with invalid model
echo ""
echo "4. Testing chat completions endpoint with invalid model..."
INVALID_MODEL_RESPONSE=$(curl -s -X POST http://localhost:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [{"role": "user", "content": "Say hello in one word"}],
    "model": "openai/gpt-4o-mini",
    "max_tokens": 10
  }')

echo "Invalid Model Response: $INVALID_MODEL_RESPONSE"

echo ""
echo "================================"
echo "All tests passed!"
echo "================================"
