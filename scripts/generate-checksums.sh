#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
ARTIFACT_DIR="${1:-${PROJECT_ROOT}/dist}"
OUTPUT_FILE="${2:-${PROJECT_ROOT}/dist/SHA256SUMS}"

if [[ ! -d "${ARTIFACT_DIR}" ]]; then
    echo "ERROR: artifact directory not found: ${ARTIFACT_DIR}" >&2
    echo "Usage: $0 [artifact-dir] [output-file]" >&2
    exit 1
fi

if command -v sha256sum >/dev/null 2>&1; then
    checksum_cmd=(sha256sum)
elif command -v shasum >/dev/null 2>&1; then
    checksum_cmd=(shasum -a 256)
else
    echo "ERROR: neither sha256sum nor shasum was found." >&2
    exit 1
fi

mkdir -p "$(dirname -- "${OUTPUT_FILE}")"
: > "${OUTPUT_FILE}"

mapfile -d '' artifacts < <(
    find "${ARTIFACT_DIR}" -type f \
        \( -name '*.zip' -o -name '*.deb' -o -name '*.dmg' -o -name '*.AppImage' -o -name '*.msi' -o -name '*.pkg' -o -name '*.tar.gz' -o -name '*.tgz' \) \
        ! -name 'SHA256SUMS' \
        -print0 | sort -z
)

if [[ "${#artifacts[@]}" -eq 0 ]]; then
    echo "ERROR: no release artifacts found under ${ARTIFACT_DIR}" >&2
    echo "Expected: .zip, .deb, .dmg, .AppImage, .msi, .pkg, .tar.gz, or .tgz" >&2
    exit 1
fi

for artifact in "${artifacts[@]}"; do
    rel_path="${artifact#${PROJECT_ROOT}/}"
    hash="$(${checksum_cmd[@]} "${artifact}" | awk '{print $1}')"
    printf '%s  %s\n' "${hash}" "${rel_path}" >> "${OUTPUT_FILE}"
done

echo "[DD-SSH] SHA256 checksums written to: ${OUTPUT_FILE}"
cat "${OUTPUT_FILE}"
