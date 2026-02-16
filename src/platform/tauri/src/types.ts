export type AppPhase =
  | "Empty"
  | "GeometryLoaded"
  | "GeometryCut"
  | "AnalysisLoaded"
  | "Running"
  | "Finished";

export interface Block {
  id: number;
  vertices: [number, number][];
  material_id: number;
}

export interface JointData {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
  joint_type: number;
}

export interface PointData {
  x: number;
  y: number;
  point_type: number; // 0=fixed, 1=load, 2=measured, 3=hole
}

export interface BoltData {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

export interface SceneData {
  blocks: Block[];
  joints: JointData[];
  points: PointData[];
  bolts: BoltData[];
  bbox: [number, number, number, number]; // [min_x, max_x, min_y, max_y]
}

export interface AnalysisState {
  time_step: number;
  elapsed_time: number;
  num_contacts: number;
  converged: boolean;
}

export interface PhaseResponse {
  phase: AppPhase;
}

export interface SceneResponse {
  phase: AppPhase;
  scene: SceneData;
}

export interface AnalysisResponse {
  phase: AppPhase;
  scene: SceneData;
  original_scene: SceneData | null;
  analysis: AnalysisState;
}

export interface ReplayInfo {
  num_frames: number;
  num_blocks: number;
  num_bolts: number;
}
