//! Parser for ntdda `.replay` files.
//!
//! The replay file is written by `initReplayFile()` (header) and
//! `writeReplayFile()` (per-timestep data) in `src/replay.c`.

use crate::bridge::{Block, BoltData, PointData, SceneData};
use std::io::{BufRead, BufReader};
use std::path::Path;

/// Parsed replay data: header + all frames.
pub struct ReplayData {
    pub num_blocks: usize,
    pub num_bolts: usize,
    pub n_f_points: usize,
    pub n_l_points: usize,
    pub n_m_points: usize,
    /// vindex[block] = (material, start_vertex, end_vertex), block 0..=num_blocks
    pub vindex: Vec<[i32; 3]>,
    pub frames: Vec<ReplayFrame>,
}

/// One timestep of replay data.
pub struct ReplayFrame {
    /// All vertices: [material, x, y] for 0..=vertex_count
    pub vertices: Vec<[f64; 3]>,
    /// Bolts: [x1,y1,x2,y2, n1,n2, e0,t0,f0] for each bolt
    pub bolts: Vec<[f64; 9]>,
    /// Points: [col0, col1, col2, col3] for 0..=point_count
    pub points: Vec<[f64; 4]>,
}

impl ReplayData {
    /// Convert a replay frame to a SceneData for rendering.
    pub fn frame_to_scene(&self, frame_idx: usize, bbox: [f64; 4]) -> Option<SceneData> {
        let frame = self.frames.get(frame_idx)?;

        let mut blocks = Vec::with_capacity(self.num_blocks);
        for i in 1..=self.num_blocks {
            let vi = &self.vindex[i];
            let start = vi[1] as usize;
            let end = vi[2] as usize;
            let material_id = vi[0] as usize;

            let mut verts = Vec::new();
            for j in start..=end {
                if j < frame.vertices.len() {
                    verts.push([frame.vertices[j][1], frame.vertices[j][2]]);
                }
            }
            blocks.push(Block {
                id: i,
                vertices: verts,
                material_id,
            });
        }

        let point_count = self.n_f_points + self.n_l_points + self.n_m_points;
        let mut points = Vec::new();
        for i in 1..=point_count {
            if i < frame.points.len() {
                points.push(PointData {
                    x: frame.points[i][1],
                    y: frame.points[i][2],
                    point_type: frame.points[i][3] as i32,
                });
            }
        }

        let mut bolts = Vec::new();
        for b in &frame.bolts {
            bolts.push(BoltData {
                x1: b[0],
                y1: b[1],
                x2: b[2],
                y2: b[3],
            });
        }

        // No joint data in replay files — joints are pre-cut geometry
        let joints = Vec::new();

        Some(SceneData {
            blocks,
            joints,
            points,
            bolts,
            bbox,
        })
    }
}

/// Parse a `.replay` file into a `ReplayData`.
pub fn parse_replay_file(path: &Path) -> Result<ReplayData, String> {
    let file =
        std::fs::File::open(path).map_err(|e| format!("Cannot open replay file: {}", e))?;
    let reader = BufReader::new(file);
    let mut lines = reader.lines();

    // Line 1: numstepssaved nBlocks nBolts vertexCount
    let line1 = next_line(&mut lines)?;
    let h1 = parse_ints(&line1)?;
    if h1.len() < 4 {
        return Err("Replay header line 1: expected 4 integers".into());
    }
    let num_steps_saved = h1[0] as usize;
    let num_blocks = h1[1] as usize;
    let num_bolts = h1[2] as usize;
    let vertex_count = h1[3] as usize;

    // Line 2: nFPoints nLPoints nMPoints
    let line2 = next_line(&mut lines)?;
    let h2 = parse_ints(&line2)?;
    if h2.len() < 3 {
        return Err("Replay header line 2: expected 3 integers".into());
    }
    let n_f_points = h2[0] as usize;
    let n_l_points = h2[1] as usize;
    let n_m_points = h2[2] as usize;
    let point_count = n_f_points + n_l_points + n_m_points;

    // vindex: num_blocks+1 rows, each with 3 ints (material, start, end)
    let mut vindex = Vec::with_capacity(num_blocks + 1);
    for _ in 0..=num_blocks {
        let line = next_line(&mut lines)?;
        let vals = parse_ints(&line)?;
        if vals.len() < 3 {
            return Err("Replay vindex: expected 3 integers per row".into());
        }
        vindex.push([vals[0], vals[1], vals[2]]);
    }

    // Per-timestep data
    let mut frames = Vec::with_capacity(num_steps_saved);
    for _ in 0..num_steps_saved {
        // Vertices: vertex_count+1 rows (0..=vertex_count), each "x y material"
        // Actually the C writes 0..=k0[numblocks][2]+4 which equals vertex_count
        let mut vertices = Vec::with_capacity(vertex_count + 1);
        for _ in 0..=vertex_count {
            let line = next_line(&mut lines)?;
            let vals = parse_floats(&line)?;
            if vals.len() < 3 {
                return Err("Replay vertex: expected 3 floats".into());
            }
            vertices.push([vals[0], vals[1], vals[2]]);
        }

        // Bolts: num_bolts rows, each 9 values
        let mut bolts = Vec::with_capacity(num_bolts);
        for _ in 0..num_bolts {
            let line = next_line(&mut lines)?;
            let vals = parse_floats(&line)?;
            if vals.len() < 9 {
                return Err(format!(
                    "Replay bolt: expected 9 values, got {}",
                    vals.len()
                ));
            }
            let mut arr = [0.0; 9];
            arr.copy_from_slice(&vals[..9]);
            bolts.push(arr);
        }

        // Points: point_count+1 rows (0..=pointCount), each 4 values
        let mut points = Vec::with_capacity(point_count + 1);
        for _ in 0..=point_count {
            let line = next_line(&mut lines)?;
            let vals = parse_floats(&line)?;
            if vals.len() < 4 {
                return Err("Replay point: expected 4 floats".into());
            }
            points.push([vals[0], vals[1], vals[2], vals[3]]);
        }

        frames.push(ReplayFrame {
            vertices,
            bolts,
            points,
        });
    }

    eprintln!(
        "[ntdda] parsed replay: {} frames, {} blocks, {} bolts, {} vertices",
        frames.len(),
        num_blocks,
        num_bolts,
        vertex_count
    );

    Ok(ReplayData {
        num_blocks,
        num_bolts,
        n_f_points,
        n_l_points,
        n_m_points,
        vindex,
        frames,
    })
}

fn next_line(lines: &mut std::io::Lines<BufReader<std::fs::File>>) -> Result<String, String> {
    lines
        .next()
        .ok_or_else(|| "Unexpected end of replay file".to_string())?
        .map_err(|e| format!("Read error: {}", e))
}

fn parse_ints(line: &str) -> Result<Vec<i32>, String> {
    line.split_whitespace()
        .map(|s| {
            s.parse::<i32>()
                .map_err(|e| format!("Parse int '{}': {}", s, e))
        })
        .collect()
}

fn parse_floats(line: &str) -> Result<Vec<f64>, String> {
    line.split_whitespace()
        .map(|s| {
            s.parse::<f64>()
                .map_err(|e| format!("Parse float '{}': {}", s, e))
        })
        .collect()
}
