#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build-linux-release}"
DIST_DIR="${DIST_DIR:-${PROJECT_ROOT}/dist/linux-release}"
PREFIX="${PREFIX:-/usr}"

if [[ ! -x "${BUILD_DIR}/dd-ssh" ]]; then
    echo "[DD-SSH] Release binary not found. Building first..."
    "${SCRIPT_DIR}/linux-build-release.sh"
fi

echo "[DD-SSH] Creating Linux release staging folder..."
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

DESTDIR="${DIST_DIR}" cmake --install "${BUILD_DIR}" --prefix "${PREFIX}"

cat > "${DIST_DIR}/README-RUN.txt" <<'EOF'
DD-SSH Linux release staging folder

This folder mirrors the install layout used by the first .deb package.
To test without installing system-wide:

  ./usr/bin/dd-ssh

This staging folder does not bundle Qt/libssh runtime libraries. It expects
runtime dependencies to be available from the Linux distribution packages.
EOF

echo "[DD-SSH] Linux release staging folder ready: ${DIST_DIR}"
echo "[DD-SSH] Local test command: ${DIST_DIR}/usr/bin/dd-ssh"
