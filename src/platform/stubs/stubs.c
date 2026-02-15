/*
 * Stub implementations of statusbar, toolbar, and dda_display_* for non-Win32 builds.
 * These are no-ops (or fprintf to stderr) so the numerical library can link without Win32 GUI code.
 */

#include <stdio.h>
#include "dda.h"
#include "statusbar.h"
#include "toolbar.h"

/* dda_display_* stubs - print to stderr for library builds */
void dda_display_error(const char * message) {
  if (message) fprintf(stderr, "DDA Error: %s\n", message);
}
void dda_display_warning(const char * message) {
  if (message) fprintf(stderr, "DDA Warning: %s\n", message);
}
void dda_display_info(const char * message) {
  if (message) fprintf(stderr, "DDA Info: %s\n", message);
}

/* statusbar stubs */
void statusbar_init(HWND hwMain) { (void)hwMain; }
void statusbar_show(void) {}
int statusbar_get_visibility(void) { return 1; }
void statusbar_set_visibility(int v) { (void)v; }
void statusbar_resize(void) {}
void statusbar_set_state(unsigned int state) { (void)state; }
void statusbar_set_text(WPARAM wParam, LPARAM lParam) { (void)wParam; (void)lParam; }
void statusbar_update_analysis(int numblocks, double elapsedtime, int currtimestep,
                               int numtimesteps, int openclosecount) {
  (void)numblocks; (void)elapsedtime; (void)currtimestep;
  (void)numtimesteps; (void)openclosecount;
}
void statusbar_update_geometry(int numblocks) { (void)numblocks; }
void statusbar_update_progbar(unsigned int timestep) { (void)timestep; }
void statusbar_set_progbar_range(unsigned short value) { (void)value; }

/* toolbar stubs */
void toolbar_init(HWND hwMain) { (void)hwMain; }
void toolbar_show(void) {}
int toolbar_get_visibility(void) { return 1; }
void toolbar_set_visibility(int v) { (void)v; }
void toolbar_resize(void) {}
void toolbar_set_state(unsigned int state) { (void)state; }
