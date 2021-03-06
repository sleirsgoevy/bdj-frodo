/*
 *  C64_x.i - Put the pieces together, X specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *  Unix stuff by Bernd Schmidt/Lutz Vieweg
 */

#include "main.h"


static struct timeval tv_start;

#ifndef HAVE_USLEEP
/*
 *  NAME:
 *      usleep     -- This is the precision timer for Test Set
 *                    Automation. It uses the select(2) system
 *                    call to delay for the desired number of
 *                    micro-seconds. This call returns ZERO
 *                    (which is usually ignored) on successful
 *                    completion, -1 otherwise.
 *
 *  ALGORITHM:
 *      1) We range check the passed in microseconds and log a
 *         warning message if appropriate. We then return without
 *         delay, flagging an error.
 *      2) Load the Seconds and micro-seconds portion of the
 *         interval timer structure.
 *      3) Call select(2) with no file descriptors set, just the
 *         timer, this results in either delaying the proper
 *         ammount of time or being interupted early by a signal.
 *
 *  HISTORY:
 *      Added when the need for a subsecond timer was evident.
 *
 *  AUTHOR:
 *      Michael J. Dyer                   Telephone:   AT&T 414.647.4044
 *      General Electric Medical Systems        GE DialComm  8 *767.4044
 *      P.O. Box 414  Mail Stop 12-27         Sect'y   AT&T 414.647.4584
 *      Milwaukee, Wisconsin  USA 53201                      8 *767.4584
 *      internet:  mike@sherlock.med.ge.com     GEMS WIZARD e-mail: DYER
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>

int usleep(unsigned long int microSeconds)
{
        unsigned int            Seconds, uSec;
        int                     nfds, readfds, writefds, exceptfds;
        struct  timeval         Timer;

        nfds = readfds = writefds = exceptfds = 0;

        if( (microSeconds == (unsigned long) 0)
                || microSeconds > (unsigned long) 4000000 )
        {
                errno = ERANGE;         /* value out of range */
                perror( "usleep time out of range ( 0 -> 4000000 ) " );
                return -1;
        }

        Seconds = microSeconds / (unsigned long) 1000000;
        uSec    = microSeconds % (unsigned long) 1000000;

        Timer.tv_sec            = Seconds;
        Timer.tv_usec           = uSec;

        if( select( nfds, &readfds, &writefds, &exceptfds, &Timer ) < 0 )
        {
                perror( "usleep (select) failed" );
                return -1;
        }

        return 0;
}
#endif


/*
 *  Constructor, system-dependent things
 */

void C64::c64_ctor1(void)
{
	// Initialize joystick variables
	joyfd[0] = joyfd[1] = -1;
	joy_minx = joy_miny = 32767;
	joy_maxx = joy_maxy = -32768;

	// we need to create a potential GUI subprocess here, because we don't want
	// it to inherit file-descriptors (such as for the audio-device and alike..)
#if defined(__svgalib__)
	gui = 0;
#else
	// try to start up Tk gui.
	gui = new CmdPipe("wish", "TkGui.tcl");
	if (gui) {
		if (gui->fail) {
			delete gui; gui = 0;
		}
	}
	// wait until the GUI process responds (if it does...)
	if (gui) {
		if (5 != gui->ewrite("ping\n",5)) {
			delete gui; gui = 0;
		} else {
			char c;
			fd_set set;
			FD_ZERO(&set);
			FD_SET(gui->get_read_fd(), &set);
 			struct timeval tv;
			tv.tv_usec = 0;
			tv.tv_sec = 5;
// Use the following commented line for HP-UX < 10.20
//			if (select(FD_SETSIZE, (int *)&set, (int *)NULL, (int *)NULL, &tv) <= 0) {
			if (select(FD_SETSIZE, &set, NULL, NULL, &tv) <= 0) {
				delete gui; gui = 0;
			} else {
				if (1 != gui->eread(&c, 1)) {
					delete gui; gui = 0;
				} else {
					if (c != 'o') {
						delete gui; gui = 0;
					}
				}
			}
		}
	}
#endif // __svgalib__
}

