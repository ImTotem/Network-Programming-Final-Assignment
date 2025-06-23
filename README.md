# 빌드 자동화
```bash
./build.sh
```
- docker 필수

# 시스템 종속성 설치
## Linux
```bash
sudo apt update
sudo apt install libwebkit2gtk-4.1-dev \
  build-essential \
  curl \
  wget \
  file \
  libxdo-dev \
  libssl-dev \
  libayatana-appindicator3-dev \
  librsvg2-dev
```

# 공통
## Node.js 설치

```bash
# nvm 다운로드 및 설치:
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.1/install.sh | bash

# Node.js 다운로드 및 설치:
nvm install 20

# Node.js 버전 확인:
node -v # "v20.19.2"가 출력되어야 합니다.
nvm current # "v20.19.2"가 출력되어야 합니다.

npm 버전 확인:
npm -v # 10.8.2가 출력되어야 합니다.

# yarn 설치:
npm i -g yarn # 프로젝트가 yarn 사용중이라 필요
```

## Rust 설치
```bash
curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh
```

## CMake 설치
```bash
sudo apt install cmake
```

# 빌드
## Backend
### Linux
```bash
cd ./backend
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j $(nproc)
```

## Frontend
`./frontend/excalidraw-app/collab/Portal.tsx`에서 43번 줄을 서버 ip와 port에 맞게 수정
```typescript
socket.connect("10.211.55.9", 3002);
```
### Linux / Mac
```bash
cd ./frontend/excalidraw-app/src-tauri
yarn install
rm -rf ./target/
yarn tauri build --debug  # debug는 선택
```
