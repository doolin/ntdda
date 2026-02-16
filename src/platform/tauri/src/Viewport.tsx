import { useRef, useEffect, useCallback } from "react";
import type { SceneData } from "./types";

// Material fill colors matching Win32 wingraph.c (6 materials)
const MATERIAL_COLORS = [
  "#ffffff", // white
  "#d0d0d0", // light gray
  "#b0d0ff", // light blue
  "#b0ffb0", // light green
  "#ffffb0", // light yellow
  "#ffb0b0", // light pink
];

// Joint type colors matching Win32 (10 types)
const JOINT_COLORS = [
  "#ffffff", // 0: white
  "#000000", // 1: black
  "#ff0000", // 2: red
  "#0000ff", // 3: blue
  "#00aa00", // 4: green
  "#800080", // 5: purple
  "#cccc00", // 6: yellow
  "#8b4513", // 7: brown
  "#808000", // 8: olive
  "#808080", // 9: gray
];

interface Transform {
  offsetX: number;
  offsetY: number;
  scale: number;
}

interface Props {
  scene: SceneData | null;
  originalScene?: SceneData | null;
  showOriginal?: boolean;
}

export default function Viewport({ scene, originalScene, showOriginal }: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const transformRef = useRef<Transform>({ offsetX: 0, offsetY: 0, scale: 1 });
  const dragRef = useRef<{ startX: number; startY: number; origOffsetX: number; origOffsetY: number } | null>(null);

  const fitToCanvas = useCallback(() => {
    if (!scene || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const [minX, maxX, minY, maxY] = scene.bbox;
    const sceneW = maxX - minX;
    const sceneH = maxY - minY;

    if (sceneW <= 0 || sceneH <= 0) return;

    const margin = 40;
    const availW = canvas.width / devicePixelRatio - 2 * margin;
    const availH = canvas.height / devicePixelRatio - 2 * margin;

    const scale = Math.min(availW / sceneW, availH / sceneH);
    const cx = (minX + maxX) / 2;
    const cy = (minY + maxY) / 2;

    transformRef.current = {
      scale,
      offsetX: canvas.width / devicePixelRatio / 2 - cx * scale,
      offsetY: canvas.height / devicePixelRatio / 2 + cy * scale, // flip Y
    };
  }, [scene]);

  const worldToScreen = useCallback(
    (wx: number, wy: number): [number, number] => {
      const t = transformRef.current;
      return [wx * t.scale + t.offsetX, -wy * t.scale + t.offsetY];
    },
    []
  );

  const render = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    const dpr = devicePixelRatio;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, canvas.width / dpr, canvas.height / dpr);

    if (showOriginal && originalScene) {
      renderScene(ctx, originalScene, true);
    }
    if (scene) {
      renderScene(ctx, scene, false);
    }
  }, [scene, originalScene, showOriginal, worldToScreen]);

  function renderScene(
    ctx: CanvasRenderingContext2D,
    data: SceneData,
    isOverlay: boolean
  ) {
    // Draw blocks
    for (const block of data.blocks) {
      if (block.vertices.length < 3) continue;

      ctx.beginPath();
      const [sx, sy] = worldToScreen(block.vertices[0][0], block.vertices[0][1]);
      ctx.moveTo(sx, sy);
      for (let i = 1; i < block.vertices.length; i++) {
        const [x, y] = worldToScreen(block.vertices[i][0], block.vertices[i][1]);
        ctx.lineTo(x, y);
      }
      ctx.closePath();

      if (isOverlay) {
        ctx.globalAlpha = 0.2;
        ctx.fillStyle = "#cccccc";
        ctx.fill();
        ctx.globalAlpha = 0.5;
        ctx.strokeStyle = "#999999";
        ctx.setLineDash([4, 4]);
        ctx.stroke();
        ctx.setLineDash([]);
        ctx.globalAlpha = 1;
      } else {
        const colorIdx = block.material_id % MATERIAL_COLORS.length;
        ctx.fillStyle = MATERIAL_COLORS[colorIdx];
        ctx.fill();
        ctx.strokeStyle = "#000000";
        ctx.lineWidth = 1;
        ctx.stroke();
      }
    }

    if (isOverlay) return;

    // Draw joints
    for (const joint of data.joints) {
      const [x1, y1] = worldToScreen(joint.x1, joint.y1);
      const [x2, y2] = worldToScreen(joint.x2, joint.y2);
      const colorIdx = Math.abs(joint.joint_type) % JOINT_COLORS.length;
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.strokeStyle = JOINT_COLORS[colorIdx];
      ctx.lineWidth = 1;
      ctx.stroke();
    }

    // Draw points
    const ptSize = 6;
    for (const pt of data.points) {
      const [sx, sy] = worldToScreen(pt.x, pt.y);

      if (pt.point_type === 0) {
        // Fixed point: green triangle
        ctx.beginPath();
        ctx.moveTo(sx, sy - ptSize);
        ctx.lineTo(sx - ptSize, sy + ptSize);
        ctx.lineTo(sx + ptSize, sy + ptSize);
        ctx.closePath();
        ctx.fillStyle = "#00aa00";
        ctx.fill();
      } else if (pt.point_type === 1) {
        // Load point: black triangle (inverted)
        ctx.beginPath();
        ctx.moveTo(sx, sy + ptSize);
        ctx.lineTo(sx - ptSize, sy - ptSize);
        ctx.lineTo(sx + ptSize, sy - ptSize);
        ctx.closePath();
        ctx.fillStyle = "#000000";
        ctx.fill();
      } else if (pt.point_type === 2) {
        // Measured point: white circle
        ctx.beginPath();
        ctx.arc(sx, sy, ptSize, 0, Math.PI * 2);
        ctx.fillStyle = "#ffffff";
        ctx.fill();
        ctx.strokeStyle = "#000000";
        ctx.lineWidth = 1;
        ctx.stroke();
      } else if (pt.point_type === 3) {
        // Hole point: black circle
        ctx.beginPath();
        ctx.arc(sx, sy, ptSize, 0, Math.PI * 2);
        ctx.fillStyle = "#000000";
        ctx.fill();
      }
    }

    // Draw bolts
    for (const bolt of data.bolts) {
      const [x1, y1] = worldToScreen(bolt.x1, bolt.y1);
      const [x2, y2] = worldToScreen(bolt.x2, bolt.y2);
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.strokeStyle = "#000000";
      ctx.lineWidth = 2;
      ctx.stroke();
    }
  }

  // Resize handler
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const resize = () => {
      const parent = canvas.parentElement;
      if (!parent) return;
      const dpr = devicePixelRatio;
      canvas.width = parent.clientWidth * dpr;
      canvas.height = parent.clientHeight * dpr;
      canvas.style.width = parent.clientWidth + "px";
      canvas.style.height = parent.clientHeight + "px";
      fitToCanvas();
      render();
    };

    const observer = new ResizeObserver(resize);
    observer.observe(canvas.parentElement!);
    resize();

    return () => observer.disconnect();
  }, [fitToCanvas, render]);

  // Re-render when scene changes
  useEffect(() => {
    fitToCanvas();
    render();
  }, [scene, originalScene, showOriginal, fitToCanvas, render]);

  // Mouse wheel zoom
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const onWheel = (e: WheelEvent) => {
      e.preventDefault();
      const t = transformRef.current;
      const factor = e.deltaY > 0 ? 0.9 : 1.1;

      const rect = canvas.getBoundingClientRect();
      const mx = e.clientX - rect.left;
      const my = e.clientY - rect.top;

      t.offsetX = mx - (mx - t.offsetX) * factor;
      t.offsetY = my - (my - t.offsetY) * factor;
      t.scale *= factor;
      render();
    };

    canvas.addEventListener("wheel", onWheel, { passive: false });
    return () => canvas.removeEventListener("wheel", onWheel);
  }, [render]);

  // Mouse drag pan
  const onMouseDown = (e: React.MouseEvent) => {
    const t = transformRef.current;
    dragRef.current = {
      startX: e.clientX,
      startY: e.clientY,
      origOffsetX: t.offsetX,
      origOffsetY: t.offsetY,
    };
  };

  const onMouseMove = (e: React.MouseEvent) => {
    if (!dragRef.current) return;
    const t = transformRef.current;
    t.offsetX = dragRef.current.origOffsetX + (e.clientX - dragRef.current.startX);
    t.offsetY = dragRef.current.origOffsetY + (e.clientY - dragRef.current.startY);
    render();
  };

  const onMouseUp = () => {
    dragRef.current = null;
  };

  return (
    <canvas
      ref={canvasRef}
      onMouseDown={onMouseDown}
      onMouseMove={onMouseMove}
      onMouseUp={onMouseUp}
      onMouseLeave={onMouseUp}
    />
  );
}
