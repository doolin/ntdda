import { useState, useCallback, useEffect } from "react";
import { open } from "@tauri-apps/plugin-dialog";
import * as cmd from "./commands";
import type { AppPhase, SceneData, AnalysisState } from "./types";
import Viewport from "./Viewport";
import ReplayControls from "./ReplayControls";
import "./App.css";

function App() {
  const [phase, setPhase] = useState<AppPhase>("Empty");
  const [scene, setScene] = useState<SceneData | null>(null);
  const [originalScene, setOriginalScene] = useState<SceneData | null>(null);
  const [analysis, setAnalysis] = useState<AnalysisState | null>(null);
  const [showOriginal, setShowOriginal] = useState(false);
  const [replayActive, setReplayActive] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  const clearError = () => setError(null);

  // Auto-load and apply geometry file passed via -g flag or NTDDA_GEO env var
  useEffect(() => {
    cmd.getStartupFile().then(async (path) => {
      if (!path) return;
      try {
        setBusy(true);
        const loadResp = await cmd.openGeometry(path);
        setPhase(loadResp.phase);
        const applyResp = await cmd.applyGeometry();
        setPhase(applyResp.phase);
        setScene(applyResp.scene);
        setOriginalScene(applyResp.scene);
      } catch (e) {
        setError(String(e));
      } finally {
        setBusy(false);
      }
    });
  }, []);

  async function handleOpenGeometry() {
    clearError();
    try {
      const selected = await open({
        filters: [
          { name: "Geometry", extensions: ["geo", "xml"] },
          { name: "All Files", extensions: ["*"] },
        ],
      });
      if (!selected) return;
      setBusy(true);
      const resp = await cmd.openGeometry(selected as string);
      setPhase(resp.phase);
      setScene(null);
      setOriginalScene(null);
      setAnalysis(null);
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  async function handleApplyGeometry() {
    clearError();
    try {
      setBusy(true);
      const resp = await cmd.applyGeometry();
      console.log("[ntdda] applyGeometry response:", JSON.stringify(resp, null, 2));
      console.log("[ntdda] blocks:", resp.scene.blocks.length,
        "joints:", resp.scene.joints.length,
        "bbox:", resp.scene.bbox);
      setPhase(resp.phase);
      setScene(resp.scene);
      setOriginalScene(resp.scene);
    } catch (e) {
      console.error("[ntdda] applyGeometry error:", e);
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  async function handleOpenAnalysis() {
    clearError();
    try {
      const selected = await open({
        filters: [
          { name: "Analysis", extensions: ["ana", "xml"] },
          { name: "All Files", extensions: ["*"] },
        ],
      });
      if (!selected) return;
      setBusy(true);
      const resp = await cmd.openAnalysis(selected as string);
      setPhase(resp.phase);
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  async function handleRunAnalysis() {
    clearError();
    try {
      setBusy(true);
      setPhase("Running");
      const resp = await cmd.runAnalysis();
      setPhase(resp.phase);
      setScene(resp.scene);
      if (resp.original_scene) {
        setOriginalScene(resp.original_scene);
      }
      setAnalysis(resp.analysis);
    } catch (e) {
      setError(String(e));
      setPhase("AnalysisLoaded");
    } finally {
      setBusy(false);
    }
  }

  const handleReplayFrame = useCallback((frameScene: SceneData) => {
    setScene(frameScene);
  }, []);

  const handleReplayClose = useCallback(() => {
    setReplayActive(false);
    // Restore final analysis scene
    cmd.getScene().then(s => { if (s) setScene(s); });
  }, []);

  const canOpenGeo = !busy && phase !== "Running" && !replayActive;
  const canApply =
    !busy && !replayActive &&
    (phase === "GeometryLoaded" || phase === "GeometryCut" ||
     phase === "AnalysisLoaded" || phase === "Finished");
  const canOpenAna =
    !busy && !replayActive &&
    (phase === "GeometryCut" || phase === "AnalysisLoaded" || phase === "Finished");
  const canRun =
    !busy && !replayActive && (phase === "AnalysisLoaded" || phase === "Finished");
  const canReplay = !busy && !replayActive && phase === "Finished";

  let statusText = "Ready";
  if (replayActive) statusText = "Replay";
  else if (phase === "GeometryLoaded") statusText = "Geometry loaded";
  else if (phase === "GeometryCut") statusText = "Geometry applied";
  else if (phase === "AnalysisLoaded") statusText = "Analysis loaded";
  else if (phase === "Running") statusText = "Running...";
  else if (phase === "Finished") statusText = "Analysis complete";

  const blockCount = scene?.blocks.length ?? 0;

  return (
    <div className="app">
      <div className="toolbar">
        <span className="app-title">ntdda</span>
        <button onClick={handleOpenGeometry} disabled={!canOpenGeo}>
          Open Geometry
        </button>
        <button onClick={handleApplyGeometry} disabled={!canApply}>
          Apply
        </button>
        <button onClick={handleOpenAnalysis} disabled={!canOpenAna}>
          Open Analysis
        </button>
        <button onClick={handleRunAnalysis} disabled={!canRun}>
          Run
        </button>
        <button onClick={() => setReplayActive(true)} disabled={!canReplay}>
          Replay
        </button>
        <span className="toolbar-spacer" />
        <button onClick={() => cmd.quitApp()}>
          Quit
        </button>
        {phase === "Finished" && originalScene && !replayActive && (
          <label className="toggle">
            <input
              type="checkbox"
              checked={showOriginal}
              onChange={(e) => setShowOriginal(e.target.checked)}
            />
            Show Original
          </label>
        )}
      </div>

      <div className="viewport">
        <Viewport
          scene={scene}
          originalScene={originalScene}
          showOriginal={showOriginal}
        />
      </div>

      {replayActive && (
        <ReplayControls
          onFrame={handleReplayFrame}
          onClose={handleReplayClose}
        />
      )}

      <div className="statusbar">
        <span className="status-left">{statusText}</span>
        {blockCount > 0 && (
          <span className="status-center">{blockCount} blocks</span>
        )}
        {analysis && (
          <span className="status-right">
            Step {analysis.time_step} | t={analysis.elapsed_time.toFixed(4)} |{" "}
            {analysis.num_contacts} contacts
          </span>
        )}
      </div>

      {error && (
        <div className="error-toast" onClick={clearError}>
          {error}
        </div>
      )}
    </div>
  );
}

export default App;
