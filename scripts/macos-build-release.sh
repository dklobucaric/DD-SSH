#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build-macos-release}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"
QT_DIR="${QT_DIR:-${HOME}/Qt/6.11.1/macos}"
MACOS_ARCH="${MACOS_ARCH:-x86_64}"

append_pkg_config_path() {
    local path="$1"
    if [[ -d "${path}" ]]; then
        if [[ -n "${PKG_CONFIG_PATH:-}" ]]; then
            PKG_CONFIG_PATH="${path}:${PKG_CONFIG_PATH}"
        else
            PKG_CONFIG_PATH="${path}"
        fi
    fi
}

append_pkg_config_path "/usr/local/opt/libssh/lib/pkgconfig"
append_pkg_config_path "/usr/local/opt/openssl@3/lib/pkgconfig"
append_pkg_config_path "/usr/local/opt/zlib/lib/pkgconfig"
append_pkg_config_path "/opt/homebrew/opt/libssh/lib/pkgconfig"
append_pkg_config_path "/opt/homebrew/opt/openssl@3/lib/pkgconfig"
append_pkg_config_path "/opt/homebrew/opt/zlib/lib/pkgconfig"
export PKG_CONFIG_PATH

if [[ ! -d "${QT_DIR}" ]]; then
    cat >&2 <<EOFMSG
ERROR: Qt directory not found: ${QT_DIR}

Set QT_DIR explicitly, for example:
  QT_DIR="${HOME}/Qt/6.11.1/macos" ./scripts/macos-build-release.sh
EOFMSG
    exit 1
fi

if [[ ! -x "${QT_DIR}/bin/macdeployqt" && ! -x "${QT_DIR}/bin/macdeployqt6" ]]; then
    echo "WARNING: macdeployqt was not found under ${QT_DIR}/bin. Build may work, deployment will not." >&2
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found." >&2
    exit 1
fi

cmake_args=(
    -S "${PROJECT_ROOT}"
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
    -DCMAKE_PREFIX_PATH="${QT_DIR}"
    -DCMAKE_OSX_ARCHITECTURES="${MACOS_ARCH}"
)

if [[ "${GENERATOR}" == "Ninja" ]]; then
    if command -v ninja >/dev/null 2>&1; then
        cmake_args=( -G "Ninja" "${cmake_args[@]}" )
    else
        echo "[DD-SSH] Ninja not found; falling back to CMake default generator."
    fi
else
    cmake_args=( -G "${GENERATOR}" "${cmake_args[@]}" )
fi

echo "[DD-SSH] Configuring macOS ${BUILD_TYPE} build..."
echo "[DD-SSH] Qt dir: ${QT_DIR}"
echo "[DD-SSH] Architecture: ${MACOS_ARCH}"
echo "[DD-SSH] PKG_CONFIG_PATH: ${PKG_CONFIG_PATH:-<empty>}"
cmake "${cmake_args[@]}"

echo "[DD-SSH] Building..."
cmake --build "${BUILD_DIR}"

echo "[DD-SSH] Build complete."
echo "[DD-SSH] App bundle: ${BUILD_DIR}/dd-ssh.app"
echo "[DD-SSH] Run: open \"${BUILD_DIR}/dd-ssh.app\""
