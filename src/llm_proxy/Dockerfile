# LLM Proxy Container - Hardened
# This container ONLY has network access to HuggingFace API

FROM python:3.11-slim

# Security: Run as non-root user
RUN groupadd -r proxy && useradd -r -g proxy proxy

WORKDIR /app

# Install dependencies
COPY requirements.txt /app/
RUN pip install --no-cache-dir -r requirements.txt && \
    rm -rf /root/.cache/pip

# Copy server code
COPY server.py /app/

# Security: Remove unnecessary packages
RUN apt-get purge -y --auto-remove && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Security: Make filesystem read-only where possible
RUN chmod 755 /app && \
    chmod 444 /app/server.py /app/requirements.txt

# Security: Create minimal tmp directory
RUN mkdir -p /tmp && chmod 1777 /tmp

# Switch to non-root user
USER proxy

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=10s --timeout=5s --retries=3 \
    CMD python -c "import urllib.request; urllib.request.urlopen('http://localhost:8000/health')" || exit 1

# Run server with waitress (production WSGI)
CMD ["python", "server.py"]
