# =========================================================================================
# 스테이지 1: 모든 빌드 도구와 의존성을 설치하는 기본 빌더 이미지
# =========================================================================================
FROM ubuntu:22.04 AS builder-base

ENV DEBIAN_FRONTEND=noninteractive

# GCC 13 컴파일러를 설치하기 위한 의존성 추가
RUN apt-get update && apt-get install -y \
    software-properties-common \
    build-essential \
    curl \
    wget \
    file \
    git \
    libssl-dev \
    # Tauri (Frontend) 의존성
    libwebkit2gtk-4.1-dev \
    libxdo-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev && \
    rm -rf /var/lib/apt/lists/*

# 공식 PPA를 통해 GCC-13 및 G++-13 컴파일러를 설치합니다.
RUN add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y gcc-13 g++-13

# 기본 C/C++ 컴파일러를 방금 설치한 13 버전으로 지정합니다.
ENV CC=gcc-13 CXX=g++-13

# 최신 버전의 CMake를 수동으로 다운로드 및 설치합니다.
ARG CMAKE_VERSION=3.29.3
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-aarch64.sh -O /tmp/cmake-install.sh && \
    chmod +x /tmp/cmake-install.sh && \
    /tmp/cmake-install.sh --prefix=/usr/local --skip-license && \
    rm /tmp/cmake-install.sh

# Rust 설치
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# Node.js (v20) 및 Yarn 설치
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash -
RUN apt-get install -y nodejs
RUN npm install -g yarn


# =========================================================================================
# 스테이지 2: C++ 백엔드 빌드
# =========================================================================================
FROM builder-base AS backend-builder

WORKDIR /app

# 백엔드 소스만 복사하여 빌드
COPY ./backend ./backend
RUN cd backend && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -- -j $(nproc)


# =========================================================================================
# 스테이지 3: Tauri 프론트엔드 빌드
# =========================================================================================
FROM builder-base AS frontend-builder

WORKDIR /app

# 프론트엔드 소스만 복사
COPY ./frontend ./frontend

# 환경 변수(IP, Port)를 build-arg로부터 받음
ARG VITE_APP_WS_SERVER_URL
ARG VITE_APP_WS_SERVER_PORT
ENV VITE_APP_WS_SERVER_URL=${VITE_APP_WS_SERVER_URL}
ENV VITE_APP_WS_SERVER_PORT=${VITE_APP_WS_SERVER_PORT}

# 의존성 설치 및 빌드
WORKDIR /app/frontend/excalidraw-app
RUN yarn install
RUN rm -rf ./src-tauri/target && yarn tauri build


# =========================================================================================
# 스테이지 4: 최종 결과물만 모으는 경량 이미지
# =========================================================================================
FROM ubuntu:22.04

WORKDIR /artifacts

# 백엔드 빌드 결과물 복사
COPY --from=backend-builder /app/backend/build/collab_server ./backend/

# [수정됨] 프론트엔드 빌드 결과물 복사 (이제 AppImage 실행 파일을 생성)
COPY --from=frontend-builder /app/frontend/excalidraw-app/src-tauri/target/release/bundle/appimage/*.AppImage ./frontend/
