
/* 
 * filebrowse.c
 * 
 * Function provides a common file broswer interface for dealing 
 * with variable types of input and output files in DDA.  This 
 * set of code was duplicated 4 times in the main dda.c file.
 * It is also being used to extend the capabilities of dda.  For 
 * instance, any valid results file generated from any dda source,
 * can be animated.  Results files from previous problems can be 
 * saved, so that it won't be necessary to rerun the problem to 
 * view that results.
 * $Author: doolin $
 * $Date: 2002/05/26 23:47:24 $
 * $Source: /cvsroot/dda/ntdda/src/filebrowse.c,v $
 * $Revision: 1.2 $
 *
 */
#define STRICT
#include"windows.h"
#include"dda.h"

/*  The char * parameters will eventually be passed in through 
 *  the struct that handles io names (FILEPATHS, see dda.h.  This 
 *  struct is currently global, so the file names have to passed in.
 *  Most of what this function does is wrap initialization of an 
 *  OPENFILENAME win32 struct.
 */

void fileBrowse(HWND hwMain,
				OPENFILENAME *ofn, LPCTSTR *szFilter, 
				char *newpath, char *newfile, char *suffix) {

    /*  Being the only extern suggests 
	 *  passing this in as a parameter. 
	 */
	extern FILEPATHS filepath;

	strcpy(filepath.oldpath, newpath);
	newpath[0] = '\0';
	memset( ofn, 0, sizeof(OPENFILENAME) );
	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = hwMain;
	ofn->lpstrFilter = szFilter[0];
	ofn->lpstrFile = newpath;
	ofn->nMaxFile = 256;
	/* The mistake is the next line; newpath != newfile... */
	ofn->lpstrFileTitle = (LPTSTR)newfile;
	ofn->lpstrDefExt = suffix;
	ofn->nMaxFileTitle = 256;
	ofn->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
} 



