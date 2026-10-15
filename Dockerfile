# ESBMC built from source at the measured commit, with the goto-contractor
# (ibex) switched off, plus CBMC 6.8.0 and the examples and benchmarks the
# paper measures. Why not a release: 8.4 and 8.5 lack is_fresh fixes the
# measurement has and start with the FPU rounding mode left upward by ibex
# (#8043), and the later v8.6 line changes the Enforce harness again (#8042)
# and fails the ML-KEM contracts. Compiling takes about an hour.
# Build:  docker build -t esbmc-contracts .
# Smoke:  docker run --rm esbmc-contracts
# Shell:  docker run --rm -it esbmc-contracts bash
FROM ubuntu:24.04 AS build
ARG ESBMC_COMMIT=2f719f59e03ab98fa54c5e89757e6d95527daad9
ENV DEBIAN_FRONTEND=noninteractive PATH="/root/.local/bin:${PATH}"
RUN apt-get update && apt-get install -y --no-install-recommends \
    git ca-certificates curl wget unzip build-essential pkg-config libzstd-dev \
    zlib1g-dev libxml2-dev libtinfo-dev libedit-dev \
    python3 python3-pip python-is-python3 python3-toml cmake ninja-build bison flex gperf \
    llvm z3 default-jdk g++-multilib csmith libcsmith-dev libbz2-dev liblzma-dev \
    libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libmpfr-dev libncurses-dev \
    && pip3 install --break-system-packages meson \
    && rm -rf /var/lib/apt/lists/*
RUN git init -q /src && cd /src \
    && git fetch -q --depth 1 https://github.com/esbmc/esbmc.git "${ESBMC_COMMIT}" \
    && git checkout -q FETCH_HEAD \
    && sed -i 's/-DENABLE_GOTO_CONTRACTOR=On/-DENABLE_GOTO_CONTRACTOR=Off/' scripts/build.sh \
    && ./scripts/build.sh -b Release -e OFF

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends libc6-dev ca-certificates curl \
    && curl -sSL -o /tmp/cbmc.deb https://github.com/diffblue/cbmc/releases/download/cbmc-6.8.0/ubuntu-24.04-cbmc-6.8.0-Linux.deb \
    && apt-get install -y --no-install-recommends /tmp/cbmc.deb && rm /tmp/cbmc.deb \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/release /opt/release
ENV PATH="/opt/release/bin:${PATH}"
WORKDIR /work
COPY examples examples
COPY benchmarks benchmarks
COPY scripts scripts
RUN esbmc --version && cbmc --version
CMD ["scripts/smoke.sh"]
