# ntdda Tauri Port Plan

Two-stage port of ntdda from Win32 GUI to Tauri (Rust + WebView).

The numerical core (`libdda.a`) already builds on macOS as pure C99.
Only the GUI layer (`platform/win32/`, 18 files) needs replacing.

---

## Hard Rules

1. **DO NOT modify any existing C code.** No new accessor functions, no new
   headers, no changes to structs, no `dda_step()`. The C codebase is frozen.
   If the Rust FFI approach hits a wall because of this, stop and rethink
   rather than patching the C side.
2. Rust replicates C struct layouts via `#[repr(C)]` to read fields directly
   from pointers returned by the existing C API. This is fragile but avoids
   changing C code. Must be validated with static assertions on struct size
   and field offsets.
3. **React** for the frontend (TypeScript).
4. **Project location:** `src/platform/tauri/` — parallel to `src/platform/win32/`.
5. **Keep libxml2** as a dependency for now.

---

## Decisions (resolved)

| Question | Answer |
|---|---|
| C struct access strategy | Replicate layouts in Rust (`#[repr(C)]`), do not touch C |
| Frontend framework | React (TypeScript) |
| Project location | `src/platform/tauri/` |
| libxml2 dependency | Keep for now |

---

## Architecture

```
┌─────────────────────────────────────────────┐
│  Frontend (system WebView)                  │
│  React + TypeScript + Canvas 2D             │
│  UI controls, rendering, interaction        │
├─────────────────────────────────────────────┤
│  Tauri IPC (invoke / events)                │
├─────────────────────────────────────────────┤
│  Rust backend (thin FFI glue)               │
│  #[repr(C)] struct mirrors for direct read  │
│  Serializes C structs → JSON for frontend   │
├─────────────────────────────────────────────┤
│  libdda.a (existing C99 code, FROZEN)       │
│  Built by build.rs via cc crate             │
└─────────────────────────────────────────────┘
```

### Struct access approach

Since we cannot add C accessor functions, the Rust side must mirror the
C struct layouts exactly. This requires:

- `#[repr(C)]` structs matching `Geometrydata`, `Analysisdata`, `DDA`, etc.
- Careful attention to pointer sizes, padding, alignment
- `double **` fields (2D arrays) read by dereferencing raw pointers
- Compile-time assertions: `assert!(size_of::<Geometrydata>() == ...)` validated
  against the C compiler's `sizeof(Geometrydata)` (printed by a test program)
- If a struct's layout is too complex or contains function pointers in
  unpredictable positions, we may only mirror the prefix up to the fields
  we actually need, using offset-based reads instead

### Risk: struct layout mismatch

If the Rust mirror drifts from the C layout, we get silent data corruption.
Mitigations:
- A C test program (compiled by build.rs) that prints `offsetof()` and
  `sizeof()` for every field we access — Rust build asserts against these
- Only access fields we actually need (don't mirror the entire struct if
  we only read 5 fields)
- Document which C header version each Rust mirror was built against

---

## Stage 1: Load, Run, View — COMPLETE

Goal: open geometry and analysis files, run DDA, view results.
Replaces the core workflow without the interactive drawing editor.
**Status**: All items complete. Geometry loads, ddacut runs, analysis loads,
ddanalysis runs (500 timesteps in ~13ms), results display on Canvas 2D.

### Win32 workflow being replicated

```
1. File → Geometry → Open .geo file
2. Geometry → Apply (calls gdata_read_input_file → ddacut → renders blocks)
3. File → Analysis → Open .ana file
4. Analysis → Run (calls adata_read_input_file → ddanalysis → renders results)
5. View: pan/zoom, original+deformed overlay, statusbar info
```

### 1.1 Project scaffolding
- [ ] Create `src/platform/tauri/` directory
- [ ] Initialize Tauri v2 project with React + TypeScript frontend
- [ ] Configure `tauri.conf.json`: window title "ntdda", size 1024x768
- [ ] Set up `src-tauri/Cargo.toml`: tauri, serde, serde_json, cc (build dep)
- [ ] Ensure `cargo build` and `npm run dev` work with empty app

