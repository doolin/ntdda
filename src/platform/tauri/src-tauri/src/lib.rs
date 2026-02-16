mod bridge;
mod commands;
mod ffi;
mod replay;

use commands::AppState;

/// Install a SIGSEGV/SIGBUS handler that prints the faulting address.
fn install_crash_handler() {
    unsafe {
        let mut sa: libc::sigaction = std::mem::zeroed();
        sa.sa_sigaction = crash_handler as *const () as usize;
        sa.sa_flags = libc::SA_SIGINFO;
        libc::sigemptyset(&mut sa.sa_mask);
        libc::sigaction(libc::SIGSEGV, &sa, std::ptr::null_mut());
        libc::sigaction(libc::SIGBUS, &sa, std::ptr::null_mut());
    }
}

extern "C" fn crash_handler(sig: libc::c_int, info: *mut libc::siginfo_t, _ctx: *mut libc::c_void) {
    unsafe {
        let name = if sig == libc::SIGSEGV { "SIGSEGV" } else { "SIGBUS" };
        let addr = if !info.is_null() { (*info).si_addr } else { std::ptr::null_mut() };
        let msg = format!("\n[ntdda] FATAL: {} at address {:?}\n", name, addr);
        libc::write(2, msg.as_ptr() as *const libc::c_void, msg.len());

        // Dump backtrace using macOS execinfo
        extern "C" {
            fn backtrace(buffer: *mut *mut libc::c_void, size: libc::c_int) -> libc::c_int;
            fn backtrace_symbols(buffer: *const *mut libc::c_void, size: libc::c_int) -> *mut *mut libc::c_char;
        }
        let mut bt_buf: [*mut libc::c_void; 64] = [std::ptr::null_mut(); 64];
        let n = backtrace(bt_buf.as_mut_ptr(), 64);
        let syms = backtrace_symbols(bt_buf.as_ptr(), n);
        if !syms.is_null() {
            let hdr = b"[ntdda] backtrace:\n";
            libc::write(2, hdr.as_ptr() as *const libc::c_void, hdr.len());
            for i in 0..n as usize {
                let s = *syms.add(i);
                if !s.is_null() {
                    let len = libc::strlen(s);
                    libc::write(2, s as *const libc::c_void, len);
                    libc::write(2, b"\n".as_ptr() as *const libc::c_void, 1);
                }
            }
            libc::free(syms as *mut libc::c_void);
        }

        // Re-raise with default handler
        libc::signal(sig, libc::SIG_DFL);
        libc::raise(sig);
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    install_crash_handler();
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_dialog::init())
        .manage(AppState {
            engine: std::sync::Mutex::new(bridge::DdaEngine::new()),
        })
        .invoke_handler(tauri::generate_handler![
            commands::get_phase,
            commands::open_geometry,
            commands::apply_geometry,
            commands::open_analysis,
            commands::run_analysis,
            commands::get_scene,
            commands::load_replay,
            commands::get_replay_frame,
            commands::get_replay_info,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
