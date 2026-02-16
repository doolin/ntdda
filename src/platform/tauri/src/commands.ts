import { invoke } from "@tauri-apps/api/core";
import type {
  PhaseResponse,
  SceneResponse,
  AnalysisResponse,
  SceneData,
  ReplayInfo,
} from "./types";

export async function getPhase(): Promise<PhaseResponse> {
  return invoke<PhaseResponse>("get_phase");
}

export async function openGeometry(path: string): Promise<PhaseResponse> {
  return invoke<PhaseResponse>("open_geometry", { path });
}

export async function applyGeometry(): Promise<SceneResponse> {
  return invoke<SceneResponse>("apply_geometry");
}

export async function openAnalysis(path: string): Promise<PhaseResponse> {
  return invoke<PhaseResponse>("open_analysis", { path });
}

export async function runAnalysis(): Promise<AnalysisResponse> {
  return invoke<AnalysisResponse>("run_analysis");
}

export async function getScene(): Promise<SceneData | null> {
  return invoke<SceneData | null>("get_scene");
}

export async function loadReplay(): Promise<ReplayInfo> {
  return invoke<ReplayInfo>("load_replay");
}

export async function getReplayFrame(index: number): Promise<SceneData> {
  return invoke<SceneData>("get_replay_frame", { index });
}

export async function getReplayInfo(): Promise<ReplayInfo> {
  return invoke<ReplayInfo>("get_replay_info");
}

export async function getStartupFile(): Promise<string | null> {
  return invoke<string | null>("get_startup_file");
}

export async function quitApp(): Promise<void> {
  return invoke<void>("quit_app");
}
