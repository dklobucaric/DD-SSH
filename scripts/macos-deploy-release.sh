#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build-macos-release}"
DIST_DIR="${DIST_DIR:-${PROJECT_ROOT}/dist/macos}"
QT_DIR="${QT_DIR:-${HOME}/Qt/6.11.1/macos}"
VERSION="${DD_SSH_MACOS_VERSION:-0.1.7.1}"
MACOS_ARCH="${MACOS_ARCH:-x86_64}"
APP_NAME="${APP_NAME:-DD-SSH}"
SOURCE_APP="${BUILD_DIR}/dd-ssh.app"
DIST_APP="${DIST_DIR}/${APP_NAME}.app"
DMG_ROOT="${DIST_DIR}/dmg-root"
DMG_FILE="${DIST_DIR}/${APP_NAME}-${VERSION}-macOS-${MACOS_ARCH}.dmg"
DEPS_REPORT="${DIST_DIR}/${APP_NAME}-${VERSION}-macOS-${MACOS_ARCH}-otool-report.txt"
DEPS_WARNINGS="${DIST_DIR}/${APP_NAME}-${VERSION}-macOS-${MACOS_ARCH}-dependency-warnings.txt"
CODESIGN_ADHOC="${CODESIGN_ADHOC:-1}"
STRICT_DEP_AUDIT="${STRICT_DEP_AUDIT:-0}"

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

list_otool_targets() {
    local app="$1"
    {
        [[ -f "${app}/Contents/MacOS/dd-ssh" ]] && echo "${app}/Contents/MacOS/dd-ssh"
        find "${app}/Contents/Frameworks" -type f \( -name "*.dylib" -o -perm -111 \) 2>/dev/null || true
        find "${app}/Contents/PlugIns" -type f -perm -111 2>/dev/null || true
    } | awk '!seen[$0]++'
}

write_otool_report() {
    local app="$1"
    local report="$2"

    {
        echo "DD-SSH macOS dependency audit"
        echo "Version: ${VERSION}"
        echo "Architecture: ${MACOS_ARCH}"
        echo "App: ${app}"
        echo "Generated: $(date -u '+%Y-%m-%dT%H:%M:%SZ')"
        echo
        while IFS= read -r target; do
            [[ -f "${target}" ]] || continue
            echo "## ${target#${app}/}"
            otool -L "${target}" || true
            echo
        done < <(list_otool_targets "${app}")
    } > "${report}"
}

find_problem_dependencies() {
    local report="$1"
    awk '
        /^[[:space:]]*\/Users\// || /^[[:space:]]*\/usr\/local\// || /^[[:space:]]*\/opt\/homebrew\// {
            print
        }
    ' "${report}" || true
}

write_dmg_readme() {
    local output="$1"
    cat > "${output}" <<EOFMSG
DD-SSH ${VERSION} macOS tester build

Install:
1. Drag DD-SSH.app to Applications.
2. Launch it from Applications.
3. Because this tester build is unsigned/not notarized, macOS Gatekeeper may block the first launch.
4. Use right-click / Control-click -> Open, then confirm Open.

Notes:
- Initial macOS package target: Intel x86_64.
- Recommended: macOS 13 or newer for the Qt 6.11 build line.
- Apple Silicon may run this Intel app through Rosetta 2; native arm64/universal builds are planned later.
- Keep dd-ssh.json private. plain-v1 secrets are human-readable and not encrypted in this early 0.x line.

EOFMSG
}

echo "[DD-SSH] Creating macOS deploy folder..."
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"
cp -R "${SOURCE_APP}" "${DIST_APP}"

echo "[DD-SSH] Running macdeployqt..."
"${MACDEPLOYQT}" "${DIST_APP}" -verbose=2

echo "[DD-SSH] Bundling Homebrew dylib dependencies discovered by otool..."
scan_and_bundle_homebrew_dylibs "${DIST_APP}"

echo "[DD-SSH] Writing dependency audit report..."
write_otool_report "${DIST_APP}" "${DEPS_REPORT}"
find_problem_dependencies "${DEPS_REPORT}" > "${DEPS_WARNINGS}"

if [[ -s "${DEPS_WARNINGS}" ]]; then
    echo "[DD-SSH] WARNING: Dependency audit found local/Homebrew paths after bundling:"
    cat "${DEPS_WARNINGS}"
    echo "[DD-SSH] Full dependency report: ${DEPS_REPORT}"
    if [[ "${STRICT_DEP_AUDIT}" == "1" ]]; then
        echo "ERROR: STRICT_DEP_AUDIT=1 and dependency warnings were found." >&2
        exit 1
    fi
else
    echo "[DD-SSH] Dependency audit OK: no /Users, /usr/local, or /opt/homebrew paths remain in otool output."
    rm -f "${DEPS_WARNINGS}"
fi

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
write_dmg_readme "${DMG_ROOT}/README_FIRST.txt"

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
echo "[DD-SSH] Dependency audit: ${DEPS_REPORT}"
echo "[DD-SSH] Local test: open \"${DIST_APP}\""
echo "[DD-SSH] DMG test: open \"${DMG_FILE}\""
echo "[DD-SSH] Inspect deps: otool -L \"${DIST_APP}/Contents/MacOS/dd-ssh\""
echo "[DD-SSH] Strict audit option: STRICT_DEP_AUDIT=1 ./scripts/macos-deploy-release.sh"