void C64::c64_ctor2(void)
{
#ifndef  __svgalib__  
   if (!gui) {
   	fprintf(stderr,"Alas, master, no preferences window will be available.\n"
   	               "If you wish to see one, make sure the 'wish' interpreter\n"
   	               "(Tk version >= 4.1) is installed in your path.\n"
   	               "You can still use Frodo, though. Use F10 to quit, \n"
   	               "F11 to cause an NMI and F12 to reset the C64.\n"
   	               "You can change the preferences by editing ~/.frodorc\n");
   }
#endif // SVGAlib
  
	gettimeofday(&tv_start, NULL);
}


/*
 *  Destructor, system-dependent things
 */

void C64::c64_dtor(void)
{
}


/*
 *  Start main emulation thread
 */

void C64::Run(void)
{
	// Reset chips
	TheCPU->Reset();
	TheSID->Reset();
	TheCIA1->Reset();
	TheCIA2->Reset();
	TheCPU1541->Reset();

	// Patch kernal IEC routines
	orig_kernal_1d84 = Kernal[0x1d84];
	orig_kernal_1d85 = Kernal[0x1d85];
	PatchKernal(ThePrefs.FastReset, ThePrefs.Emul1541Proc);

	quit_thyself = false;
	thread_func();
}



/* From dreamcast port */
typedef struct key_seq_item
{
  int kc;
  bool shift;
} key_seq_item_t;

#define MATRIX(a,b) (((a) << 3) | (b))
static key_seq_item_t d_auto_seq[4][18] =
{
  {
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(5, 2), false }, /*L*/
    { MATRIX(4, 6), false }, /*O*/
    { MATRIX(1, 2), false }, /*A*/
    { MATRIX(2, 2), false }, /*D*/
    { MATRIX(7, 4), false }, /* */
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(6, 1), false }, /***/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(3, 3), false }, /*8*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(2, 1), false }, /*R*/
    { MATRIX(3, 6), false }, /*U*/    
    { MATRIX(4, 7), false }, /*N*/
    { MATRIX(0, 1), false }, /*RET*/
  },
  {
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(5, 2), false }, /*L*/
    { MATRIX(4, 6), false }, /*O*/
    { MATRIX(1, 2), false }, /*A*/
    { MATRIX(2, 2), false }, /*D*/
    { MATRIX(7, 4), false }, /* */
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(6, 1), false }, /***/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(4, 0), false }, /*9*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(2, 1), false }, /*R*/
    { MATRIX(3, 6), false }, /*U*/    
    { MATRIX(4, 7), false }, /*N*/
    { MATRIX(0, 1), false }, /*RET*/
  },
  {
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(5, 2), false }, /*L*/
    { MATRIX(4, 6), false }, /*O*/
    { MATRIX(1, 2), false }, /*A*/
    { MATRIX(2, 2), false }, /*D*/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(6, 1), false }, /***/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(4, 3), false }, /*0*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(2, 1), false }, /*R*/
    { MATRIX(3, 6), false }, /*U*/    
    { MATRIX(4, 7), false }, /*N*/
    { MATRIX(0, 1), false }, /*RET*/
  },
  {
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(5, 2), false }, /*L*/
    { MATRIX(4, 6), false }, /*O*/
    { MATRIX(1, 2), false }, /*A*/
    { MATRIX(2, 2), false }, /*D*/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(6, 1), false }, /***/
    { MATRIX(7, 3), true  }, /*"*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(5, 7), false }, /*,*/
    { MATRIX(7, 0), false }, /*1*/
    { MATRIX(0, 1), false }, /*RET*/
    { MATRIX(2, 1), false }, /*R*/
    { MATRIX(3, 6), false }, /*U*/    
    { MATRIX(4, 7), false }, /*N*/
    { MATRIX(0, 1), false }, /*RET*/
  },
};


