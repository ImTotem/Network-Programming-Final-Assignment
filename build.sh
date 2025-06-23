#!/bin/bash

# 스크립트 실행 중 오류가 발생하면 즉시 중단
set -e

# 빌드 결과물을 저장할 로컬 디렉토리 생성
echo ">>> Creating ./dist directory for build artifacts..."
mkdir -p ./dist

# docker-compose를 사용하여 Docker 이미지 빌드
echo ">>> Building project using Docker Compose..."
sudo docker compose build

# 빌드된 이미지를 기반으로 임시 컨테이너 생성
echo ">>> Creating a temporary container to extract artifacts..."
CONTAINER_ID=$(docker create project-builder)

# 임시 컨테이너에서 로컬 ./dist 디렉토리로 빌드 결과물 복사
echo ">>> Copying artifacts from the container to ./dist..."
sudo docker cp "${CONTAINER_ID}:/artifacts/." "./dist/"

# 임시 컨테이너 삭제
echo ">>> Cleaning up the temporary container..."
sudo docker rm -f "${CONTAINER_ID}"

echo ""
echo "✅ Build finished successfully!"
echo "Artifacts are located in the ./dist directory."
ls -l ./dist/
