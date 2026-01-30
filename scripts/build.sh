#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building Absokino..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with CMake
cd "$BUILD_DIR"
cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build . --parallel $(nproc)

echo ""
echo "Build complete!"
echo "Run with: $BUILD_DIR/absokino"
echo "Or use: ./scripts/run.sh"
