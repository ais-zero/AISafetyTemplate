# Eval Container Security Configuration

This document describes the security hardening measures applied to the eval container.

## Network Isolation

The eval container operates in a **fully isolated network** with the following restrictions:

1. **No Internet Access**: The container is connected only to `sandbox_internal` network which is marked as `internal: true`
2. **Limited Connectivity**: Can only communicate with `llm_proxy` container on port 8000
3. **No DNS**: External DNS is disabled to prevent DNS-based exfiltration
4. **Egress Filtering**: All outbound traffic except to llm_proxy is blocked

## Container Hardening

### User Privileges
- Runs as non-root user `eval` (UID/GID configured at build time)
- `no-new-privileges` security option prevents privilege escalation
- All capabilities dropped except `NET_BIND_SERVICE`

### Filesystem Security
- Build tools removed after compilation
- Minimal filesystem permissions
- `/tmp` mounted as tmpfs with size limits
- User component mounted read-only

### Resource Limits
- CPU: 4 cores max, 1 core reserved
- Memory: 8GB max, 2GB reserved
- Prevents resource exhaustion attacks

## Dependency Management

### Offline Installation
For air-gapped environments, dependencies must be pre-downloaded:

```bash
# On a machine with internet access:
cd eval_container
./download_offline_packages.sh

# Copy offline_packages/ to target build environment
```

### Allowed Python Modules
The Controller enforces an import allowlist. See `security.cpp` for the full list.

**Blocked modules** (security-sensitive):
- `subprocess` - Command execution
- `socket` - Raw network access
- `ftplib`, `telnetlib` - Legacy protocols
- `paramiko`, `fabric` - SSH access

## Data Exfiltration Prevention

1. **No network egress** except to llm_proxy
2. **Read-only mounts** for user code
3. **Output validation** by Controller before writing results
4. **Rate limiting** on LLM requests to detect unusual patterns

## Secrets Management

- API keys passed via environment variables
- `.env` file not mounted in eval container
- Secrets only available in llm_proxy container

## Monitoring Recommendations

For production deployments:

1. Enable Docker audit logging
2. Monitor container network traffic
3. Set up alerts for unusual API patterns
4. Review output files for anomalies

## Building Securely

```bash
# Build with security scanning
docker build --no-cache -t eval_sandbox eval_container/

# Scan for vulnerabilities
docker scan eval_sandbox

# Run with security profile
docker run --security-opt=no-new-privileges:true \
           --cap-drop=ALL \
           --read-only \
           eval_sandbox
```
