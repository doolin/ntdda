//! FFI bindings to libdda.
//!
//! Mirrors C struct layouts via #[repr(C)] for direct field access.
//! Validated against the C compiler's sizeof/offsetof via probe.c.

use std::os::raw::{c_char, c_double, c_int, c_uint, c_void};

// Opaque types — we only hold pointers, never inspect internals
pub enum DDA {}
pub enum DList {}
pub enum Loadpoint {}
pub enum TimeHistory {}
pub enum Constants {}
pub enum Gravity {}

// Function pointer types matching C typedefs
pub type DisplayFunc = Option<unsafe extern "C" fn(*const c_char)>;

/// Mirrors `struct _geo_data_tag` from geometrydata.h.
/// Field order and types must match the C layout exactly.
#[repr(C)]
pub struct Geometrydata {
    pub display_warning: DisplayFunc,
    pub display_error: DisplayFunc,

    pub index: c_int,
    pub blocksize: c_int,

    pub e00: c_double,
    pub n_joints: c_int,
    pub n_boundary_lines: c_int,
    pub n_mat_lines: c_int,
    pub n_bolts: c_int,
    pub n_f_points: c_int,
    pub max_fixed_points_per_fixed_line: c_int,
    pub n_l_points: c_int,
    pub n_m_points: c_int,
    pub n_h_points: c_int,
    pub n_s_points: c_int,
    pub n_points: c_int,
    pub n_pore_pres: c_int,
    pub n_wtable: c_int,
    pub n_intersection_points: c_int,
    pub n_contacts: c_int,
    pub point_count: c_int,
    pub n_blocks: c_int,
    pub vertex_count: c_int,
    pub n2: c_int,
    pub n3: c_int,

    pub scale: [c_double; 4],
    pub w0: c_double,

    pub pointsize1: c_int,
    pub pointsize2: c_int,
    pub points: *mut *mut c_double,
    pub prevpoints: *mut *mut c_double,
    pub origpoints: *mut *mut c_double,

    pub fpoints: *mut DList,
    pub lpoints: *mut DList,
    pub mpoints: *mut DList,
    pub hpoints: *mut DList,
    pub dirpoints: *mut DList,
    pub dispoints: *mut DList,
    pub seispoints: *mut DList,

    pub jointsize1: c_int,
    pub jointsize2: c_int,
    pub joints: *mut *mut c_double,

    pub matlinesize1: c_int,
    pub matlinesize2: c_int,
    pub matlines: *mut *mut c_double,

    pub rockboltsize1: c_int,
    pub rockboltsize2: c_int,
    pub rockbolts: *mut *mut c_double,
    pub origbolts: *mut *mut c_double,
    pub max_segments_per_bolt: c_int,

    pub vertexsize1: c_int,
    pub vertexsize2: c_int,
    pub vertices: *mut *mut c_double,
    pub origvertices: *mut *mut c_double,

    pub vindexsize1: c_int,
    pub vindexsize2: c_int,
    pub vindex: *mut *mut c_int,

    pub nn0size: c_int,
    pub nn0: *mut c_int,

    pub porepressize1: c_int,
    pub porepressize2: c_int,
    pub porepres: *mut *mut c_double,

    pub wtablesize1: c_int,
    pub wtablesize2: c_int,
    pub watertable: *mut *mut c_double,

    pub momentsize1: c_int,
    pub momentsize2: c_int,
    pub moments: *mut *mut c_double,

    pub origareasize: c_int,
    pub origarea: *mut c_double,

    pub origdensitysize: c_int,
    pub origdensity: *mut c_double,

    // Function pointers at the end of the struct
    pub compute_bbox: Option<unsafe extern "C" fn(*mut Geometrydata)>,
    pub deleteblock: Option<unsafe extern "C" fn(*mut Geometrydata, c_int)>,
    pub deletepoint: Option<unsafe extern "C" fn(*mut Geometrydata, c_int)>,
    pub dumptofile: Option<
        unsafe extern "C" fn(
            *mut Geometrydata,
            Option<unsafe extern "C" fn(*mut c_void, *const c_char, ...) -> c_int>,
            *mut c_void,
        ),
    >,
}

/// Mirrors `struct _analysisdata_tag` from analysisdata.h.
/// Field order and types must match the C layout exactly.
#[repr(C)]
pub struct Analysisdata {
    pub options: c_uint,

    pub display_warning: DisplayFunc,
    pub display_error: DisplayFunc,