### 1.2 Build system (build.rs)
- [ ] Compile all 52 libdda C sources + `platform/stubs/stubs.c` via `cc` crate
- [ ] Set include path to `ntdda/include/`
- [ ] Set `-std=c99`, define `LIBXML_STATIC`
- [ ] Link libxml2 via pkg-config
- [ ] Build a small C probe program that prints `sizeof()` and `offsetof()`
      for Geometrydata, Analysisdata, DDA — save output for Rust assertions
- [ ] Verify `cargo build` produces working binary

### 1.3 Rust FFI layer (ffi.rs)
- [ ] `#[repr(C)]` struct `Geometrydata` — mirror fields up through what we need:
      `nBlocks`, `nJoints`, `nBolts`, `nFPoints`, `nLPoints`, `nMPoints`,
      `nHPoints`, `scale[4]`, `points`/`pointsize1`/`pointsize2`,
      `vertices`/`vertexsize1`/`vertexsize2`, `vindex`/`vindexsize1`,
      `rockbolts`/`rockboltsize1`/`rockboltsize2`,
      `joints`/`jointsize1`/`jointsize2`
- [ ] `#[repr(C)]` struct `Analysisdata` — mirror fields for:
      `isRunning`, `nTimeSteps`, `cts`, `elapsedTime`, `delta_t`,
      `nCurrentContacts`, `OCConverged`, `m9`
- [ ] `#[repr(C)]` struct `DDA` — examine `dda.c` to determine layout, or
      treat as opaque and only use via function API
- [ ] `#[repr(C)]` struct `Filepaths` — mirror `gfile[256]`, `afile[256]`, etc.
- [ ] Extern function declarations:
  ```rust
  extern "C" {
      fn dda_new() -> *mut DDA;
      fn dda_delete(dda: *mut DDA);
      fn dda_run(dda: *mut DDA) -> c_int;
      fn dda_get_geometrydata(dda: *mut DDA) -> *mut Geometrydata;
      fn dda_set_geometrydata(dda: *mut DDA, gd: *mut Geometrydata);
      fn dda_get_analysisdata(dda: *mut DDA) -> *mut Analysisdata;
      fn dda_set_analysisdata(dda: *mut DDA, ad: *mut Analysisdata);
      fn gdata_new() -> *mut Geometrydata;
      fn gdata_delete(gd: *mut Geometrydata);
      fn gdata_read_input_file(gd: *mut Geometrydata, filename: *const c_char);
      fn gdata_get_number_of_blocks(gd: *mut Geometrydata) -> c_int;
      fn adata_new() -> *mut Analysisdata;
      fn adata_delete(ad: *mut Analysisdata);
      fn adata_read_input_file(ad: *mut Analysisdata, ...);
      fn ddacut(dda: *mut DDA, ...) -> c_int;  // check signature
      fn ddanalysis(dda: *mut DDA, fp: *mut Filepaths) -> c_int;
      fn initFilePaths(fp: *mut Filepaths);
  }
  ```
- [ ] Size/offset assertions against values from build.rs probe

### 1.4 Rust bridge layer (bridge.rs)
- [ ] Safe `DdaEngine` struct wrapping `*mut DDA`, implements `Drop`
- [ ] `load_geometry(&self, path: &str) -> Result<()>`
- [ ] `apply_geometry(&self) -> Result<SceneData>` (calls ddacut, reads results)
- [ ] `load_analysis(&self, path: &str) -> Result<()>`
- [ ] `run_analysis(&self) -> Result<AnalysisState>` (calls ddanalysis)
- [ ] `get_scene(&self) -> SceneData` — reads Geometrydata fields:
  - Iterate blocks 0..nBlocks, for each read vindex[block] start/end,
    then read vertices[start..end] as (x,y) pairs
  - Read points array for fixed/load/measured/hole points
  - Read rockbolts array for bolt endpoints
  - Read joints array for joint lines
  - Read scale[4] for bounding box
