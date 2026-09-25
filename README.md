# XPirep - Automated Pilot Report Plugin for X-Plane

**XPirep** is a lightweight, cross-platform X-Plane plugin that automatically tracks, records, and calculates essential flight parameters for pilot reporting (PIREP).

It detects key flight events (Block Out, Takeoff, Landing, Block In) in real time.

---

## Features

- **Automated Event Detection**:
  - **Block Out**: Triggered when engine(s) are running and ground speed exceeds 0.5 m/s.
  - **Takeoff**: Triggered when radio/AGL altitude exceeds 25 meters.
  - **Landing**: Triggered upon touchdown following a takeoff phase.
  - **Block In**: Triggered when engines are shut down and ground speed drops below 0.5 m/s.
- **Flight & Fuel Metrics**:
  - Real-time calculation of Block Time and Flight Time.
  - Fuel consumption tracking (Block Fuel & Flight Fuel).
  - Flight distance tracking (Nautical Miles).
- **Persistent Flight State**:
  - Periodic auto-save in flight.
  - Automatically restores unfinished flight tracking upon X-Plane launch.
- **In-Game Interface**:
  - Native X-Plane floating window showing real-time metrics and event summaries.
  - Controls to manually **START**, **STOP**, or **RESET** the flight recorder.
  - Toggleable visibility via X-Plane's `Plugins` menu.
- **Cross-Platform**:
  - Full native support for **Linux** and **Windows** (64-bit).

**Compatible with X-Plane 11 and X-Plane 12.**

---

## Installation

1. Download the latest release package.
2. Extract the `XPirep` directory into your X-Plane plugins folder:
   ```text
   X-Plane 11 (or 12)/
   └── Resources/
       └── plugins/
           └── XPirep/
               └── 64/
                   ├── lin.xpl   (Linux 64-bit)
                   └── win.xpl   (Windows 64-bit)
   ```

---

## Building from Source

### Prerequisites

- **Linux** (Tested on Linux Mint 22 / Debian 13)
- `g++` (64-bit) and `make`
- MinGW-w64 (`x86_64-w64-mingw32-g++`) for cross-compiling the Windows binary
- X-Plane SDK (XPLM300 / XPLM301 compatible)

### SDK Directory Layout

Ensure the X-Plane C SDK headers and libraries are placed under the `SDK/` folder in your repository root before building:

### Compilation Commands

Build both Linux (lin.xpl) and Windows (win.xpl) binaries with a single command:

```bash
# Build both Linux and Windows binaries
make all

# Build Linux target only (lin.xpl)
make linux

# Build Windows target only (win.xpl)
make windows

# Clean build artifacts
make clean
```
The compiled binaries will be generated inside the build/XPirep/64/ directory.

---

## License & Credits

XPirep is free and open-source software released under the
GNU General Public License v3.0.

See the `LICENSE` file for the complete license text.

XPirep may include or depend on third-party components distributed
under their own licenses. Their respective licenses remain applicable
to those components.

- Author: Elekaj34
- Developed against X-Plane SDK 3.0+
- OpenGL & XPLM API integration
