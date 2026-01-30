#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
EXECUTABLE="$BUILD_DIR/absokino"

# Check if built
if [ ! -f "$EXECUTABLE" ]; then
    echo "Absokino not built. Building first..."
    "$SCRIPT_DIR/build.sh"
fi

# Run with optional file argument
echo "Starting Absokino..."
exec "$EXECUTABLE" "$@"
