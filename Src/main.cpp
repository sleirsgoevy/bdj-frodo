/*
 *  main.cpp - Main program
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#include "sysdeps.h"

#include "main.h"
#include "C64.h"
#include "Display.h"
#include "Prefs.h"
#include "SAM.h"


// Global variables
char AppDirPath[1024];	// Path of application directory


// ROM file names
#ifdef __riscos__
#define BASIC_ROM_FILE	"FrodoRsrc:Basic_ROM"
#define KERNAL_ROM_FILE	"FrodoRsrc:Kernal_ROM"
#define CHAR_ROM_FILE	"FrodoRsrc:Char_ROM"
#define FLOPPY_ROM_FILE	"FrodoRsrc:1541_ROM"
#elif CIBYL
/* Placed as resources */
#define BASIC_ROM_FILE	"/Basic_ROM"
#define KERNAL_ROM_FILE	"/Kernal_ROM"
#define CHAR_ROM_FILE	"/Char_ROM"
#define FLOPPY_ROM_FILE	"/1541_ROM"
#else
#define BASIC_ROM_FILE	"Basic ROM"
#define KERNAL_ROM_FILE	"Kernal ROM"
#define CHAR_ROM_FILE	"Char ROM"
#define FLOPPY_ROM_FILE	"1541 ROM"
#endif


/*
 *  Load C64 ROM files
 */

#ifndef __PSXOS__

static inline char* do_concat(char* a, char* b)
{
    int ll = 0;
    for(int i = 0; a[i]; i++)
        ll++;
    for(int i = 0; b[i]; i++)
        ll++;
    char* ans = (char*)malloc(ll+1);
    int idx = 0;
    for(int i = 0; a[i]; i++)
        ans[idx++] = a[i];
    for(int i = 0; b[i]; i++)
        ans[idx++] = b[i];
    ans[idx++] = 0;
    return ans;
}

extern char* fs_root;

#ifndef CIBYL
#define do_concat(a, b) b
#define free(x)
#endif

bool Frodo::load_rom_files(void)
{
	FILE *file;
        char* path;

	// Load Basic ROM
        path = do_concat(fs_root, BASIC_ROM_FILE);
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(TheC64->Basic, 1, 0x2000, file) != 0x2000) {
			ShowRequester("Can't read 'Basic ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Basic ROM'.", "Quit");
		return false;
	}
        free(path);

	// Load Kernal ROM
        path = do_concat(fs_root, KERNAL_ROM_FILE);
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(TheC64->Kernal, 1, 0x2000, file) != 0x2000) {
			ShowRequester("Can't read 'Kernal ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Kernal ROM'.", "Quit");
		return false;
	}
        free(path);

	// Load Char ROM
        path = do_concat(fs_root, CHAR_ROM_FILE);
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(TheC64->Char, 1, 0x1000, file) != 0x1000) {
			ShowRequester("Can't read 'Char ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find 'Char ROM'.", "Quit");
		return false;
	}
        free(path);

	// Load 1541 ROM
        path = do_concat(fs_root, FLOPPY_ROM_FILE);
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(TheC64->ROM1541, 1, 0x4000, file) != 0x4000) {
			ShowRequester("Can't read '1541 ROM'.", "Quit");
			return false;
		}
		fclose(file);
	} else {
		ShowRequester("Can't find '1541 ROM'.", "Quit");
		return false;
	}
        free(path);

	return true;
}

#ifndef CIBYL
#undef do_concat
#undef free
#endif

#endif


#ifdef __BEOS__
#include "main_Be.i"
#endif

#ifdef AMIGA
#include "main_Amiga.i"
#endif


#ifdef CIBYL
#include "main_cibyl.i"
#elif __unix
#include "main_x.i"
#endif

#ifdef __mac__
#include "main_mac.i"
#endif

#ifdef WIN32
#include "main_WIN32.i"
#endif

#ifdef __riscos__
#include "main_Acorn.i"
#endif

#ifdef __PSXOS__
#include "main_PSX.i"
#endif