    pub is_running: c_int,
    pub is_paused: c_int,

    pub index: c_int,
    pub blocksize: c_int,

    pub rotationflag: c_int, // ddaboolean = int
    pub gravityflag: c_int,
    pub autotimestepflag: c_int,
    pub autopenaltyflag: c_int,
    pub planestrainflag: c_int,
    pub verticesflag: c_int,

    pub writemfile: c_int,
    pub analysistype: c_int,
    pub pfactor: c_double,

    // enums — C enums are int-sized
    pub rotationtype: c_int,
    pub solvetype: c_int,
    pub contactmethod: c_int,
    pub frictionlaw: c_int,
    pub units: c_int,
    pub fileformat: c_int,

    pub k5size1: c_int,
    pub k5size2: c_int,
    pub tindex: *mut *mut c_int,

    pub loadpoints: *mut Loadpoint,
    pub timehistory: *mut TimeHistory,

    pub mu: c_double,

    pub point_count: c_int,
    pub n_l_points: c_int,

    pub csize1: c_int,
    pub csize2: c_int,
    pub c: *mut *mut c_double,

    pub timedepsize1: c_int,
    pub timedepsize2: c_int,
    pub n_td_points: c_int,
    pub time_deps: *mut *mut c_double,

    pub materialpropsize1: c_int,
    pub materialpropsize2: c_int,
    pub material_props: *mut *mut c_double,

    pub phicohesionsize1: c_int,
    pub phicohesionsize2: c_int,
    pub phi_cohesion: *mut *mut c_double,

    pub boltmatsize1: c_int,
    pub boltmatsize2: c_int,
    pub boltmats: *mut *mut c_double,

    pub globaltimesize1: c_int,
    pub globaltimesize2: c_int,
    pub global_time: *mut *mut c_double,

    pub gravaccel: c_double,
    pub gravity: *mut Gravity,

    pub ksize1: c_int,
    pub ksize2: c_int,
    pub k: *mut *mut c_double,
    pub kcopy: *mut *mut c_double,

    pub fsize1: c_int,
    pub fsize2: c_int,
    pub f: *mut *mut c_double,
    pub fcopy: *mut *mut c_double,

    pub abortedtimestep: c_int,
    pub n_time_steps: c_int,
    pub n_block_mats: c_int,
    pub n_joint_mats: c_int,
    pub n_bolt_mats: c_int,

    pub contactpenalty: c_double,
    pub g0initial: c_double,
    pub contact_damping: c_double,

    pub springstiffsize: c_int,
    pub springstiffness: *mut c_double,

    pub maxtimestep: c_double,
    pub max_delta_t: c_double,
    pub min_delta_t: c_double,
    pub maxdisplacement: c_double,

    pub f_point_spring: c_double,
    pub joint_normal_spring: c_double,

    pub nt: c_int,
    pub n3: c_int,
    pub w6: c_double,
    pub n_current_contacts: c_int,
    pub n_prev_contacts: c_int,
    pub n_bolt_contacts: c_int,
    pub m9: c_int,
    pub oc_converged: c_int,
    pub oc_count: c_int,
    pub oc_limit: c_int,
    pub n9: c_int,
    pub k00: c_int,
    pub nn00: c_int,

    pub avg_area: *mut c_double,
    pub avgareasize: c_int,

    pub cts: c_int,
    pub elapsed_time: c_double,
    pub ts_save_interval: c_int,
    pub save_time_step: c_int,
    pub extra_iteration: c_int,
    pub delta_t: c_double,

    pub constants: *mut Constants,
    pub initialconstants: *mut Constants,

    // Function pointers at the end
    pub abort_fn: Option<unsafe extern "C" fn(*mut Analysisdata)>,
    pub newanalysis: Option<unsafe extern "C" fn() -> *mut Analysisdata>,
    pub validate: Option<unsafe extern "C" fn()>,
    pub deletematerial: Option<unsafe extern "C" fn(*mut Analysisdata, c_int)>,
    pub free_fn: Option<unsafe extern "C" fn(*mut Analysisdata)>,
}

/// Mirrors `struct _filepath_tag` from ddafile.h.
/// Each field is a char[256] buffer.
pub const FNAME_BUFSIZE: usize = 256;

