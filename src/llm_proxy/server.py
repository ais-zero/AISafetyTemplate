"""
LLM Proxy Server - OpenAI-compatible API

This server acts as a proxy between the Controller container and the actual
LLM API (OpenAI). It exposes an OpenAI-compatible interface on port 8000.
"""

from flask import Flask, request, jsonify
import openai
import os
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Initialize OpenAI client
api_key = os.environ.get('OPENAI_API_KEY')
if not api_key:
    logger.warning("OPENAI_API_KEY not set - proxy will fail on actual requests")

client = openai.OpenAI(api_key=api_key) if api_key else None


@app.route('/v1/chat/completions', methods=['POST'])
def chat_completions():
    """
    OpenAI-compatible chat completions endpoint.
    Forwards requests to the actual OpenAI API.
    """
    if not client:
        return jsonify({
            "error": "OPENAI_API_KEY not configured"
        }), 500

    try:
        data = request.json
        logger.info(f"Received chat completion request for model: {data.get('model', 'default')}")

        # Forward to actual OpenAI API
        response = client.chat.completions.create(
            model=data.get('model', 'gpt-4o-mini'),
            messages=data['messages'],
            temperature=data.get('temperature', 1.0),
            max_tokens=data.get('max_tokens', 100)
        )

        # Return in OpenAI format
        logger.info(f"Successfully proxied request, response length: {len(response.choices[0].message.content)}")
        return jsonify(response.model_dump())

    except KeyError as e:
        logger.error(f"Missing required field: {e}")
        return jsonify({
            "error": f"Missing required field: {e}"
        }), 400
    except Exception as e:
        logger.error(f"Error proxying request: {e}")
        return jsonify({
            "error": str(e)
        }), 500


@app.route('/health', methods=['GET'])
def health():
    """Health check endpoint for Docker healthcheck"""
    status = "ok" if client else "no_api_key"
    return jsonify({
        "status": status,
        "api_key_configured": client is not None
    })


@app.route('/', methods=['GET'])
def root():
    """Root endpoint with basic info"""
    return jsonify({
        "service": "LLM Proxy",
        "version": "0.1.0",
        "endpoints": {
            "chat_completions": "/v1/chat/completions",
            "health": "/health"
        }
    })


if __name__ == '__main__':
    logger.info("Starting LLM Proxy Server on port 8000")
    app.run(host='0.0.0.0', port=8000, debug=False)
