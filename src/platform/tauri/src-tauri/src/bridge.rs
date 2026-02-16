//! Safe bridge between Tauri commands and the C FFI layer.
//!
//! DdaEngine wraps the raw C pointers and provides a safe API
//! for loading files, running analysis, and extracting scene data.

use crate::ffi;
use crate::replay::{self, ReplayData};
use serde::Serialize;
use std::ffi::CString;
use std::path::Path;

/// Scene data extracted from Geometrydata, ready for JSON serialization.
#[derive(Serialize, Clone, Debug)]
pub struct SceneData {
    pub blocks: Vec<Block>,
    pub joints: Vec<JointData>,
    pub points: Vec<PointData>,
    pub bolts: Vec<BoltData>,
    pub bbox: [f64; 4], // [min_x, max_x, min_y, max_y]
}

#[derive(Serialize, Clone, Debug)]
pub struct Block {
    pub id: usize,
    pub vertices: Vec<[f64; 2]>,
    pub material_id: usize,
}

#[derive(Serialize, Clone, Debug)]
pub struct JointData {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
    pub joint_type: i32,
}

#[derive(Serialize, Clone, Debug)]
pub struct PointData {
    pub x: f64,
    pub y: f64,
    pub point_type: i32, // 0=fixed, 1=load, 2=measured, 3=hole
}

#[derive(Serialize, Clone, Debug)]
pub struct BoltData {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
}

#[derive(Serialize, Clone, Debug)]
pub struct AnalysisState {
    pub time_step: i32,
    pub elapsed_time: f64,
    pub num_contacts: i32,
    pub converged: bool,
}

#[derive(Serialize, Clone, Debug)]
pub struct ReplayInfo {
    pub num_frames: usize,
    pub num_blocks: usize,
    pub num_bolts: usize,
}

/// Application phase tracking the DDA workflow state machine.
#[derive(Serialize, Clone, Debug, PartialEq)]
pub enum AppPhase {
    Empty,
    GeometryLoaded,
    GeometryCut,
    AnalysisLoaded,
    Running,
    Finished,
}

/// Safe wrapper around the C DDA engine.
pub struct DdaEngine {
    dda: *mut ffi::DDA,
    filepaths: Box<ffi::Filepaths>,
    phase: AppPhase,
    scene: Option<SceneData>,
    original_scene: Option<SceneData>,
    geo_dir: Option<std::path::PathBuf>,
    replay: Option<ReplayData>,
}

// SAFETY: DDA engine is only accessed from one thread at a time
// (protected by Mutex in Tauri state).
unsafe impl Send for DdaEngine {}

impl DdaEngine {
    pub fn new() -> Self {
        let dda = unsafe { ffi::dda_new() };
        assert!(!dda.is_null(), "dda_new() returned null");

        let mut filepaths: Box<ffi::Filepaths> = Box::new(unsafe { std::mem::zeroed() });
        unsafe {
            ffi::initFilePaths(&mut *filepaths);
        }

        DdaEngine {
            dda,
            filepaths,
            phase: AppPhase::Empty,
            scene: None,
            original_scene: None,
            geo_dir: None,
            replay: None,
        }
    }

    pub fn phase(&self) -> &AppPhase {
        &self.phase
    }

    /// Load a geometry file (.geo or .xml).
    pub fn load_geometry(&mut self, path: &str) -> Result<(), String> {
        eprintln!("[ntdda] load_geometry: {}", path);

        let gd = unsafe { ffi::gdata_new() };
        if gd.is_null() {
            return Err("gdata_new() returned null".into());
        }

        // Set display callbacks before reading (C parser uses these via global fn ptrs)
        unsafe {
            (*gd).display_error = Some(ffi::dda_display_error);
            (*gd).display_warning = Some(ffi::dda_display_warning);
        }

        let c_path = CString::new(path).map_err(|e| e.to_string())?;
        unsafe {
            ffi::gdata_read_input_file(gd, c_path.as_ptr() as *mut _);
            ffi::dda_set_geometrydata(self.dda, gd);
        }

        // Copy path into filepaths.gfile
        copy_path_to_buf(path, &mut self.filepaths.gfile);

        // Store the geo file's directory for ddacut's log files
        self.geo_dir = Path::new(path)
            .parent()
            .map(|p| p.to_path_buf());

        unsafe {
            let gd_ref = &*gd;
            eprintln!(
                "[ntdda] after read: nJoints={}, nBolts={}, nFPoints={}, nLPoints={}, nMPoints={}, nHPoints={}",
                gd_ref.n_joints, gd_ref.n_bolts, gd_ref.n_f_points,
                gd_ref.n_l_points, gd_ref.n_m_points, gd_ref.n_h_points
            );
        }

        self.phase = AppPhase::GeometryLoaded;
        self.scene = None;
        self.original_scene = None;
        Ok(())
    }

