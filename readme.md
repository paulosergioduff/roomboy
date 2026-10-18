# Roomboy — Docker Development Environment

Roomboy provides a base development environment built on Ubuntu 24.04, with Node.js 24, Python 3, C/C++ build tools, Git, and AI-powered command-line tools.

## Prerequisites

- Docker Engine installed and running.
- A terminal.
- An internet connection for downloading packages and authenticating Codex.

## 1. Build the Docker Image

Run the following command from the directory containing the `Dockerfile`:

```bash
docker build -t roomboy:latest .
```

Verify that the image was created:

```bash
docker images roomboy
```

## 2. Run the Container

Start an interactive Bash session:

```bash
docker run --rm -it roomboy:latest
```

Verify the installed tools:

```bash
node --version
npm --version
python3 --version
git --version
codex --version
```

Type `exit` to leave the container.

**Note:** The `--rm` option automatically removes the container when it exits. Files stored only inside the container will not persist after removal.

## 3. Configure the Authentication Port

Codex CLI uses a local OAuth callback on port `1455` during browser-based authentication.

When running Codex inside a Docker container, the callback port may need to be forwarded from the host to the container.

Start the container with port `1455` published on the host's loopback interface:

```bash
docker run --rm -it \
  -p 127.0.0.1:1455:1455 \
  roomboy:latest
```

This maps:

| Host | Container |
|---|---|
| `127.0.0.1:1455` | Port `1455` |

Binding to `127.0.0.1` limits access to the host's loopback interface instead of exposing the port on all network interfaces.

### Authenticate Codex

Inside the container, run:

```bash
codex login
```

Follow the instructions displayed in the terminal and complete the login in your browser.

If the browser redirects to `http://localhost:1455`, the callback must be able to reach the Codex process running inside the container.

**Important:** Publishing the port does not guarantee that browser authentication will work in every Docker configuration. If the callback fails, the Codex process may be listening only on the container's loopback interface, or the browser may be running in a different environment.

## 4. Alternative: Device Authentication

If supported by your Codex CLI version and account, device authentication can avoid the need for a local browser callback port.

Run:

```bash
codex login --device-auth
```

Follow the displayed instructions and complete authentication in your browser.

If this option is unavailable, consult the official Codex CLI documentation for supported authentication methods.

## 5. Persist Codex Configuration

A container started with `--rm` is disposable. To reuse authentication between container sessions, persist the Codex configuration outside the container.

One option is to authenticate Codex on the host and mount its configuration directory:

```bash
docker run --rm -it \
  -v "$HOME/.codex:/root/.codex" \
  roomboy:latest
```

This example assumes the container runs as `root`, as in the current Dockerfile.

Alternatively, use a dedicated Docker volume for persistent configuration. Make sure the directory is writable by the user running Codex.

**Security warning:** Codex configuration may contain authentication credentials. Do not commit these files to Git, include them in a Docker image, or share them with untrusted containers.

## 6. Troubleshooting

### Port 1455 is already in use

Check which process is listening on the port:

```bash
ss -ltnp | grep ':1455'
```

Stop the conflicting process if appropriate, or investigate whether another Codex session is already using the callback port.

### Browser authentication fails

1. Confirm that the container was started with the required port mapping.
2. Check that the Codex process is listening on port `1455` inside the container.
3. Verify that the browser is opening the expected callback address.
4. Try device authentication if your account and CLI version support it.
5. Review the official Codex CLI documentation for the current Docker authentication requirements.

### Codex is not found

Check the global npm installation:

```bash
npm list --global --depth=0
```

If necessary, rebuild the image after resolving any package installation errors:

```bash
docker build --no-cache -t roomboy:latest .
```

## 7. Useful References

- [OpenAI Codex CLI — GitHub](https://github.com/openai/codex)
- [OpenAI Codex Documentation](https://developers.openai.com/codex/)

---

## Quick Start

Build the image:

```bash
docker build -t roomboy:latest .
```

Start the container with the OAuth callback port published:

```bash
docker run --rm -it \
  -p 127.0.0.1:1455:1455 \
  roomboy:latest
```

Launch Codex:

```bash
codex
```

For browser-based authentication, follow the instructions shown by Codex. If the local callback cannot reach the container, use a supported alternative authentication method or adjust the networking configuration.