#[repr(C)]
pub struct Filepaths {
    pub gpath: [c_char; FNAME_BUFSIZE],
    pub path: [c_char; FNAME_BUFSIZE],
    pub apath: [c_char; FNAME_BUFSIZE],
    pub vpath: [c_char; FNAME_BUFSIZE],
    pub oldpath: [c_char; FNAME_BUFSIZE],
    pub gfile: [c_char; FNAME_BUFSIZE],
    pub afile: [c_char; FNAME_BUFSIZE],
    pub vfile: [c_char; FNAME_BUFSIZE],
    pub oldfile: [c_char; FNAME_BUFSIZE],
    pub resfile: [c_char; FNAME_BUFSIZE],
    pub replayfile: [c_char; FNAME_BUFSIZE],
    pub logfile: [c_char; FNAME_BUFSIZE],
    pub errorfile: [c_char; FNAME_BUFSIZE],
    pub blockfile: [c_char; FNAME_BUFSIZE],
    pub measfile: [c_char; FNAME_BUFSIZE],
    pub porefile: [c_char; FNAME_BUFSIZE],
    pub timefile: [c_char; FNAME_BUFSIZE],
    pub parfile: [c_char; FNAME_BUFSIZE],
    pub momentfile: [c_char; FNAME_BUFSIZE],
    pub massfile: [c_char; FNAME_BUFSIZE],
    pub gravfile: [c_char; FNAME_BUFSIZE],
    pub htmlfile: [c_char; FNAME_BUFSIZE],
    pub datafile: [c_char; FNAME_BUFSIZE],
    pub gnuplotfile: [c_char; FNAME_BUFSIZE],
    pub spyfile1: [c_char; FNAME_BUFSIZE],
    pub spyfile2: [c_char; FNAME_BUFSIZE],
    pub mfile: [c_char; FNAME_BUFSIZE],
    pub dfile: [c_char; FNAME_BUFSIZE],
    pub eqfile: [c_char; FNAME_BUFSIZE],
    pub fpointfile: [c_char; FNAME_BUFSIZE],
    pub cforce: [c_char; FNAME_BUFSIZE],
    pub fforce: [c_char; FNAME_BUFSIZE],
    pub stressfile: [c_char; FNAME_BUFSIZE],
    pub boltfile: [c_char; FNAME_BUFSIZE],
    pub boltlogfile: [c_char; FNAME_BUFSIZE],
    pub vertexfile: [c_char; FNAME_BUFSIZE],
    pub vertexlogfile: [c_char; FNAME_BUFSIZE],
    pub rootname: [c_char; FNAME_BUFSIZE],
}

// Display callback functions from stubs.c
extern "C" {
    pub fn dda_display_error(message: *const c_char);
    pub fn dda_display_warning(message: *const c_char);
}

// Extern C functions from libdda
extern "C" {
    pub fn dda_new() -> *mut DDA;
    pub fn dda_delete(dda: *mut DDA);
    pub fn dda_get_geometrydata(dda: *mut DDA) -> *mut Geometrydata;
    pub fn dda_set_geometrydata(dda: *mut DDA, gd: *mut Geometrydata);
    pub fn dda_get_analysisdata(dda: *mut DDA) -> *mut Analysisdata;
    pub fn dda_set_analysisdata(dda: *mut DDA, ad: *mut Analysisdata);

    pub fn gdata_new() -> *mut Geometrydata;
    pub fn gdata_read_input_file(gd: *mut Geometrydata, filename: *mut c_char);
    // gdata_delete exists in C but is unsafe to call after ddacut() — it tries
    // to free2DMat on arrays that geometryToReturn() reallocated, and has a
    // known FIXME about memory overwrites. Kept here for reference; do not use
    // until the C-side lifecycle is better understood.
    #[allow(dead_code)]
    pub fn gdata_delete(gd: *mut Geometrydata);

    pub fn ddacut(gd: *mut Geometrydata);

    pub fn adata_new() -> *mut Analysisdata;
    // Same concern as gdata_delete — adata_delete's safety after ddanalysis()
    // has not been verified. Kept for future use.
    #[allow(dead_code)]
    pub fn adata_delete(ad: *mut Analysisdata);
    pub fn adata_read_input_file(
        ad: *mut Analysisdata,
        filename: *mut c_char,
        numfixedpoints: c_int,
        pointcount: c_int,
        numloadpoints: c_int,
    );

    pub fn ddanalysis(dda: *mut DDA, fp: *mut Filepaths) -> c_int;
    pub fn initFilePaths(fp: *mut Filepaths);
}

