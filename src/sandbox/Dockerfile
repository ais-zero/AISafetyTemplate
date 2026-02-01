FROM python:3.10-slim

WORKDIR /app

RUN pip install --no-cache-dir crfm-helm openai

# Copy HELM config files
COPY helm_config/ /app/prod_env/

# Copy user eval component from additional build context
COPY --from=user_eval . /app/eval/

# Install user eval dependencies if requirements.txt is non-empty
RUN if [ -s /app/eval/requirements.txt ]; then \
      pip install --no-cache-dir -r /app/eval/requirements.txt; \
    fi

# HELM discovers @run_spec_function decorators via PYTHONPATH
ENV PYTHONPATH=/app/eval

CMD ["helm-run", \
     "--plugins", "/app/eval/helm_example_run_specs.py", \
     "--conf-paths", "/app/prod_env/run_entries.conf", \
     "--models-to-run", "openai/smollm3-3b", \
     "--suite", "v1", \
     "--max-eval-instances", "10", \
     "--output-path", "/app/benchmark_output"]
