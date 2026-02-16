# DDA Example Files

Example geometry (`.geo`) and analysis (`.ana`) files for testing ntdda.

## File Formats

DDA supports two geometry/analysis file formats:

- **Original format** — plain text, no XML. The file type is detected by the absence of an XML header.
- **DDAML (XML) format** — XML with `Berkeley:DDA` namespace. Detected by the XML declaration on the first line.

## Examples

### circleslope

Original (non-XML) format. A circular slope geometry with 18 arc segments, 7 boundary lines, 5 interior joints, and 3 fixed points. Coordinates range approximately [-1.5, 1.3]. Paired with an analysis file for 20 timesteps with 3 materials and gravity.

- `circleslope.geo` — geometry
- `circleslope.ana` — analysis (20 timesteps, gravity)

### loadpoint/pushblock

DDAML (XML) format. A rectangular block (10-50, 10-20) with a smaller block (15-20, 20-25) sitting on top. Includes 4 fixed points and 1 load point. The analysis applies a time-dependent horizontal force to push the small block.

- `loadpoint/pushblock.geo` — geometry (XML)
- `loadpoint/pushblock.ana` — analysis (500 timesteps, dynamic, load point with time-dependent forces)

### fixedlines/

DDAML (XML) format. Eight test cases (`case1.geo` through `case8.geo`) for fixed-line boundary conditions. Each is a simple unit square (0-1, 0-1) with varying fixed line configurations. Minimal test cases for validating fixed point generation from fixed lines.