/*
 *  Vertical blank: Poll keyboard and joysticks, update window
 */
int autostart = 0;
int autostart_index = 0;
int autostart_keytime = 5;
void C64::VBlank(bool draw_frame)
{
	// Poll keyboard
	TheDisplay->PollKeyboard(TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);
	if (TheDisplay->quit_requested)
		quit_thyself = true;

        if (autostart == 1)
        {
                key_seq_item_t cur = d_auto_seq[0][autostart_index];

                TheDisplay->FakeKeyPress(cur.kc, cur.shift, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);

                autostart_keytime --;
                if (autostart_keytime == 0)
                {
                        autostart_keytime = 1;
                        autostart_index ++;

                        if (autostart_index == 18)
                        {
                                autostart = 0;
                                autostart_index = 0;
                                autostart_keytime = 5;
                        }
                }
        }

	// Poll joysticks
	TheCIA1->Joystick1 = poll_joystick(0);
	TheCIA1->Joystick2 = poll_joystick(1);

	if (ThePrefs.JoystickSwap) {
		uint8 tmp = TheCIA1->Joystick1;
		TheCIA1->Joystick1 = TheCIA1->Joystick2;
		TheCIA1->Joystick2 = tmp;
	}

	// Joystick keyboard emulation
	if (TheDisplay->NumLock())
		TheCIA1->Joystick1 &= joykey;
	else
		TheCIA1->Joystick2 &= joykey;
       
	// Count TOD clocks
	TheCIA1->CountTOD();
	TheCIA2->CountTOD();

	// Update window if needed
	if (draw_frame) {
    	TheDisplay->Update();

		// Calculate time between VBlanks, display speedometer
		struct timeval tv;
		gettimeofday(&tv, NULL);
		if ((tv.tv_usec -= tv_start.tv_usec) < 0) {
			tv.tv_usec += 1000000;
			tv.tv_sec -= 1;
		}
		tv.tv_sec -= tv_start.tv_sec;
		double elapsed_time = (double)tv.tv_sec * 1000000 + tv.tv_usec;
		speed_index = 20000 / (elapsed_time + 1) * ThePrefs.SkipFrames * 100;

		// Limit speed to 100% if desired
		if ((speed_index > 100) && ThePrefs.LimitSpeed) {
			usleep((unsigned long)(ThePrefs.SkipFrames * 20000 - elapsed_time));
			speed_index = 100;
		}

		gettimeofday(&tv_start, NULL);

		TheDisplay->Speedometer((int)speed_index);
	}
}


/*
 *  Open/close joystick drivers given old and new state of
 *  joystick preferences
 */

void C64::open_close_joysticks(bool oldjoy1, bool oldjoy2, bool newjoy1, bool newjoy2)
{
#ifdef HAVE_LINUX_JOYSTICK_H
	if (oldjoy1 != newjoy1) {
		joy_minx = joy_miny = 32767;	// Reset calibration
		joy_maxx = joy_maxy = -32768;
		if (newjoy1) {
			joyfd[0] = open("/dev/js0", O_RDONLY);
			if (joyfd[0] < 0)
				fprintf(stderr, "Couldn't open joystick 1\n");
		} else {
			close(joyfd[0]);
			joyfd[0] = -1;
		}
	}

	if (oldjoy2 != newjoy2) {
		joy_minx = joy_miny = 32767;	// Reset calibration
		joy_maxx = joy_maxy = -32768;
		if (newjoy2) {
			joyfd[1] = open("/dev/js1", O_RDONLY);
			if (joyfd[1] < 0)
				fprintf(stderr, "Couldn't open joystick 2\n");
		} else {
			close(joyfd[1]);
			joyfd[1] = -1;
		}
	}
#endif
}


/*
 *  Poll joystick port, return CIA mask
 */