- [ ] Serde structs:
  ```rust
  struct SceneData { blocks, bolts, joints, points, bbox }
  struct Block { id, vertices: Vec<[f64;2]>, material_id }
  struct PointData { x, y, point_type }  // 0=fixed,1=measured,2=load,3=hole
  struct BoltData { x1, y1, x2, y2 }
  struct JointData { x1, y1, x2, y2, joint_type }
  struct AnalysisState { time_step, elapsed_time, num_contacts, converged }
  ```

### 1.5 Tauri commands (commands.rs)
- [ ] `AppState` with `Mutex<DdaEngine>` and `AppPhase` enum
- [ ] `AppPhase`: `Empty → GeometryLoaded → GeometryCut → AnalysisLoaded → Running → Finished`
- [ ] Commands (each returns `Result<Response, String>`):
  - `open_geometry` — file dialog, load, return phase
  - `apply_geometry` — ddacut, return SceneData + phase
  - `open_analysis` — file dialog, load, return phase
  - `run_analysis` — ddanalysis, return SceneData + AnalysisState + phase
  - `get_scene` — return current SceneData
- [ ] Guard functions: `require_phase(current, minimum)` — return error if not ready

### 1.6 Frontend: React app structure
- [ ] `App.tsx` — layout: toolbar top, canvas center, statusbar bottom
- [ ] `useAppState` hook — tracks phase, scene data, analysis state
- [ ] `commands.ts` — typed wrappers around `invoke()`

### 1.7 Frontend: canvas renderer (`Viewport.tsx`)
- [ ] HTML Canvas element, fills available space
- [ ] `useCanvas` hook managing context, resize, DPI scaling
- [ ] World-to-screen transform:
  - Fit scene bbox into canvas with margin
  - Y-axis flip (math coords → screen coords)
  - Pan offset (mouse drag or arrow keys)
  - Zoom factor (mouse wheel or +/- keys)
- [ ] `renderScene(ctx, scene, transform)`:
  - Clear canvas
  - Draw blocks: filled polygons, 6 material colors
    (Win32 uses: white, light gray, light blue, light green, light yellow, light pink)
  - Draw block edges: colored by joint type, 10 colors
    (Win32: white, black, red, blue, green, purple, yellow, brown, olive, gray)
  - Draw points: fixed=green triangle, load=black triangle,
    measured=white circle, hole=black circle
  - Draw bolts: black lines (original), red lines (deformed)
- [ ] `renderOverlay(ctx, origScene, deformedScene, transform)`:
  - Original geometry in dashed lines / transparent fill
  - Deformed geometry in solid lines / opaque fill

### 1.8 Frontend: toolbar and statusbar
- [ ] `Toolbar.tsx`:
  - Buttons: Open Geometry, Apply, Open Analysis, Run
  - Each button disabled unless correct phase
  - Phase-to-button-enable mapping:
    | Phase | Open Geo | Apply | Open Ana | Run |
    |---|---|---|---|---|
    | Empty | yes | no | no | no |
    | GeometryLoaded | yes | yes | no | no |
    | GeometryCut | yes | yes | yes | no |
    | AnalysisLoaded | yes | yes | yes | yes |
    | Running | no | no | no | no |
    | Finished | yes | yes | yes | yes |
- [ ] `StatusBar.tsx`:
  - Left: phase label ("Ready", "Geometry loaded", "Analysis complete", etc.)
  - Center: block count (when geometry loaded)
  - Right: cursor coordinates (from canvas mouse move)
  - During analysis: timestep, elapsed time, contacts

### 1.9 Frontend: results view
- [ ] After analysis completes, re-fetch scene with deformed positions
- [ ] Toggle button: "Show Original" — overlays original geometry
- [ ] Analysis summary panel or toast: timesteps completed, elapsed time

### 1.10 Integration testing
- [ ] Find .geo and .ana sample files in the repo or test data
- [ ] Manual test: open → apply → verify blocks render
- [ ] Manual test: open geo + ana → run → verify deformed overlay
- [ ] Automated: Rust unit tests for FFI struct size assertions
- [ ] Automated: Rust unit tests for `load_geometry` → verify block count

---

## Stage 2: Feature Development

Stage 1 complete. Stage 2 is broken into independent parts (2a–2d), developed
in priority order chosen by user.

