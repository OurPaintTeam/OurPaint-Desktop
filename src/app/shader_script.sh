#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${1:-$ROOT_DIR/build}"
APP_DIR="$BUILD_DIR/src/app"

mkdir -p "$APP_DIR/shaders" "$APP_DIR/fonts"

# Main renderer shaders
if [ -d "$ROOT_DIR/src/rendering/renderer/shaders" ]; then
  cp -a "$ROOT_DIR/src/rendering/renderer/shaders/." "$APP_DIR/shaders/"
fi

# Extra text shaders/resources from legacy-cased folder
if [ -d "$ROOT_DIR/src/Rendering/renderer/shaders" ]; then
  cp -a "$ROOT_DIR/src/Rendering/renderer/shaders/." "$APP_DIR/shaders/"
fi

if [ -d "$ROOT_DIR/src/Rendering/renderer/fonts" ]; then
  cp -a "$ROOT_DIR/src/Rendering/renderer/fonts/." "$APP_DIR/fonts/"
fi