    /// Apply geometry (run ddacut) to generate blocks from joints.
    pub fn apply_geometry(&mut self) -> Result<SceneData, String> {
        if self.phase != AppPhase::GeometryLoaded && self.phase != AppPhase::GeometryCut {
            return Err(format!(
                "Cannot apply geometry in phase {:?}",
                self.phase
            ));
        }

        let gd = unsafe { ffi::dda_get_geometrydata(self.dda) };
        if gd.is_null() {
            return Err("No geometry data loaded".into());
        }

        // ddacut() opens "cut.log" and "pnp.log" in the CWD.
        // Change to the geo file's directory so these can be created.
        let prev_dir = std::env::current_dir().ok();
        if let Some(ref dir) = self.geo_dir {
            eprintln!("[ntdda] chdir to {:?} for ddacut", dir);
            let _ = std::env::set_current_dir(dir);
        }

        eprintln!("[ntdda] calling ddacut...");
        unsafe {
            ffi::ddacut(gd);
        }
        eprintln!("[ntdda] ddacut returned");

        // Fix point-to-block assignments.  The C crr() function uses
        // srand(2)/rand() with `e1 = rand()/2048.0`, which assumes
        // Win32's RAND_MAX of 32767.  On macOS RAND_MAX is 2^31-1,
        // making e1 >> 1, so sqrt(1 - e1²) = NaN and every point is
        // classified "outside all blocks".  We redo the assignment in
        // Rust with a deterministic ray-cast that doesn't use rand().
        unsafe {
            fix_point_block_assignments(&*gd);
        }

        // Restore previous directory
        if let Some(dir) = prev_dir {
            let _ = std::env::set_current_dir(dir);
        }

        let scene = self.extract_scene()?;
        eprintln!(
            "[ntdda] scene: {} blocks, {} joints, {} points, {} bolts, bbox={:?}",
            scene.blocks.len(),
            scene.joints.len(),
            scene.points.len(),
            scene.bolts.len(),
            scene.bbox
        );
        if !scene.blocks.is_empty() {
            let b = &scene.blocks[0];
            eprintln!(
                "[ntdda] block[0]: id={}, material={}, {} vertices, first={:?}",
                b.id, b.material_id, b.vertices.len(),
                b.vertices.first()
            );
        }

        self.scene = Some(scene.clone());
        self.original_scene = Some(scene.clone());
        self.phase = AppPhase::GeometryCut;
        Ok(scene)
    }

