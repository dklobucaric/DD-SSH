#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build-macos-release}"
DIST_DIR="${DIST_DIR:-${PROJECT_ROOT}/dist/macos}"
QT_DIR="${QT_DIR:-${HOME}/Qt/6.11.1/macos}"
VERSION="${DD_SSH_MACOS_VERSION:-0.1.6.2}"
MACOS_ARCH="${MACOS_ARCH:-x86_64}"
APP_NAME="${APP_NAME:-DD-SSH}"
SOURCE_APP="${BUILD_DIR}/dd-ssh.app"
DIST_APP="${DIST_DIR}/${APP_NAME}.app"
DMG_ROOT="${DIST_DIR}/dmg-root"
DMG_FILE="${DIST_DIR}/${APP_NAME}-${VERSION}-macOS-${MACOS_ARCH}.dmg"
CODESIGN_ADHOC="${CODESIGN_ADHOC:-1}"

if [[ ! -d "${SOURCE_APP}" ]]; then
    echo "[DD-SSH] Build app not found. Building first..."
    "${SCRIPT_DIR}/macos-build-release.sh"
fi

MACDEPLOYQT="${QT_DIR}/bin/macdeployqt"
if [[ ! -x "${MACDEPLOYQT}" ]]; then
    MACDEPLOYQT="${QT_DIR}/bin/macdeployqt6"
fi
if [[ ! -x "${MACDEPLOYQT}" ]]; then
    echo "ERROR: macdeployqt not found under ${QT_DIR}/bin." >&2
    exit 1
fi

resolve_path() {
    local path="$1"
    if [[ -L "${path}" ]]; then
        local dir target
        dir="$(cd -- "$(dirname -- "${path}")" && pwd -P)"
        target="$(readlink "${path}")"
        if [[ "${target}" != /* ]]; then
            target="${dir}/${target}"
        fi
        resolve_path "${target}"
    else
        local dir base
        dir="$(cd -- "$(dirname -- "${path}")" && pwd -P)"
        base="$(basename -- "${path}")"
        echo "${dir}/${base}"
    fi
}

is_bundle_candidate() {
    local dep="$1"
    case "${dep}" in
        /usr/local/*|/opt/homebrew/*)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

scan_and_bundle_homebrew_dylibs() {
    local app="$1"
    local bin="${app}/Contents/MacOS/dd-ssh"
    local frameworks="${app}/Contents/Frameworks"
    mkdir -p "${frameworks}"

    local -a targets=("${bin}")
    local changed=1
    local guard=0

    while [[ "${changed}" -eq 1 && "${guard}" -lt 12 ]]; do
        changed=0
        guard=$((guard + 1))

        local current_targets=("${targets[@]}")
        for target in "${current_targets[@]}"; do
            [[ -f "${target}" ]] || continue
            while IFS= read -r dep; do
                [[ -n "${dep}" ]] || continue
                if ! is_bundle_candidate "${dep}"; then
                    continue
                fi

                local base dest real
                base="$(basename -- "${dep}")"
                dest="${frameworks}/${base}"

                if [[ ! -f "${dest}" ]]; then
                    real="$(resolve_path "${dep}")"
                    echo "[DD-SSH] Bundling dylib: ${dep} -> ${dest}"
                    cp -p "${real}" "${dest}"
                    chmod u+w "${dest}" || true
                    install_name_tool -id "@executable_path/../Frameworks/${base}" "${dest}" || true
                    targets+=("${dest}")
                    changed=1
                fi

                echo "[DD-SSH] Rewriting dependency in $(basename -- "${target}"): ${dep}"
                install_name_tool -change "${dep}" "@executable_path/../Frameworks/${base}" "${target}" || true
            done < <(otool -L "${target}" | tail -n +2 | awk '{print $1}')
        done
    done
}

echo "[DD-SSH] Creating macOS deploy folder..."
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"
cp -R "${SOURCE_APP}" "${DIST_APP}"

echo "[DD-SSH] Running macdeployqt..."
"${MACDEPLOYQT}" "${DIST_APP}" -verbose=2

echo "[DD-SSH] Bundling Homebrew dylib dependencies discovered by otool..."
scan_and_bundle_homebrew_dylibs "${DIST_APP}"

if [[ "${CODESIGN_ADHOC}" == "1" ]]; then
    echo "[DD-SSH] Applying ad-hoc signature for local testing..."
    if command -v codesign >/dev/null 2>&1; then
        codesign --force --deep --sign - "${DIST_APP}" || echo "[DD-SSH] WARNING: ad-hoc codesign failed; continuing with unsigned app."
    else
        echo "[DD-SSH] WARNING: codesign not found; app remains unsigned."
    fi
fi

echo "[DD-SSH] Creating DMG staging folder..."
rm -rf "${DMG_ROOT}" "${DMG_FILE}"
mkdir -p "${DMG_ROOT}"
cp -R "${DIST_APP}" "${DMG_ROOT}/${APP_NAME}.app"
ln -s /Applications "${DMG_ROOT}/Applications"

if [[ "${CODESIGN_ADHOC}" == "1" && -x "$(command -v codesign || true)" ]]; then
    codesign --force --deep --sign - "${DMG_ROOT}/${APP_NAME}.app" || true
fi

echo "[DD-SSH] Creating DMG: ${DMG_FILE}"
hdiutil create \
    -volname "${APP_NAME}" \
    -srcfolder "${DMG_ROOT}" \
    -ov \
    -format UDZO \
    "${DMG_FILE}"

rm -rf "${DMG_ROOT}"

echo "[DD-SSH] macOS deploy complete."
echo "[DD-SSH] App: ${DIST_APP}"
echo "[DD-SSH] DMG: ${DMG_FILE}"
echo "[DD-SSH] Local test: open \"${DIST_APP}\""
echo "[DD-SSH] DMG test: open \"${DMG_FILE}\""
echo "[DD-SSH] Inspect deps: otool -L \"${DIST_APP}/Contents/MacOS/dd-ssh\""
