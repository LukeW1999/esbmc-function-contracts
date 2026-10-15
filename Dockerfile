# ESBMC built from source at a commit on the v8.6 line, plus the examples and
# benchmarks the paper measures. v8.6 is the first line with the is_fresh fixes
# of the measured commit and the rounding-mode fix (#8045) that the 8.4 and 8.5
# releases lack. The v8.6 tag still moves and has no release build, so the
# commit is pinned here. Compiling takes about an hour.
# Build:  docker build -t esbmc-contracts .
# Smoke:  docker run --rm esbmc-contracts
# Shell:  docker run --rm -it esbmc-contracts bash
FROM ubuntu:24.04 AS build
ARG ESBMC_COMMIT=199741e8d8
ENV DEBIAN_FRONTEND=noninteractive PATH="/root/.local/bin:${PATH}"
RUN apt-get update && apt-get install -y --no-install-recommends \
    git ca-certificates curl unzip build-essential pkg-config libzstd-dev \
    python3 python3-pip python-is-python3 python3-toml cmake ninja-build bison flex gperf \
    llvm z3 default-jdk g++-multilib csmith libcsmith-dev libbz2-dev liblzma-dev \
    libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libmpfr-dev libncurses-dev \
    && pip3 install --break-system-packages meson \
    && rm -rf /var/lib/apt/lists/*
RUN git init -q /src && cd /src \
    && git fetch -q --depth 1 https://github.com/esbmc/esbmc.git "${ESBMC_COMMIT}" \
    && git checkout -q FETCH_HEAD && ./scripts/build.sh -b Release -e OFF

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
