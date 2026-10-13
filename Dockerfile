# ESBMC 8.5 release binary plus the examples and benchmarks the paper measures.
# Build:  docker build -t esbmc-contracts .
# Smoke:  docker run --rm esbmc-contracts
# Shell:  docker run --rm -it esbmc-contracts bash
FROM debian:bookworm-slim
ARG TARGETARCH
ARG ESBMC_VERSION=v8.5
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl unzip libc6-dev \
    && rm -rf /var/lib/apt/lists/*
RUN case "$TARGETARCH" in \
      amd64) zip=esbmc-linux.zip ;; \
      arm64) zip=esbmc-linux-armv8.zip ;; \
      *) echo "no ESBMC release for $TARGETARCH" && exit 1 ;; \
    esac \
    && curl -sSL -o /tmp/esbmc.zip "https://github.com/esbmc/esbmc/releases/download/${ESBMC_VERSION}/${zip}" \
    && unzip -q /tmp/esbmc.zip -d /opt && rm /tmp/esbmc.zip \
    && chmod +x /opt/release/bin/esbmc
ENV PATH="/opt/release/bin:${PATH}"
WORKDIR /work
COPY examples examples
COPY benchmarks benchmarks
COPY scripts scripts
RUN esbmc --version
CMD ["scripts/smoke.sh"]
