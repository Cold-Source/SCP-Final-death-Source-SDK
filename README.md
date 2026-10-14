# SCP:FD Source SDK Based on Cold Source SDK
### game downloads
### coming soon...
// ### [de](https://github.com/Cold-Source/Cold-Source-Game/tree/gamepadui)
The foundational source code for the **SCP: Final death** development environment. This SDK is a modernized branch of the Source Engine, providing the framework for advanced community projects.

Currently contains updated game code for:
*   **Source SP**
*   **Source MP**

---

## 🛠 Build Instructions

### Windows
**Requirements:**
*   **Source Multiplayer Game** or **Source SDK 2013 Multiplayer**
*   **Visual Studio 2022 or Visual Studio 2026** with "Desktop development with C++"
*   **Python 3.13** or later

**Setup:**
1.  Navigate to the `src` directory.
2.  Run `createallprojects.bat` to generate `everything.sln`.
3.  Open the solution in Visual Studio and use **Build > Build Solution**.

### There is currently no full support Linux
---

## 🗺 Roadmap & Future Plans

We are actively working on modernizing the engine core. Our current development focus includes:

### 🟢 Core Architecture
- [x] **x64 Architecture:** — Full migration to 64-bit for improved memory addressing and performance.
- [ ] **Jolt Physics Integration:** Replacing the legacy Havok physics engine with **Jolt Physics** for better multithreading and stability.

### 🟡 Graphics & UI
- [ ] **Panorama UI** — `IN PROGRESS (50%)`
  > Implementing the Panorama framework to replace legacy VGUI components.
- [ ] **Shader Overhaul:** Significant improvements to the rendering pipeline and shader system.

### 🔵 Scripting & Extensibility
- [x] **Lua Integration:** Embedding Lua support for flexible, high-level game logic and modding.
- [x] **Modern Tooling:** Ensuring full compatibility with Visual Studio 2022 and modern C++ standards.

---

## 📜 License & Distribution

This project is licensed under the same terms as the original Valve Source SDK. When distributing your mod:
*   Ensure compliance with the [Steam Distribution Guidelines](https://steamgames.com).
*   Mods distributed on Steam **must** be built against the Steam Runtime.

---
* by Cold Source organization.*
