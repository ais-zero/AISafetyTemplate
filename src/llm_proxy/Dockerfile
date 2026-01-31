FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY llm_proxy/requirements.txt /app/
RUN pip install --no-cache-dir -r requirements.txt

# Copy server code
COPY llm_proxy/server.py /app/

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=10s --timeout=5s --retries=3 \
    CMD python -c "import requests; requests.get('http://localhost:8000/health')" || exit 1

# Run server
CMD ["python", "server.py"]