// Probe functions from probe.c — validate that Rust #[repr(C)] struct mirrors
// match the C compiler's actual layout (sizeof and offsetof for every field).
// Gated behind #[cfg(test)] because these are only called from layout validation
// tests below. Without this gate, rustc warns about 42 unused functions in
// non-test builds. See: https://doc.rust-lang.org/reference/conditional-compilation.html
#[cfg(test)]
extern "C" {
    pub fn probe_sizeof_geometrydata() -> usize;
    pub fn probe_offsetof_gd_display_warning() -> usize;
    pub fn probe_offsetof_gd_display_error() -> usize;
    pub fn probe_offsetof_gd_index() -> usize;
    pub fn probe_offsetof_gd_blocksize() -> usize;
    pub fn probe_offsetof_gd_e00() -> usize;
    pub fn probe_offsetof_gd_nJoints() -> usize;
    pub fn probe_offsetof_gd_nBolts() -> usize;
    pub fn probe_offsetof_gd_nFPoints() -> usize;
    pub fn probe_offsetof_gd_nLPoints() -> usize;
    pub fn probe_offsetof_gd_nMPoints() -> usize;
    pub fn probe_offsetof_gd_nHPoints() -> usize;
    pub fn probe_offsetof_gd_nBlocks() -> usize;
    pub fn probe_offsetof_gd_vertexCount() -> usize;
    pub fn probe_offsetof_gd_scale() -> usize;
    pub fn probe_offsetof_gd_w0() -> usize;
    pub fn probe_offsetof_gd_pointsize1() -> usize;
    pub fn probe_offsetof_gd_pointsize2() -> usize;
    pub fn probe_offsetof_gd_points() -> usize;
    pub fn probe_offsetof_gd_jointsize1() -> usize;
    pub fn probe_offsetof_gd_joints() -> usize;
    pub fn probe_offsetof_gd_rockboltsize1() -> usize;
    pub fn probe_offsetof_gd_rockbolts() -> usize;
    pub fn probe_offsetof_gd_vertexsize1() -> usize;
    pub fn probe_offsetof_gd_vertices() -> usize;
    pub fn probe_offsetof_gd_vindexsize1() -> usize;
    pub fn probe_offsetof_gd_vindex() -> usize;
    pub fn probe_offsetof_gd_pointCount() -> usize;
    pub fn probe_offsetof_gd_nPoints() -> usize;

    pub fn probe_sizeof_analysisdata() -> usize;
    pub fn probe_offsetof_ad_options() -> usize;
    pub fn probe_offsetof_ad_isRunning() -> usize;
    pub fn probe_offsetof_ad_nTimeSteps() -> usize;
    pub fn probe_offsetof_ad_nCurrentContacts() -> usize;
    pub fn probe_offsetof_ad_m9() -> usize;
    pub fn probe_offsetof_ad_OCConverged() -> usize;
    pub fn probe_offsetof_ad_cts() -> usize;
    pub fn probe_offsetof_ad_elapsedTime() -> usize;
    pub fn probe_offsetof_ad_delta_t() -> usize;

    pub fn probe_sizeof_filepaths() -> usize;
    pub fn probe_offsetof_fp_gfile() -> usize;
    pub fn probe_offsetof_fp_afile() -> usize;
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::mem::{offset_of, size_of};

    #[test]
    fn geometrydata_layout_matches_c() {
        unsafe {
            assert_eq!(
                size_of::<Geometrydata>(),
                probe_sizeof_geometrydata(),
                "Geometrydata sizeof mismatch"
            );
            assert_eq!(
                offset_of!(Geometrydata, display_warning),
                probe_offsetof_gd_display_warning(),
                "display_warning offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, display_error),
                probe_offsetof_gd_display_error(),
                "display_error offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, index),
                probe_offsetof_gd_index(),
                "index offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, blocksize),
                probe_offsetof_gd_blocksize(),
                "blocksize offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, e00),
                probe_offsetof_gd_e00(),
                "e00 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_joints),
                probe_offsetof_gd_nJoints(),
                "nJoints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_bolts),
                probe_offsetof_gd_nBolts(),
                "nBolts offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_f_points),
                probe_offsetof_gd_nFPoints(),
                "nFPoints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_l_points),
                probe_offsetof_gd_nLPoints(),
                "nLPoints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_m_points),
                probe_offsetof_gd_nMPoints(),
                "nMPoints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_h_points),
                probe_offsetof_gd_nHPoints(),
                "nHPoints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_blocks),
                probe_offsetof_gd_nBlocks(),
                "nBlocks offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, vertex_count),
                probe_offsetof_gd_vertexCount(),
                "vertexCount offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, scale),
                probe_offsetof_gd_scale(),
                "scale offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, w0),
                probe_offsetof_gd_w0(),
                "w0 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, pointsize1),
                probe_offsetof_gd_pointsize1(),
                "pointsize1 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, pointsize2),
                probe_offsetof_gd_pointsize2(),
                "pointsize2 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, points),
                probe_offsetof_gd_points(),
                "points offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, jointsize1),
                probe_offsetof_gd_jointsize1(),
                "jointsize1 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, joints),
                probe_offsetof_gd_joints(),
                "joints offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, rockboltsize1),
                probe_offsetof_gd_rockboltsize1(),
                "rockboltsize1 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, rockbolts),
                probe_offsetof_gd_rockbolts(),
                "rockbolts offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, vertexsize1),
                probe_offsetof_gd_vertexsize1(),
                "vertexsize1 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, vertices),
                probe_offsetof_gd_vertices(),
                "vertices offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, vindexsize1),
                probe_offsetof_gd_vindexsize1(),
                "vindexsize1 offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, vindex),
                probe_offsetof_gd_vindex(),
                "vindex offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, point_count),
                probe_offsetof_gd_pointCount(),
                "pointCount offset"
            );
            assert_eq!(
                offset_of!(Geometrydata, n_points),
                probe_offsetof_gd_nPoints(),
                "nPoints offset"
            );
        }
    }

    #[test]
    fn analysisdata_layout_matches_c() {
        unsafe {
            assert_eq!(
                size_of::<Analysisdata>(),
                probe_sizeof_analysisdata(),
                "Analysisdata sizeof mismatch"
            );
            assert_eq!(
                offset_of!(Analysisdata, options),
                probe_offsetof_ad_options(),
                "options offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, is_running),
                probe_offsetof_ad_isRunning(),
                "isRunning offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, n_time_steps),
                probe_offsetof_ad_nTimeSteps(),
                "nTimeSteps offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, n_current_contacts),
                probe_offsetof_ad_nCurrentContacts(),
                "nCurrentContacts offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, m9),
                probe_offsetof_ad_m9(),
                "m9 offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, oc_converged),
                probe_offsetof_ad_OCConverged(),
                "OCConverged offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, cts),
                probe_offsetof_ad_cts(),
                "cts offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, elapsed_time),
                probe_offsetof_ad_elapsedTime(),
                "elapsedTime offset"
            );
            assert_eq!(
                offset_of!(Analysisdata, delta_t),
                probe_offsetof_ad_delta_t(),
                "delta_t offset"
            );
        }
    }

    /// Full pipeline test: load geometry, ddacut, load analysis, ddanalysis.
    /// Uses the pushblock example files from the repo.
    #[test]
    fn pushblock_analysis_runs_without_crash() {
        use std::ffi::CString;
        use std::path::Path;

        // Locate example files relative to the Cargo.toml (src-tauri/)
        let manifest_dir = env!("CARGO_MANIFEST_DIR");
        let repo_root = Path::new(manifest_dir)
            .parent().unwrap()  // src/platform/tauri/
            .parent().unwrap()  // src/platform/
            .parent().unwrap()  // src/
            .parent().unwrap(); // repo root
        let geo_path = repo_root.join("tests/fixtures/pushblock.geo");
        let ana_path = repo_root.join("tests/fixtures/pushblock.ana");
        assert!(geo_path.exists(), "pushblock.geo not found at {:?}", geo_path);
        assert!(ana_path.exists(), "pushblock.ana not found at {:?}", ana_path);

        unsafe {
            // 1. Create DDA engine
            let dda = dda_new();
            assert!(!dda.is_null(), "dda_new returned null");

            // 2. Load geometry
            let gd = gdata_new();
            assert!(!gd.is_null(), "gdata_new returned null");
            (*gd).display_error = Some(dda_display_error);
            (*gd).display_warning = Some(dda_display_warning);

            let c_geo = CString::new(geo_path.to_str().unwrap()).unwrap();
            gdata_read_input_file(gd, c_geo.as_ptr() as *mut _);
            dda_set_geometrydata(dda, gd);

            assert!((*gd).n_joints > 0, "no joints after reading geometry");

            // 3. ddacut
            // CWD must be writable (for cut.log, pnp.log)
            let prev_dir = std::env::current_dir().ok();
            let geo_dir = geo_path.parent().unwrap();
            let _ = std::env::set_current_dir(geo_dir);

            ddacut(gd);

            // Fix macOS rand()/RAND_MAX bug: crr() misclassifies all points
            // as "outside" because rand()/2048.0 overflows on macOS.
            // See bridge.rs for full explanation.
            crate::bridge::fix_point_block_assignments(&*gd);

            assert!((*gd).n_blocks > 0, "no blocks after ddacut");
            assert!(!(*gd).moments.is_null(), "moments is NULL after ddacut");
            assert!(!(*gd).vertices.is_null(), "vertices is NULL after ddacut");
            assert!(!(*gd).vindex.is_null(), "vindex is NULL after ddacut");

            // 4. Load analysis
            let ad = adata_new();
            assert!(!ad.is_null(), "adata_new returned null");
            (*ad).display_error = Some(dda_display_error);
            (*ad).display_warning = Some(dda_display_warning);

            let c_ana = CString::new(ana_path.to_str().unwrap()).unwrap();
            adata_read_input_file(
                ad,
                c_ana.as_ptr() as *mut _,
                (*gd).n_f_points,
                (*gd).point_count,
                (*gd).n_l_points,
            );
            dda_set_analysisdata(dda, ad);

            assert!((*ad).n_time_steps > 0, "no timesteps after reading analysis");

            // 5. Set up filepaths (rootname required by openAnalysisFiles)
            let mut fp: Filepaths = std::mem::zeroed();
            initFilePaths(&mut fp);
            // Copy geo and ana paths
            let geo_bytes = geo_path.to_str().unwrap().as_bytes();
            let ana_bytes = ana_path.to_str().unwrap().as_bytes();
            for (i, &b) in geo_bytes.iter().enumerate().take(FNAME_BUFSIZE - 1) {
                fp.gfile[i] = b as c_char;
            }
            for (i, &b) in ana_bytes.iter().enumerate().take(FNAME_BUFSIZE - 1) {
                fp.afile[i] = b as c_char;
            }
            // rootname = file stem without extension
            let stem = ana_path.file_stem().unwrap().to_str().unwrap();
            for (i, &b) in stem.as_bytes().iter().enumerate().take(FNAME_BUFSIZE - 1) {
                fp.rootname[i] = b as c_char;
            }

            // 6. Dump state before analysis
            eprintln!("[test] gd ptr={:p}", gd);
            eprintln!("[test] gd.nBlocks={} nFPoints={} nLPoints={} nMPoints={} nHPoints={} nPoints={} pointCount={}",
                (*gd).n_blocks, (*gd).n_f_points, (*gd).n_l_points,
                (*gd).n_m_points, (*gd).n_h_points, (*gd).n_points, (*gd).point_count);
            eprintln!("[test] gd.moments={:p} momentsize1={} momentsize2={}",
                (*gd).moments, (*gd).momentsize1, (*gd).momentsize2);
            eprintln!("[test] gd.points={:p} pointsize1={} pointsize2={}",
                (*gd).points, (*gd).pointsize1, (*gd).pointsize2);
            eprintln!("[test] gd.vertices={:p} vindex={:p}", (*gd).vertices, (*gd).vindex);
            eprintln!("[test] ad ptr={:p}", ad);
            eprintln!("[test] ad.nTimeSteps={} delta_t={}", (*ad).n_time_steps, (*ad).delta_t);

            // Run analysis
            let result = ddanalysis(dda, &mut fp);
            assert_eq!(result, 1, "ddanalysis should return 1 (TRUE) on success");

            // 7. Verify post-analysis state
            assert!((*ad).cts > 0, "no timesteps completed");

            // Restore CWD
            if let Some(dir) = prev_dir {
                let _ = std::env::set_current_dir(dir);
            }

            // 8. Cleanup
            dda_delete(dda);
        }
    }

    #[test]
    fn filepaths_layout_matches_c() {
        unsafe {
            assert_eq!(
                size_of::<Filepaths>(),
                probe_sizeof_filepaths(),
                "Filepaths sizeof mismatch"
            );
            assert_eq!(
                offset_of!(Filepaths, gfile),
                probe_offsetof_fp_gfile(),
                "gfile offset"
            );
            assert_eq!(
                offset_of!(Filepaths, afile),
                probe_offsetof_fp_afile(),
                "afile offset"
            );
        }
    }
}
