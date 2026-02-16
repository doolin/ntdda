mod bridge;
mod commands;
mod ffi;

use commands::AppState;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
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
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
