#pragma once
#include <cibyl-syscall_defs.h>
#include <java/lang.h>
#include <java/io.h>

/* GUI */

// forwards to DVBBufferedImage::setRGB(int, int, int, int, int[], int, int)
void NOPH_MyXlet_blitFramebuffer(int x, int y, int w, int h, int* ptr, int scansize);

// calls scene.repaint()
void NOPH_MyXlet_repaint(void);

/* Check for key events
   Returns:
     -keyCode on KEY_RELEASED
     +keyCode on KEY_PRESSED
     0 if there are no pending events
*/
int NOPH_MyXlet_pollInput(void);

/* stdio */

// returns MyXlet.class
NOPH_Class_t NOPH_MyXlet_getclass(void);

// returns an OutputStream for printing to the screen
NOPH_OutputStream_t NOPH_MyXlet_getStdout(void);

// get the VFS root, prefixed with 'file://'
int NOPH_MyXlet_strlenVFSRoot(int which); /* Throws */
void NOPH_MyXlet_getVFSRoot(int which, void* buf); /* Throws */

// listdir
int NOPH_MyXlet_pathNEntries(const char* path);
NOPH_String_t NOPH_MyXlet_pathGetEntry(const char* path, int idx);

// remove last line on console
void NOPH_MyXlet_popMessage(void);

// switch from GUI to console mode
void NOPH_MyXlet_closeGUI(void);
