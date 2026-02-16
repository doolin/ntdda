/*
 * C probe for validating Rust #[repr(C)] struct mirrors.
 * Compiled and linked by build.rs. Called from Rust tests
 * to verify sizeof and offsetof match exactly.
 */
#include <stddef.h>
#include "dda.h"

/* Geometrydata probes */
size_t probe_sizeof_geometrydata(void) { return sizeof(Geometrydata); }
size_t probe_offsetof_gd_display_warning(void) { return offsetof(Geometrydata, display_warning); }
size_t probe_offsetof_gd_display_error(void) { return offsetof(Geometrydata, display_error); }
size_t probe_offsetof_gd_index(void) { return offsetof(Geometrydata, index); }
size_t probe_offsetof_gd_blocksize(void) { return offsetof(Geometrydata, blocksize); }
size_t probe_offsetof_gd_e00(void) { return offsetof(Geometrydata, e00); }
size_t probe_offsetof_gd_nJoints(void) { return offsetof(Geometrydata, nJoints); }
size_t probe_offsetof_gd_nBolts(void) { return offsetof(Geometrydata, nBolts); }
size_t probe_offsetof_gd_nFPoints(void) { return offsetof(Geometrydata, nFPoints); }
size_t probe_offsetof_gd_nLPoints(void) { return offsetof(Geometrydata, nLPoints); }
size_t probe_offsetof_gd_nMPoints(void) { return offsetof(Geometrydata, nMPoints); }
size_t probe_offsetof_gd_nHPoints(void) { return offsetof(Geometrydata, nHPoints); }
size_t probe_offsetof_gd_nBlocks(void) { return offsetof(Geometrydata, nBlocks); }
size_t probe_offsetof_gd_vertexCount(void) { return offsetof(Geometrydata, vertexCount); }
size_t probe_offsetof_gd_scale(void) { return offsetof(Geometrydata, scale); }
size_t probe_offsetof_gd_w0(void) { return offsetof(Geometrydata, w0); }
size_t probe_offsetof_gd_pointsize1(void) { return offsetof(Geometrydata, pointsize1); }
size_t probe_offsetof_gd_pointsize2(void) { return offsetof(Geometrydata, pointsize2); }
size_t probe_offsetof_gd_points(void) { return offsetof(Geometrydata, points); }
size_t probe_offsetof_gd_jointsize1(void) { return offsetof(Geometrydata, jointsize1); }
size_t probe_offsetof_gd_joints(void) { return offsetof(Geometrydata, joints); }
size_t probe_offsetof_gd_rockboltsize1(void) { return offsetof(Geometrydata, rockboltsize1); }
size_t probe_offsetof_gd_rockbolts(void) { return offsetof(Geometrydata, rockbolts); }
size_t probe_offsetof_gd_vertexsize1(void) { return offsetof(Geometrydata, vertexsize1); }
size_t probe_offsetof_gd_vertices(void) { return offsetof(Geometrydata, vertices); }
size_t probe_offsetof_gd_vindexsize1(void) { return offsetof(Geometrydata, vindexsize1); }
size_t probe_offsetof_gd_vindex(void) { return offsetof(Geometrydata, vindex); }
size_t probe_offsetof_gd_pointCount(void) { return offsetof(Geometrydata, pointCount); }
size_t probe_offsetof_gd_nPoints(void) { return offsetof(Geometrydata, nPoints); }

/* Analysisdata probes */
size_t probe_sizeof_analysisdata(void) { return sizeof(Analysisdata); }
size_t probe_offsetof_ad_options(void) { return offsetof(Analysisdata, options); }
size_t probe_offsetof_ad_isRunning(void) { return offsetof(Analysisdata, isRunning); }
size_t probe_offsetof_ad_nTimeSteps(void) { return offsetof(Analysisdata, nTimeSteps); }
size_t probe_offsetof_ad_nCurrentContacts(void) { return offsetof(Analysisdata, nCurrentContacts); }
size_t probe_offsetof_ad_m9(void) { return offsetof(Analysisdata, m9); }
size_t probe_offsetof_ad_OCConverged(void) { return offsetof(Analysisdata, OCConverged); }
size_t probe_offsetof_ad_cts(void) { return offsetof(Analysisdata, cts); }
size_t probe_offsetof_ad_elapsedTime(void) { return offsetof(Analysisdata, elapsedTime); }
size_t probe_offsetof_ad_delta_t(void) { return offsetof(Analysisdata, delta_t); }

/* Filepaths probes */
size_t probe_sizeof_filepaths(void) { return sizeof(Filepaths); }
size_t probe_offsetof_fp_gfile(void) { return offsetof(Filepaths, gfile); }
size_t probe_offsetof_fp_afile(void) { return offsetof(Filepaths, afile); }