    /// Load an analysis file (.ana or .xml).
    pub fn load_analysis(&mut self, path: &str) -> Result<(), String> {
        eprintln!("[ntdda] load_analysis: {}", path);

        if self.phase != AppPhase::GeometryCut
            && self.phase != AppPhase::AnalysisLoaded
            && self.phase != AppPhase::Finished
        {
            return Err(format!(
                "Cannot load analysis in phase {:?}",
                self.phase
            ));
        }

        let gd = unsafe { ffi::dda_get_geometrydata(self.dda) };
        if gd.is_null() {
            return Err("No geometry data available".into());
        }

        // Read counts from geometry for adata_read_input_file
        let (n_f_points, point_count, n_l_points) = unsafe {
            ((*gd).n_f_points, (*gd).point_count, (*gd).n_l_points)
        };
        eprintln!(
            "[ntdda] geometry counts: n_f_points={}, point_count={}, n_l_points={}",
            n_f_points, point_count, n_l_points
        );

        let ad = unsafe { ffi::adata_new() };
        if ad.is_null() {
            return Err("adata_new() returned null".into());
        }
        eprintln!("[ntdda] adata_new() ok, setting display callbacks...");

        // Set display callbacks before reading
        unsafe {
            (*ad).display_error = Some(ffi::dda_display_error);
            (*ad).display_warning = Some(ffi::dda_display_warning);
        }
        eprintln!("[ntdda] display callbacks set, calling adata_read_input_file...");

        let c_path = CString::new(path).map_err(|e| e.to_string())?;
        unsafe {
            ffi::adata_read_input_file(ad, c_path.as_ptr() as *mut _, n_f_points, point_count, n_l_points);
        }
        eprintln!("[ntdda] adata_read_input_file returned, setting on DDA...");

        unsafe {
            ffi::dda_set_analysisdata(self.dda, ad);
        }

        // Copy path into filepaths.afile
        copy_path_to_buf(path, &mut self.filepaths.afile);

        self.phase = AppPhase::AnalysisLoaded;
        eprintln!("[ntdda] analysis loaded successfully");
        Ok(())
    }

    /// Run the DDA analysis.
    pub fn run_analysis(&mut self) -> Result<(SceneData, AnalysisState), String> {
        if self.phase != AppPhase::AnalysisLoaded && self.phase != AppPhase::Finished {
            return Err(format!(
                "Cannot run analysis in phase {:?}",
                self.phase
            ));
        }

        self.phase = AppPhase::Running;

        // Set rootname from the analysis file (stem without extension).
        // openAnalysisFiles() in ddafile.c uses rootname to construct output paths.
        let afile = c_buf_to_string(&self.filepaths.afile);
        if let Some(stem) = Path::new(&afile).file_stem().and_then(|s| s.to_str()) {
            copy_path_to_buf(stem, &mut self.filepaths.rootname);
            eprintln!("[ntdda] rootname set to: {}", stem);
        }

        // Change CWD to geo directory — ddanalysis opens output files relative to CWD.
        // openAnalysisFiles() calls getcwd() and dda_set_output_directory("output").
        let prev_dir = std::env::current_dir().ok();
        if let Some(ref dir) = self.geo_dir {
            eprintln!("[ntdda] chdir to {:?} for ddanalysis", dir);
            let _ = std::env::set_current_dir(dir);
        }

        // Pre-flight checks: verify GData and AData are non-null
        let gd_check = unsafe { ffi::dda_get_geometrydata(self.dda) };
        let ad_check = unsafe { ffi::dda_get_analysisdata(self.dda) };
        if gd_check.is_null() {
            return Err("GData is null — geometry not loaded or lost".into());
        }
        if ad_check.is_null() {
            return Err("AData is null — analysis not loaded or lost".into());
        }

        eprintln!("[ntdda] calling ddanalysis...");
        let result = unsafe { ffi::ddanalysis(self.dda, &mut *self.filepaths) };
        eprintln!("[ntdda] ddanalysis returned: {}", result);

        // Restore previous directory
        if let Some(dir) = prev_dir {
            let _ = std::env::set_current_dir(dir);
        }

        // ddanalysis() returns TRUE (1) on success
        if result == 0 {
            return Err("ddanalysis returned 0 (failure)".into());
        }

        let scene = self.extract_scene()?;
        let state = self.extract_analysis_state()?;

        self.scene = Some(scene.clone());
        self.phase = AppPhase::Finished;
        Ok((scene, state))
    }

    /// Get the current scene data.
    pub fn get_scene(&self) -> Option<&SceneData> {
        self.scene.as_ref()
    }

    /// Get the original (pre-analysis) scene for overlay.
    pub fn get_original_scene(&self) -> Option<&SceneData> {
        self.original_scene.as_ref()
    }

