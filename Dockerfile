
# Roomboy — ambiente base de desenvolvimento
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Ferramentas básicas, Git e compilação C/C++.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    build-essential \
    cmake \
    gdb \
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    procps \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Instala Node.js 24.x.
RUN curl -fsSL https://deb.nodesource.com/setup_24.x -o /tmp/nodesource_setup.sh \
    && bash /tmp/nodesource_setup.sh \
    && apt-get update \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -f /tmp/nodesource_setup.sh \
    && rm -rf /var/lib/apt/lists/*

# Install REA and Codex CLI
RUN npm install --global rea-agents @openai/codex

# Configure REA integration for Codex
RUN rea setup --yes --client codex

WORKDIR /workspace

# Directory for host-mounted projects
RUN mkdir -p /workspace/project

CMD ["/bin/bash"]
