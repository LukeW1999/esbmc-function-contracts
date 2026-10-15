# ESBMC v8.6 built from source, plus the examples and benchmarks the paper
# measures. v8.6 is the first tag with the is_fresh fixes of the measured
# commit and the rounding-mode fix (#8045) that the 8.4 and 8.5 releases lack.
# There is no release build of v8.6 yet, so this compiles it (about an hour).
# Build:  docker build -t esbmc-contracts .
# Smoke:  docker run --rm esbmc-contracts
# Shell:  docker run --rm -it esbmc-contracts bash
FROM ubuntu:24.04 AS build
ARG ESBMC_VERSION=v8.6
ENV DEBIAN_FRONTEND=noninteractive PATH="/root/.local/bin:${PATH}"
RUN apt-get update && apt-get install -y --no-install-recommends \
    git ca-certificates curl unzip build-essential pkg-config libzstd-dev \
    python3 python3-pip python-is-python3 python3-toml cmake ninja-build bison flex gperf \
    llvm z3 default-jdk g++-multilib csmith libcsmith-dev libbz2-dev liblzma-dev \
    libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libmpfr-dev libncurses-dev \
    && pip3 install --break-system-packages meson \
    && rm -rf /var/lib/apt/lists/*
RUN git clone --depth 1 --branch "${ESBMC_VERSION}" https://github.com/esbmc/esbmc.git /src \
    && cd /src && ./scripts/build.sh -b Release -e OFF

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends libc6-dev \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/release /opt/release
ENV PATH="/opt/release/bin:${PATH}"
WORKDIR /work
COPY examples examples
COPY benchmarks benchmarks
COPY scripts scripts
RUN esbmc --version
CMD ["scripts/smoke.sh"]
