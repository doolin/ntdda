use std::path::PathBuf;

fn main() {
    tauri_build::build();

    // Path from src-tauri/ to the ntdda repo root
    let manifest_dir = PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").unwrap());
    let repo_root = manifest_dir
        .parent() // tauri/
        .unwrap()
        .parent() // platform/
        .unwrap()
        .parent() // src/
        .unwrap()
        .parent() // ntdda/
        .unwrap();

    let src_dir = repo_root.join("src");
    let include_dir = repo_root.join("include");
    let stubs_dir = src_dir.join("platform").join("stubs");

    // The same source files as src/CMakeLists.txt LIBDDA_SRCS
    let libdda_srcs = [
        "analysisdata.c",
        "analysisddaml.c",
        "analysisdriver.c",
        "analysisreader.c",
        "blockhandler.c",
        "blocks.c",
        "bolt.c",
        "combineddf.c",
        "constants.c",
        "contact.c",
        "contacts.c",
        "dda.c",
        "dda_error.c",
        "ddadlist.c",
        "ddafile.c",
        "ddaml.c",
        "ddamemory.c",
        "display.c",
        "dxf.c",
        "friction.c",
        "functions.c",
        "geomddaml.c",
        "geomdriver.c",
        "geometrydata.c",
        "geomreader.c",
        "ghssolver.c",
        "graphics.c",
        "gravity.c",
        "inpoly.c",
        "joint.c",
        "lns.c",
        "loadmatrix.c",
        "loadpoint.c",
        "loghtml.c",
        "material.c",
        "mohrcoulomb.c",
        "postprocess.c",
        "rb.c",
        "replay.c",
        "sparsestorage.c",
        "stress.c",
        "timehistory.c",
        "timestep.c",
        "transfercontact.c",
        "utils.c",
        "datalog.c",
    ];

    let mut build = cc::Build::new();
    build
        .std("c99")
        .define("LIBXML_STATIC", None)
        // ntdda headers
        .include(&include_dir)
        // stubs dir provides windef.h, direct.h, malloc.h shims
        .include(&stubs_dir)
        .warnings(false);

    // Add all libdda source files
    for src in &libdda_srcs {
        build.file(src_dir.join(src));
    }

    // Add the stubs files
    build.file(stubs_dir.join("stubs.c"));
    build.file(manifest_dir.join("tauri_stubs.c"));

    build.compile("dda");

    // Compile the probe (sizeof/offsetof validation) as a separate library
    cc::Build::new()
        .std("c99")
        .define("LIBXML_STATIC", None)
        .include(&include_dir)
        .include(&stubs_dir)
        .warnings(false)
        .file(manifest_dir.join("probe.c"))
        .compile("dda_probe");

    // Link libxml2
    println!("cargo:rustc-link-lib=xml2");

    // Rebuild if any C source changes
    println!("cargo:rerun-if-changed={}", src_dir.display());
    println!("cargo:rerun-if-changed={}", include_dir.display());
    println!("cargo:rerun-if-changed={}", stubs_dir.display());
    println!("cargo:rerun-if-changed={}", manifest_dir.join("probe.c").display());
}
