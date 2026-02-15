# ntdda

**ntdda** is a Windows C application for **Discontinuous Deformation Analysis (DDA)**—a numerical method for modeling blocky rock masses, joints, contacts, stress, and deformation in geotechnical and rock mechanics applications.

---

## Repository Structure

- **~209** C source and header files
- **`include/`** – Headers for analysis, geometry, contacts, stress, materials, bolts, joints, DXF import, XML parsing
- **`resources/`** – Icons, bitmaps (earthquake, rockbolt, geometry, etc.)
- **`Makefile`** – Minimal (mostly `clean` target)
- **`winclean.bat`** – Windows build helper

---

## Technical Details

- **Platform:** Win32 GUI (menus, dialogs, toolbar, status bar)
- **Features:** Static/dynamic analysis, Mohr-Coulomb materials, rockbolts, joints, tunnels, DXF import, XML geometry/analysis files
- **References:** Gen-Hua Shi's DDA program; LAPACK for linear algebra
- **Analysis:** Blocks, contacts, time steps, iterations, fixed/load/measured points

---

## Commit History (247 commits)

| Period | Activity |
|--------|----------|
| **May 2001** | Initial revision, coordinate tracking for "Mission Peak" runout |
| **2002–2003** | DXF import, XML parsing, Roozbeh's contributions (joints, Mohr-Coulomb, dialogs, zoom, toolbar) |
| **~2005** | VS2005 project files, time-step fix, time info dialog |
| **Long gap** | Little or no activity |
| **~2010s** | Revival: "First compile and run on some of this code since 2002. Still works. C is such a cool programming language." |
| **Recent** | Whitespace/formatting cleanup, bolt allocation refactor, unit tests, compile warning fixes |

---

## Contributors

- **doolin** – Primary author
- **Roozbeh** – Mohr-Coulomb, joint drawing, geometry dialogs, zoom, toolbar buttons
