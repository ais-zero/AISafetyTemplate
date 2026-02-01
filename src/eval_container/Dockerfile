# Eval Container - Hardened & Isolated
# This container has NO internet access except to llm_proxy
# All dependencies must be pre-installed via offline packages
#
# Build context: parent directory (src/)
# Dockerfile location: eval_container/Dockerfile

ARG PYTHON_IMAGE=python:3.11.8-slim
FROM ${PYTHON_IMAGE}

# Build arguments
ARG CONTROLLER_BUILD=true

# Security: Create non-root user
RUN groupadd -r eval && useradd -r -g eval -d /app -s /sbin/nologin eval

WORKDIR /app

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONPYCACHEPREFIX=/tmp/pycache

# Install system dependencies for building Controller
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy offline dependencies first (if available)
# These are pre-downloaded wheel files for air-gapped installation
COPY eval_container/offline_packages/ /app/offline_packages/

# Install Python dependencies from offline packages if available,
# otherwise install from PyPI (during build only)
COPY eval_container/requirements.lock /app/
COPY eval_container/requirements.txt /app/
RUN if [ -d "/app/offline_packages" ] && [ "$(ls -A /app/offline_packages/*.whl 2>/dev/null)" ]; then \
        echo "Installing from offline packages..."; \
        pip install --no-cache-dir --no-index --find-links=/app/offline_packages -r requirements.lock; \
    else \
        echo "Installing from PyPI (build-time only)..."; \
        pip install --no-cache-dir -r requirements.lock; \
    fi

# Copy and build Controller
COPY controller/ /app/controller/
RUN cd /app/controller && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    cp controller.so /app/libcontroller.so

# Copy Controller Python module
RUN cp /app/controller/python/controller.py /app/

# Copy HELM configuration
COPY helm_config/ /app/helm_config/

# Copy offline datasets (pre-downloaded for air-gapped installation)
# Run download_datasets.sh before building to populate this directory
COPY eval_container/offline_datasets/ /app/offline_datasets/

# Copy user evaluation component (will be mounted at runtime, but include default)
COPY user_component/boilerplate/ /app/eval/

# Set Python path
ENV PYTHONPATH=/app:/app/eval

# Security: Remove build tools after compilation
RUN apt-get purge -y --auto-remove build-essential cmake && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    /app/controller/src /app/controller/include /app/controller/build \
    /root/.cache /app/offline_packages

# Security: Set proper permissions
RUN chown -R eval:eval /app && \
    chmod 755 /app && \
    chmod 555 /app/libcontroller.so /app/controller.py

# Create output directory with write permissions
RUN mkdir -p /app/output /app/benchmark_output && \
    chown eval:eval /app/output /app/benchmark_output && \
    chmod 755 /app/output /app/benchmark_output

# Security: Create minimal tmp
RUN mkdir -p /tmp && chmod 1777 /tmp

# Switch to non-root user
USER eval

# Environment variables
ENV LLM_PROXY_URL=http://llm_proxy:8000
ENV OUTPUT_PATH=/app/output/evaluation_results.json

# Health check - verify controller loads
HEALTHCHECK --interval=30s --timeout=10s --retries=3 \
    CMD python -c "from controller import Controller; c = Controller(); print('ok')" || exit 1

# Default command - run user evaluation
CMD ["python", "/app/eval/my_eval.py"]
