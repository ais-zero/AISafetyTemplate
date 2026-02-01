"""
LLM Proxy Server - OpenAI-compatible API for HuggingFace Inference

This server acts as a secure proxy between the isolated eval container and
HuggingFace Inference API. It exposes an OpenAI-compatible interface.

Security:
- Only accepts requests from internal Docker network
- Only makes outbound requests to HuggingFace API
- Rate limiting and request validation
- No filesystem access beyond /tmp
"""

from flask import Flask, request, jsonify, abort
import requests as http_requests
import os
import logging
import time
import hashlib
from functools import wraps
from typing import Dict, Any, Optional

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Configuration
HUGGINGFACE_API_BASE = "https://router.huggingface.co/hf-inference/v1"
HUGGINGFACE_MODEL = os.environ.get('HF_MODEL', 'HuggingFaceTB/SmolLM3-3B')
HF_API_KEY = os.environ.get('HF_API_KEY', os.environ.get('LLM_API_KEY', ''))

# Rate limiting
REQUEST_LIMIT_PER_MINUTE = int(os.environ.get('RATE_LIMIT', '60'))
request_counts: Dict[str, list] = {}

# Allowed models (whitelist)
ALLOWED_MODELS = {
    'smollm3-3b',
    'openai/smollm3-3b',
    'HuggingFaceTB/SmolLM3-3B',
    'gpt-4o-mini',  # Map to smollm3-3b
}


def rate_limit(f):
    """Rate limiting decorator"""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        client_ip = request.remote_addr
        current_time = time.time()

        # Clean old entries
        if client_ip in request_counts:
            request_counts[client_ip] = [
                t for t in request_counts[client_ip]
                if current_time - t < 60
            ]
        else:
            request_counts[client_ip] = []

        # Check rate limit
        if len(request_counts[client_ip]) >= REQUEST_LIMIT_PER_MINUTE:
            logger.warning(f"Rate limit exceeded for {client_ip}")
            return jsonify({"error": "Rate limit exceeded"}), 429

        request_counts[client_ip].append(current_time)
        return f(*args, **kwargs)
    return decorated_function


def validate_request(data: Dict[str, Any]) -> Optional[str]:
    """Validate incoming request data"""
    if not data:
        return "Empty request body"

    if 'messages' not in data:
        return "Missing 'messages' field"

    if not isinstance(data['messages'], list):
        return "'messages' must be a list"

    for msg in data['messages']:
        if not isinstance(msg, dict):
            return "Each message must be a dict"
        if 'role' not in msg or 'content' not in msg:
            return "Each message must have 'role' and 'content'"
        if msg['role'] not in ['system', 'user', 'assistant']:
            return f"Invalid role: {msg['role']}"

    # Validate model
    model = data.get('model', 'smollm3-3b')
    if model not in ALLOWED_MODELS:
        return f"Model '{model}' not allowed. Use: {ALLOWED_MODELS}"

    # Validate token limits
    max_tokens = data.get('max_tokens', 150)
    if not isinstance(max_tokens, int) or max_tokens < 1 or max_tokens > 4096:
        return "max_tokens must be between 1 and 4096"

    return None


@app.route('/v1/chat/completions', methods=['POST'])
@rate_limit
def chat_completions():
    """
    OpenAI-compatible chat completions endpoint.
    Forwards requests to HuggingFace Inference API.
    """
    if not HF_API_KEY:
        logger.error("HF_API_KEY not configured")
        return jsonify({"error": "API key not configured"}), 500

    try:
        data = request.json

        # Validate request
        error = validate_request(data)
        if error:
            logger.warning(f"Invalid request: {error}")
            return jsonify({"error": error}), 400

        logger.info(f"Received chat completion request, messages: {len(data['messages'])}")

        # Prepare request for HuggingFace
        hf_request = {
            "model": HUGGINGFACE_MODEL,
            "messages": data['messages'],
            "temperature": min(max(data.get('temperature', 0.7), 0.0), 2.0),
            "max_tokens": min(data.get('max_tokens', 150), 4096),
            "stream": False
        }

        # Make request to HuggingFace
        headers = {
            "Authorization": f"Bearer {HF_API_KEY}",
            "Content-Type": "application/json"
        }

        response = http_requests.post(
            f"{HUGGINGFACE_API_BASE}/chat/completions",
            json=hf_request,
            headers=headers,
            timeout=60
        )

        if response.status_code != 200:
            logger.error(f"HuggingFace API error: {response.status_code} - {response.text}")
            return jsonify({
                "error": f"Upstream API error: {response.status_code}"
            }), 502

        result = response.json()
        logger.info(f"Successfully proxied request")

        # Return in OpenAI-compatible format
        return jsonify(result)

    except http_requests.exceptions.Timeout:
        logger.error("Request to HuggingFace timed out")
        return jsonify({"error": "Request timeout"}), 504
    except http_requests.exceptions.RequestException as e:
        logger.error(f"Network error: {e}")
        return jsonify({"error": "Network error"}), 502
    except Exception as e:
        logger.error(f"Error proxying request: {e}")
        return jsonify({"error": str(e)}), 500


@app.route('/v1/models', methods=['GET'])
def list_models():
    """List available models"""
    return jsonify({
        "object": "list",
        "data": [
            {
                "id": "openai/smollm3-3b",
                "object": "model",
                "created": 1700000000,
                "owned_by": "huggingface"
            }
        ]
    })


@app.route('/health', methods=['GET'])
def health():
    """Health check endpoint for Docker healthcheck"""
    status = "ok" if HF_API_KEY else "no_api_key"
    return jsonify({
        "status": status,
        "api_key_configured": bool(HF_API_KEY),
        "model": HUGGINGFACE_MODEL
    })


@app.route('/', methods=['GET'])
def root():
    """Root endpoint with basic info"""
    return jsonify({
        "service": "LLM Proxy",
        "version": "0.2.0",
        "backend": "HuggingFace Inference API",
        "model": HUGGINGFACE_MODEL,
        "endpoints": {
            "chat_completions": "/v1/chat/completions",
            "models": "/v1/models",
            "health": "/health"
        }
    })


# Security: Reject requests to unexpected endpoints
@app.errorhandler(404)
def not_found(e):
    logger.warning(f"404 request to: {request.path}")
    return jsonify({"error": "Endpoint not found"}), 404


if __name__ == '__main__':
    if not HF_API_KEY:
        logger.warning("HF_API_KEY not set - proxy will fail on actual requests")
    else:
        logger.info(f"API key configured (length: {len(HF_API_KEY)})")

    logger.info(f"Using model: {HUGGINGFACE_MODEL}")
    logger.info("Starting LLM Proxy Server on port 8000")

    # Use waitress for production-grade serving
    try:
        from waitress import serve
        serve(app, host='0.0.0.0', port=8000)
    except ImportError:
        # Fallback to Flask dev server
        app.run(host='0.0.0.0', port=8000, debug=False)