### 2a: Geometry drawing editor (deferred)
- [ ] Tool palette: select, joint (8 types), fixed/measured/load/hole point, bolt
- [ ] Click-to-place on canvas, grid snap
- [ ] Undo (remove last element)
- [ ] Save/load .geo files
- NOTE: No C editing API exists. Must build entirely in Rust/TypeScript
  using in-memory lists (matching Win32 drawdialog.c approach), then
  serialize to .geo format.

### 2b: Analysis parameter editor (deferred)
- [ ] Flags, timestep, joint materials, block materials, load points,
      contact, bolts, gravity, seismic
- [ ] Save/load .ana files

### 2c: Enhanced visualization + replay

#### 2c-1: Replay animation — COMPLETE
- [x] Parse `.replay` file (header + per-timestep vertex/bolt/point data)
- [x] Rust: `replay.rs` parser, Tauri commands (load_replay, get_replay_frame, get_replay_info)
- [x] TS: `ReplayControls.tsx` — play/pause/stop, frame slider, speed (1x/2x/5x/10x)
- [x] Integrate into App.tsx — "Replay" button enabled after Finished phase
- [x] Show Original overlay available during replay

#### 2c-2: Visualization overlays
- [ ] Parse MATLAB output files (`*_meas.m`, `*_moments.m`, `*_stress.m`)
- [ ] Centroid trajectories (purple polylines across timesteps)
- [ ] Principal stress directions (cross-hairs at block centroids)
- [ ] Measured point tracking (displacement trails)
- [ ] Toggle checkboxes in toolbar

### 2d: Cursor interaction (deferred)
- [ ] Block-under-cursor hit testing, coordinate display, click-to-select

### 2e: Export (deferred)
- [ ] SVG/PNG screenshot, DXF geometry export, CSV results export

### 2f: Polish (deferred)
- [ ] macOS menu bar, keyboard shortcuts, drag-drop, recent files

---

## Reference: Win32 menu structure (from Dda.rc)

```
File
  About
  Export → EMF
  Import → DXF
  Exit

View
  Tool Bar (toggle)
  Status Bar (toggle)
  Enable Pop-up (toggle)
  Tool Tips (toggle)

Geometry
  New...              → drawing dialog
  Open...             → file browse .geo
  View/Edit (Dialog)  → geometry edit dialog
  View/Edit (Notepad) → external editor
  Draw GL             → OpenGL drawing
  Apply               → ddacut
  Cancel

Analysis
  New...              → analysis dialog
  Open...             → file browse .ana
  View/Edit (Dialog)  → analysis param dialog
  View/Edit (Notepad) → external editor
  Run                 → ddanalysis
  Pause
  Abort

Results
  Replay
  View Graphics
  View Text
  Print Graphics

Options
  Original geometry (toggle)
  Spy plot, Flag states, Timing, Parameters,
  Fixed points, Block areas, Measured points

Help
  Overview, Index, Tutorial, Search,
  Geometry, Analysis, Post processing, About DDA
```

## Reference: Win32 rendering (wingraph.c)

- Blocks: filled polygons via `Polygon()`, max 35 vertices, 6 material brushes
- Joints: colored lines via `MoveToEx/LineTo`, 10 pen colors by joint type
- Points: triangles (fixed/load) and circles (measured/hole) via `Polygon/Ellipse`
- Bolts: `MoveToEx/LineTo`, black=original, red=deformed
- Centroids: connected path across timesteps
- Stresses: line pairs for major/minor principal directions
- Scale: `MM_ISOTROPIC` mapping mode, computed from geometry bbox + zoom

## Reference: Win32 state machine

```
READY_STATE
  → load .geo → GEOM_STATE
  → apply     → GEOM_STATE | FINISHED
  → load .ana → ANA_STATE | READY_STATE
  → run       → ANA_STATE | RUNNING
  → complete  → ANA_STATE | FINISHED
  → abort     → ANA_STATE | ABORTED  (or GEOM_STATE | ABORTED)
```

---

## Cleanup notes

- `include/.!82497!Dda.rc` — orphaned Visual Studio crash recovery file.
  Safe to delete. Add `.!*!*` pattern to `.gitignore`.
