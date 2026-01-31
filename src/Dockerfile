FROM python:3.11-slim

# Install build dependencies for Controller C++ compilation
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy Controller source code
COPY controller/ /build/controller/

# Build Controller DLL
RUN cd /build/controller && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    make && \
    echo "Controller built successfully" && \
    ls -lh controller.so

# Install Controller DLL and Python wrapper
RUN mkdir -p /opt/controller && \
    cp /build/controller/build/controller.so /opt/controller/ && \
    cp /build/controller/python/controller.py /opt/controller/ && \
    echo "Controller installed to /opt/controller"

# Set up application directory
WORKDIR /app

# Install Python dependencies
COPY requirements.txt /app/
RUN pip install --no-cache-dir -r requirements.txt

# Copy configuration
COPY config.yaml /app/

# Copy UserComponent
COPY user_component/ /app/user_component/

# Create results directory
RUN mkdir -p /tmp/results /tmp/helm_cache

# Add Controller to Python path
ENV PYTHONPATH=/opt/controller:$PYTHONPATH
ENV PYTHONUNBUFFERED=1
ENV OUTPUT_PATH=/tmp/results/evaluation_output.json

# Verify installation
RUN python3 -c "import sys; print('Python path:', sys.path)" && \
    ls -lh /opt/controller/ && \
    python3 -c "from controller import Controller; print('Controller imported successfully')" || \
    echo "WARNING: Controller import failed - will try at runtime"

# Run UserComponent (HarmBench evaluation)
CMD ["python3", "/app/user_component/examples/harmbench_eval/harmbench_eval.py"]