    /// Load and parse the replay file written by ddanalysis.
    pub fn load_replay(&mut self) -> Result<ReplayInfo, String> {
        if self.phase != AppPhase::Finished {
            return Err(format!("Cannot load replay in phase {:?}", self.phase));
        }

        // Get replay file path from filepaths struct
        let replay_path_str = c_buf_to_string(&self.filepaths.replayfile);
        eprintln!("[ntdda] loading replay from: {}", replay_path_str);

        let replay_path = Path::new(&replay_path_str);
        if !replay_path.exists() {
            return Err(format!("Replay file not found: {}", replay_path_str));
        }

        let data = replay::parse_replay_file(replay_path)?;
        let info = ReplayInfo {
            num_frames: data.frames.len(),
            num_blocks: data.num_blocks,
            num_bolts: data.num_bolts,
        };
        self.replay = Some(data);
        Ok(info)
    }

    /// Get scene data for a specific replay frame.
    pub fn get_replay_frame(&self, index: usize) -> Result<SceneData, String> {
        let replay = self.replay.as_ref().ok_or("No replay data loaded")?;
        let bbox = self
            .original_scene
            .as_ref()
            .map(|s| s.bbox)
            .unwrap_or([0.0, 1.0, 0.0, 1.0]);
        replay
            .frame_to_scene(index, bbox)
            .ok_or_else(|| format!("Frame {} out of range (have {})", index, replay.frames.len()))
    }

    /// Explicit cleanup: free the C DDA engine and null the pointer.
    pub fn shutdown(&mut self) {
        if !self.dda.is_null() {
            eprintln!("[ntdda] shutdown: calling dda_delete");
            unsafe {
                ffi::dda_delete(self.dda);
            }
            self.dda = std::ptr::null_mut();
        }
        self.replay = None;
        self.scene = None;
        self.original_scene = None;
    }

    /// Get replay info without full frame data.
    pub fn get_replay_info(&self) -> Result<ReplayInfo, String> {
        let replay = self.replay.as_ref().ok_or("No replay data loaded")?;
        Ok(ReplayInfo {
            num_frames: replay.frames.len(),
            num_blocks: replay.num_blocks,
            num_bolts: replay.num_bolts,
        })
    }

    /// Extract scene data from the current Geometrydata.
    fn extract_scene(&self) -> Result<SceneData, String> {
        let gd = unsafe { ffi::dda_get_geometrydata(self.dda) };
        if gd.is_null() {
            return Err("No geometry data".into());
        }

        unsafe {
            let gd = &*gd;
            eprintln!(
                "[ntdda] extract_scene: nBlocks={}, vertexCount={}, nJoints={}, nPoints={}, nBolts={}",
                gd.n_blocks, gd.vertex_count, gd.n_joints, gd.n_points, gd.n_bolts
            );
            eprintln!(
                "[ntdda] scale={:?}, w0={}, vindex null={}, vertices null={}",
                gd.scale, gd.w0, gd.vindex.is_null(), gd.vertices.is_null()
            );

            let blocks = self.extract_blocks(gd);
            let joints = self.extract_joints(gd);
            let points = self.extract_points(gd);
            let bolts = self.extract_bolts(gd);
            let mut bbox = gd.scale;

            // Fallback: if scale is all zeros, compute bbox from block vertices
            if bbox[0] == 0.0 && bbox[1] == 0.0 && bbox[2] == 0.0 && bbox[3] == 0.0 {
                eprintln!("[ntdda] WARNING: scale is all zeros, computing bbox from vertices");
                if let Some(computed) = compute_bbox_from_blocks(&blocks) {
                    bbox = computed;
                    eprintln!("[ntdda] computed bbox={:?}", bbox);
                }
            }

            Ok(SceneData {
                blocks,
                joints,
                points,
                bolts,
                bbox,
            })
        }
    }