uint8 C64::poll_joystick(int port)
{
#ifdef HAVE_LINUX_JOYSTICK_H
	JS_DATA_TYPE js;
	uint8 j = 0xff;

	if (joyfd[port] >= 0) {
		if (read(joyfd[port], &js, JS_RETURN) == JS_RETURN) {
			if (js.x > joy_maxx)
				joy_maxx = js.x;
			if (js.x < joy_minx)
				joy_minx = js.x;
			if (js.y > joy_maxy)
				joy_maxy = js.y;
			if (js.y < joy_miny)
				joy_miny = js.y;

			if (joy_maxx-joy_minx < 100 || joy_maxy-joy_miny < 100)
				return 0xff;

			if (js.x < (joy_minx + (joy_maxx-joy_minx)/3))
				j &= 0xfb;							// Left
			else if (js.x > (joy_minx + 2*(joy_maxx-joy_minx)/3))
				j &= 0xf7;							// Right

			if (js.y < (joy_miny + (joy_maxy-joy_miny)/3))
				j &= 0xfe;							// Up
			else if (js.y > (joy_miny + 2*(joy_maxy-joy_miny)/3))
				j &= 0xfd;							// Down

			if (js.buttons & 1)
				j &= 0xef;							// Button
		}
	}
	return j;
#else
	return 0xff;
#endif
}


/*
 * The emulation's main loop
 */

void C64::thread_func(void)
{
	int linecnt = 0;

#ifdef FRODO_SC
	while (!quit_thyself) {

		// The order of calls is important here
		if (TheVIC->EmulateCycle())
			TheSID->EmulateLine();
		TheCIA1->CheckIRQs();
		TheCIA2->CheckIRQs();
		TheCIA1->EmulateCycle();
		TheCIA2->EmulateCycle();
		TheCPU->EmulateCycle();

		if (ThePrefs.Emul1541Proc) {
			TheCPU1541->CountVIATimers(1);
			if (!TheCPU1541->Idle)
				TheCPU1541->EmulateCycle();
		}
		CycleCounter++;
#else
	while (!quit_thyself) {

		// The order of calls is important here
		int cycles = TheVIC->EmulateLine();
		TheSID->EmulateLine();
#if !PRECISE_CIA_CYCLES
		TheCIA1->EmulateLine(ThePrefs.CIACycles);
		TheCIA2->EmulateLine(ThePrefs.CIACycles);
#endif

		if (ThePrefs.Emul1541Proc) {
			int cycles_1541 = ThePrefs.FloppyCycles;
			TheCPU1541->CountVIATimers(cycles_1541);

			if (!TheCPU1541->Idle) {
				// 1541 processor active, alternately execute
				//  6502 and 6510 instructions until both have
				//  used up their cycles
				while (cycles >= 0 || cycles_1541 >= 0)
					if (cycles > cycles_1541)
						cycles -= TheCPU->EmulateLine(1);
					else
						cycles_1541 -= TheCPU1541->EmulateLine(1);
			} else
				TheCPU->EmulateLine(cycles);
		} else
			// 1541 processor disabled, only emulate 6510
			TheCPU->EmulateLine(cycles);
#endif
		linecnt++;
#if !defined(__svgalib__)
		if ((linecnt & 0xfff) == 0 && gui) {

			// check for command from GUI process
		// fprintf(stderr,":");
			while (gui->probe()) {
				char c;
				if (gui->eread(&c, 1) != 1) {
					delete gui;
					gui = 0;
					fprintf(stderr,"Oops, GUI process died...\n");
				} else {
               // fprintf(stderr,"%c",c);
					switch (c) {
						case 'q':
							quit_thyself = true;
							break;
						case 'r':
							Reset();
							break;
						case 'p':{
							Prefs *np = Frodo::reload_prefs();
							NewPrefs(np);
							ThePrefs = *np;
							break;
						}
						default:
							break;
					}
				}
			}
		}
#endif
	}
#if !defined(__svgalib__)
	if (gui) {
		gui->ewrite("quit\n",5);
	}
#endif
}
