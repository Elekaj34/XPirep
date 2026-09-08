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
- **In-Game Interface**:
  - Native X-Plane floating window showing real-time metrics and event summaries.
  - Controls to manually **START**, **STOP**, or **RESET** the flight recorder.
  - Toggleable visibility via X-Plane's `Plugins` menu.

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
- X-Plane SDK (XPLM300 / XPLM301 compatible)

### SDK Directory Layout

Ensure the X-Plane C SDK headers and libraries are placed under the `SDK/` folder in your repository root before building:

### Compilation Commands

Build both Linux (lin.xpl) and Windows (win.xpl) binaries with a single command:

```bash
# Build for Linux
make all

# Clean build artifacts
make clean
```

---

## License & Credits

- Author: Elekaj34
- Developed against X-Plane SDK 3.0+
- OpenGL & XPLM API integration