    unsafe fn extract_blocks(&self, gd: &ffi::Geometrydata) -> Vec<Block> {
        let n_blocks = gd.n_blocks as usize;
        let mut blocks = Vec::with_capacity(n_blocks);

        if gd.vindex.is_null() || gd.vertices.is_null() {
            return blocks;
        }

        for i in 1..=n_blocks {
            let vindex_row = *gd.vindex.add(i);
            if vindex_row.is_null() {
                continue;
            }
            // vindex[block][1] = start vertex, vindex[block][2] = end vertex
            let start = *vindex_row.add(1) as usize;
            let end = *vindex_row.add(2) as usize;

            let mut vertices = Vec::new();
            for j in start..=end {
                let vertex_row = *gd.vertices.add(j);
                if vertex_row.is_null() {
                    continue;
                }
                let x = *vertex_row.add(1);
                let y = *vertex_row.add(2);
                vertices.push([x, y]);
            }

            // Material ID from vindex[block][0] (if available), else 0
            let material_id = *vindex_row.add(0) as usize;

            blocks.push(Block {
                id: i,
                vertices,
                material_id,
            });
        }

        blocks
    }

    unsafe fn extract_joints(&self, gd: &ffi::Geometrydata) -> Vec<JointData> {
        let n_joints = gd.n_joints as usize;
        let mut joints = Vec::with_capacity(n_joints);

        if gd.joints.is_null() {
            return joints;
        }

        for i in 1..=n_joints {
            let row = *gd.joints.add(i);
            if row.is_null() {
                continue;
            }
            joints.push(JointData {
                x1: *row.add(1),
                y1: *row.add(2),
                x2: *row.add(3),
                y2: *row.add(4),
                joint_type: *row.add(5) as i32,
            });
        }

        joints
    }

    unsafe fn extract_points(&self, gd: &ffi::Geometrydata) -> Vec<PointData> {
        let n_points = gd.n_points as usize;
        let mut points = Vec::with_capacity(n_points);

        if gd.points.is_null() {
            return points;
        }

        for i in 1..=n_points {
            let row = *gd.points.add(i);
            if row.is_null() {
                continue;
            }
            points.push(PointData {
                x: *row.add(1),
                y: *row.add(2),
                point_type: *row.add(3) as i32,
            });
        }

        points
    }

    unsafe fn extract_bolts(&self, gd: &ffi::Geometrydata) -> Vec<BoltData> {
        let n_bolts = gd.n_bolts as usize;
        let mut bolts = Vec::with_capacity(n_bolts);

        if gd.rockbolts.is_null() {
            return bolts;
        }

        for i in 1..=n_bolts {
            let row = *gd.rockbolts.add(i);
            if row.is_null() {
                continue;
            }
            bolts.push(BoltData {
                x1: *row.add(1),
                y1: *row.add(2),
                x2: *row.add(3),
                y2: *row.add(4),
            });
        }

        bolts
    }

    fn extract_analysis_state(&self) -> Result<AnalysisState, String> {
        let ad = unsafe { ffi::dda_get_analysisdata(self.dda) };
        if ad.is_null() {
            return Err("No analysis data".into());
        }

        unsafe {
            let ad = &*ad;
            Ok(AnalysisState {
                time_step: ad.cts,
                elapsed_time: ad.elapsed_time,
                num_contacts: ad.n_current_contacts,
                converged: ad.oc_converged != 0,
            })
        }
    }
}

impl Drop for DdaEngine {
    fn drop(&mut self) {
        if !self.dda.is_null() {
            unsafe {
                ffi::dda_delete(self.dda);
            }
        }
    }
}

/// Fix point-to-block assignments after ddacut().
///
/// The C `crr()` point-in-polygon function uses `e1 = rand()/2048.0` to
/// pick a ray direction, assuming Win32's RAND_MAX of 32767.  On macOS
/// RAND_MAX is 2^31-1, so `e1` is ~16 instead of ~0.02, making
/// `sqrt(1 - e1²)` produce NaN.  NaN comparisons always return false,
/// so every edge counts as an intersection and every point is classified
/// as "outside all blocks" (points[i][3] = 0).
///
/// This function redoes the assignment using a standard ray-casting
/// algorithm with a fixed horizontal ray (no randomness needed).
pub(crate) unsafe fn fix_point_block_assignments(gd: &ffi::Geometrydata) {
    let n_points = (gd.n_f_points + gd.n_l_points + gd.n_m_points) as usize;
    let n_blocks = gd.n_blocks as usize;

    if n_points == 0 || n_blocks == 0 || gd.points.is_null() || gd.vertices.is_null() || gd.vindex.is_null() {
        return;
    }

    let mut fixed_count = 0;

    for i in 1..=n_points {
        let pt_row = *gd.points.add(i);
        if pt_row.is_null() {
            continue;
        }
        let px = *pt_row.add(1);
        let py = *pt_row.add(2);
        let current_block = *pt_row.add(3) as i32;

        // Only fix points that weren't assigned (block = 0)
        if current_block != 0 {
            continue;
        }

        for block in 1..=n_blocks {
            let vi_row = *gd.vindex.add(block);
            if vi_row.is_null() {
                continue;
            }
            let j0 = *vi_row.add(1) as usize; // start vertex index
            let j1 = *vi_row.add(2) as usize; // end vertex index

            if point_in_polygon(gd, px, py, j0, j1) {
                *pt_row.add(3) = block as f64;
                fixed_count += 1;
                break;
            }
        }
    }

    if fixed_count > 0 {
        eprintln!(
            "[ntdda] fix_point_block_assignments: reassigned {} of {} points",
            fixed_count, n_points
        );
    }
}

