# Absokino

A modern Linux desktop video player with focus on maximum playback quality and accurate HDR diagnostics. Built with Qt 6, QML, Kirigami, and libmpv.

**Primary target:** KDE Plasma on Wayland with NVIDIA proprietary drivers
**Secondary:** AMD GPUs, X11 fallback

## Features

- **High-quality playback** via libmpv render API (direct embedding, not IPC)
- **HDR passthrough support** with honest, accurate diagnostics
- **Non-overlay UI** - controls are around the video, not on top of it
- **Clean fullscreen** - no UI, no click handling, just video
- **Breeze Dark** theme with Kirigami integration
- Hardware-accelerated decoding (VAAPI, NVDEC, etc.)
- Subtitle and audio track selection
- Chapter navigation
- A-B loop and frame stepping
- Recent files library
- Drag-and-drop support

## Building

### Dependencies

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-declarative qt6-quickcontrols2 \
    kirigami extra-cmake-modules mpv cmake base-devel
```

**Ubuntu/Debian (22.04+):**
```bash
sudo apt install qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick-controls qml6-module-qtquick-layouts \
    qml6-module-qtquick-dialogs qml6-module-qt-labs-platform \
    kirigami2-dev libmpv-dev cmake build-essential \
    extra-cmake-modules libkf6config-dev
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtquickcontrols2-devel kf6-kirigami-devel \
    mpv-libs-devel cmake gcc-c++ extra-cmake-modules \
    kf6-kconfig-devel
```

### Build Commands

```bash
# Quick build
./scripts/build.sh

# Or manually:
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(nproc)
```

### Run

```bash
./scripts/run.sh [optional-video-file]

# Or directly:
./build/absokino [optional-video-file]
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Play/Pause |
| `Left` | Seek -5 seconds |
| `Right` | Seek +5 seconds |
| `Shift+Left` | Seek -30 seconds |
| `Shift+Right` | Seek +30 seconds |
| `Up` | Volume up |
| `Down` | Volume down |
| `M` | Toggle mute |
| `F` | Toggle fullscreen |
| `Escape` | Exit fullscreen (does NOT quit app) |
| `,` | Frame step backward |
| `.` | Frame step forward |
| `[` | Decrease playback speed |
| `]` | Increase playback speed |

## Fullscreen Behavior

By design, fullscreen mode is **completely distraction-free**:

- **No UI elements** appear on screen
- **Single clicks do nothing** - no click-to-pause
- **Double-click** exits fullscreen
- **Space** toggles play/pause
- **Escape** exits fullscreen (does NOT quit the app)

This is intentional for maximum immersion.

## HDR and "Display HDR: Unknown"

### Why "Unknown"?

On Linux/Wayland, **reliably detecting if your display is actually receiving HDR signal is extremely difficult**. Unlike Windows, there's no standard API to query display HDR state.

Absokino is **honest about this uncertainty**:

- **Content HDR: Yes/No** - Based on video metadata (transfer/primaries). This is reliable.
- **Output mode: Passthrough/Tone-mapped** - What mpv is configured to do. This is reliable.
- **Display HDR: Unknown** - Whether your display is actually in HDR mode. This is almost always "Unknown" on Linux, and that's **normal**.

### How to verify HDR is working

1. Check your **monitor's OSD** - most HDR displays show an "HDR" indicator when receiving HDR signal
2. Run the **HDR Diagnostics** (Settings > Run HDR/Output Diagnostics)
3. Ensure **KDE Display Settings has HDR enabled**
4. Try **fullscreen mode** - HDR passthrough often works better in fullscreen

### HDR Configuration

In Settings > HDR Output Mode:

- **Auto (Passthrough preferred)** - Default. Attempts passthrough when possible.
- **Passthrough (HDR output)** - Forces passthrough, no tone-mapping.
- **Tone-map to SDR** - Always tone-maps HDR to SDR.

## Smoke Test Checklist

After building, verify these work:

1. [ ] Open file plays video
2. [ ] Double-click enters fullscreen
3. [ ] In fullscreen: no UI visible, single clicks do nothing
4. [ ] In fullscreen: Space pauses/plays
5. [ ] In fullscreen: Escape exits fullscreen (app stays open)
6. [ ] Subtitle track selection works (right-click > Subtitles)
7. [ ] Audio track selection works (right-click > Audio)
8. [ ] Volume slider works
9. [ ] Status bar shows codec/resolution/fps/bit-depth
10. [ ] HDR Diagnostics report generates without crash

## Architecture

```
src/
├── main.cpp              # Application entry point
├── mpvobject.cpp/h       # libmpv integration via render API
├── mpvrenderer.cpp/h     # OpenGL FBO renderer for Qt
├── playercontroller.cpp/h # Playback state management
├── settingsmanager.cpp/h  # Persistent settings
├── hdrdiagnostics.cpp/h   # HDR/output diagnostics
├── recentfilesmodel.cpp/h # Recent files for Library
├── trackmodel.cpp/h       # Audio/subtitle track model
└── chaptermodel.cpp/h     # Chapter navigation model

qml/
├── Main.qml              # Main window
├── components/           # Reusable UI components
│   ├── ControlBar.qml    # Bottom playback controls
│   ├── StatusBar.qml     # Top quality indicators
│   ├── LibraryDrawer.qml # Recent files panel
│   ├── SeekBar.qml       # Timeline with chapters
│   └── ...
└── dialogs/
    ├── SettingsDialog.qml
    └── DiagnosticsDialog.qml
```

## MPV Configuration

Default mpv options are set in `mpvobject.cpp::initializeMpv()`:

```cpp
// HDR passthrough (when supported)
setMpvOption("target-trc", "auto");
setMpvOption("target-colorspace-hint", "yes");
setMpvOption("tone-mapping", "auto");  // or "clip" for passthrough

// Hardware decoding
setMpvOption("hwdec", "auto-safe");

// Video output
setMpvOption("vo", "libmpv");
```

## Known Limitations

1. **Display HDR detection** - Cannot reliably confirm display HDR state on Linux
2. **Vulkan renderer** - Currently uses OpenGL via Qt; Vulkan would require different integration approach
3. **"Show UI on mouse move" in fullscreen** - Not yet implemented
4. **HDR passthrough** - Effectiveness depends on display, compositor, and driver support

## License

MIT License - see LICENSE file

## Contributing

Issues and PRs welcome at the project repository.
