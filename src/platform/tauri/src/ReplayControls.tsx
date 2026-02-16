import { useState, useRef, useCallback, useEffect } from "react";
import * as cmd from "./commands";
import type { SceneData, ReplayInfo } from "./types";

interface Props {
  onFrame: (scene: SceneData) => void;
  onClose: () => void;
}

const SPEEDS = [1, 2, 5, 10];

export default function ReplayControls({ onFrame, onClose }: Props) {
  const [info, setInfo] = useState<ReplayInfo | null>(null);
  const [frameIdx, setFrameIdx] = useState(0);
  const [playing, setPlaying] = useState(false);
  const [speed, setSpeed] = useState(1);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const playingRef = useRef(false);
  const frameRef = useRef(0);
  const speedRef = useRef(1);
  const infoRef = useRef<ReplayInfo | null>(null);
  const rafRef = useRef<number>(0);
  const lastTimeRef = useRef(0);

  // Load replay data on mount
  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const replayInfo = await cmd.loadReplay();
        if (cancelled) return;
        setInfo(replayInfo);
        infoRef.current = replayInfo;
        setLoading(false);
        // Show first frame
        if (replayInfo.num_frames > 0) {
          const scene = await cmd.getReplayFrame(0);
          if (!cancelled) onFrame(scene);
        }
      } catch (e) {
        if (!cancelled) {
          setError(String(e));
          setLoading(false);
        }
      }
    })();
    return () => { cancelled = true; };
  }, []);

  // Sync refs
  useEffect(() => { playingRef.current = playing; }, [playing]);
  useEffect(() => { speedRef.current = speed; }, [speed]);

  const showFrame = useCallback(async (idx: number) => {
    try {
      const scene = await cmd.getReplayFrame(idx);
      onFrame(scene);
      setFrameIdx(idx);
      frameRef.current = idx;
    } catch (e) {
      setPlaying(false);
      setError(String(e));
    }
  }, [onFrame]);

  // Animation loop
  const animate = useCallback((timestamp: number) => {
    if (!playingRef.current || !infoRef.current) return;

    const elapsed = timestamp - lastTimeRef.current;
    // Base rate: ~30 fps at 1x speed
    const interval = 1000 / (30 * speedRef.current);

    if (elapsed >= interval) {
      lastTimeRef.current = timestamp;
      const nextFrame = frameRef.current + 1;
      if (nextFrame >= infoRef.current.num_frames) {
        // Reached end
        setPlaying(false);
        return;
      }
      showFrame(nextFrame);
    }

    rafRef.current = requestAnimationFrame(animate);
  }, [showFrame]);

  const handlePlay = useCallback(() => {
    if (!info || info.num_frames === 0) return;
    // If at end, restart from beginning
    if (frameRef.current >= info.num_frames - 1) {
      frameRef.current = 0;
      setFrameIdx(0);
    }
    setPlaying(true);
    lastTimeRef.current = performance.now();
    rafRef.current = requestAnimationFrame(animate);
  }, [info, animate]);

  const handlePause = useCallback(() => {
    setPlaying(false);
    if (rafRef.current) cancelAnimationFrame(rafRef.current);
  }, []);

  const handleStop = useCallback(() => {
    setPlaying(false);
    if (rafRef.current) cancelAnimationFrame(rafRef.current);
    showFrame(0);
  }, [showFrame]);

  const handleSlider = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    const idx = parseInt(e.target.value, 10);
    showFrame(idx);
  }, [showFrame]);

  const handleSpeed = useCallback(() => {
    setSpeed(prev => {
      const idx = SPEEDS.indexOf(prev);
      return SPEEDS[(idx + 1) % SPEEDS.length];
    });
  }, []);

  // Cleanup animation on unmount
  useEffect(() => {
    return () => {
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
    };
  }, []);

  if (loading) {
    return <div className="replay-controls">Loading replay data...</div>;
  }

  if (error) {
    return (
      <div className="replay-controls">
        <span className="replay-error">{error}</span>
        <button onClick={onClose}>Close</button>
      </div>
    );
  }

  if (!info || info.num_frames === 0) {
    return (
      <div className="replay-controls">
        <span>No replay data available</span>
        <button onClick={onClose}>Close</button>
      </div>
    );
  }

  return (
    <div className="replay-controls">
      {!playing ? (
        <button onClick={handlePlay} title="Play">Play</button>
      ) : (
        <button onClick={handlePause} title="Pause">Pause</button>
      )}
      <button onClick={handleStop} title="Stop">Stop</button>
      <input
        type="range"
        className="replay-slider"
        min={0}
        max={info.num_frames - 1}
        value={frameIdx}
        onChange={handleSlider}
      />
      <span className="replay-frame-counter">
        {frameIdx + 1} / {info.num_frames}
      </span>
      <button onClick={handleSpeed} className="replay-speed" title="Playback speed">
        {speed}x
      </button>
      <button onClick={onClose} title="Close replay">Close</button>
    </div>
  );
}