/// Ray-casting point-in-polygon test using a horizontal ray (positive x).
///
/// Vertices are stored in gd.vertices[j0..=j1] (1-based, Fortran style).
/// The polygon is closed: edge from vertex j1 back to vertex j0.
/// Returns true if the point (px, py) is inside the polygon.
unsafe fn point_in_polygon(
    gd: &ffi::Geometrydata,
    px: f64,
    py: f64,
    j0: usize,
    j1: usize,
) -> bool {
    let mut crossings = 0;
    let n_verts = j1 - j0 + 1;

    for k in 0..n_verts {
        let i_curr = j0 + k;
        // Next vertex wraps: after j1 comes j0
        let i_next = if k + 1 < n_verts { j0 + k + 1 } else { j0 };

        let v_curr = *gd.vertices.add(i_curr);
        let v_next = *gd.vertices.add(i_next);
        if v_curr.is_null() || v_next.is_null() {
            continue;
        }

        let y1 = *v_curr.add(2);
        let y2 = *v_next.add(2);

        // Edge must straddle the horizontal line y = py
        if (y1 <= py && y2 > py) || (y2 <= py && y1 > py) {
            // Compute x-coordinate of intersection: linear interpolation
            let x1 = *v_curr.add(1);
            let x2 = *v_next.add(1);
            let t = (py - y1) / (y2 - y1);
            let x_intersect = x1 + t * (x2 - x1);

            if px < x_intersect {
                crossings += 1;
            }
        }
    }

    // Odd number of crossings = inside
    crossings % 2 == 1
}

/// Compute bounding box from block vertex data as a fallback.
fn compute_bbox_from_blocks(blocks: &[Block]) -> Option<[f64; 4]> {
    let mut iter = blocks.iter().flat_map(|b| b.vertices.iter());
    let first = iter.next()?;
    let mut min_x = first[0];
    let mut max_x = first[0];
    let mut min_y = first[1];
    let mut max_y = first[1];
    for v in iter {
        if v[0] < min_x { min_x = v[0]; }
        if v[0] > max_x { max_x = v[0]; }
        if v[1] < min_y { min_y = v[1]; }
        if v[1] > max_y { max_y = v[1]; }
    }
    Some([min_x, max_x, min_y, max_y])
}

/// Copy a Rust string path into a C char buffer.
fn copy_path_to_buf(path: &str, buf: &mut [std::os::raw::c_char; ffi::FNAME_BUFSIZE]) {
    let bytes = path.as_bytes();
    let len = bytes.len().min(ffi::FNAME_BUFSIZE - 1);
    for i in 0..len {
        buf[i] = bytes[i] as std::os::raw::c_char;
    }
    buf[len] = 0;
}

/// Read a C char buffer into a Rust String (up to first NUL).
fn c_buf_to_string(buf: &[std::os::raw::c_char; ffi::FNAME_BUFSIZE]) -> String {
    let bytes: Vec<u8> = buf
        .iter()
        .take_while(|&&c| c != 0)
        .map(|&c| c as u8)
        .collect();
    String::from_utf8_lossy(&bytes).into_owned()
}
