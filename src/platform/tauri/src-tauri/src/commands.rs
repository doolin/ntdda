//! Tauri IPC command handlers.

use crate::bridge::{AnalysisState, AppPhase, DdaEngine, ReplayInfo, SceneData};
use serde::Serialize;
use std::sync::Mutex;
use tauri::State;

pub struct AppState {
    pub engine: Mutex<DdaEngine>,
}

#[derive(Serialize)]
pub struct PhaseResponse {
    pub phase: AppPhase,
}

#[derive(Serialize)]
pub struct SceneResponse {
    pub phase: AppPhase,
    pub scene: SceneData,
}

#[derive(Serialize)]
pub struct AnalysisResponse {
    pub phase: AppPhase,
    pub scene: SceneData,
    pub original_scene: Option<SceneData>,
    pub analysis: AnalysisState,
}

#[tauri::command]
pub fn get_phase(state: State<AppState>) -> Result<PhaseResponse, String> {
    let engine = state.engine.lock().map_err(|e| e.to_string())?;
    Ok(PhaseResponse {
        phase: engine.phase().clone(),
    })
}

#[tauri::command]
pub fn open_geometry(state: State<AppState>, path: String) -> Result<PhaseResponse, String> {
    let mut engine = state.engine.lock().map_err(|e| e.to_string())?;
    engine.load_geometry(&path)?;
    Ok(PhaseResponse {
        phase: engine.phase().clone(),
    })
}

#[tauri::command]
pub fn apply_geometry(state: State<AppState>) -> Result<SceneResponse, String> {
    let mut engine = state.engine.lock().map_err(|e| e.to_string())?;
    let scene = engine.apply_geometry()?;
    Ok(SceneResponse {
        phase: engine.phase().clone(),
        scene,
    })
}

#[tauri::command]
pub fn open_analysis(state: State<AppState>, path: String) -> Result<PhaseResponse, String> {
    let mut engine = state.engine.lock().map_err(|e| e.to_string())?;
    engine.load_analysis(&path)?;
    Ok(PhaseResponse {
        phase: engine.phase().clone(),
    })
}

#[tauri::command]
pub fn run_analysis(state: State<AppState>) -> Result<AnalysisResponse, String> {
    let mut engine = state.engine.lock().map_err(|e| e.to_string())?;
    let (scene, analysis) = engine.run_analysis()?;
    let original_scene = engine.get_original_scene().cloned();
    Ok(AnalysisResponse {
        phase: engine.phase().clone(),
        scene,
        original_scene,
        analysis,
    })
}

#[tauri::command]
pub fn get_scene(state: State<AppState>) -> Result<Option<SceneData>, String> {
    let engine = state.engine.lock().map_err(|e| e.to_string())?;
    Ok(engine.get_scene().cloned())
}

#[tauri::command]
pub fn load_replay(state: State<AppState>) -> Result<ReplayInfo, String> {
    let mut engine = state.engine.lock().map_err(|e| e.to_string())?;
    engine.load_replay()
}

#[tauri::command]
pub fn get_replay_frame(state: State<AppState>, index: usize) -> Result<SceneData, String> {
    let engine = state.engine.lock().map_err(|e| e.to_string())?;
    engine.get_replay_frame(index)
}

#[tauri::command]
pub fn get_replay_info(state: State<AppState>) -> Result<ReplayInfo, String> {
    let engine = state.engine.lock().map_err(|e| e.to_string())?;
    engine.get_replay_info()
}
