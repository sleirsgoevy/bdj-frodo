/*********************************************************************
 *
 * Copyright (C) 2008,  Simon Kagstrom
 *
 * Filename:      cibyl-menus.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:   Cibyl menu item impl.
 *
 * $Id:$
 *
 ********************************************************************/
#include "Prefs.h"

//#include <javax/microedition/lcdui.h>
//#include <javax/microedition/io.h>
#include <org/homebrew.h>
#include <java/util.h>
#include <stdlib.h>
//#include "command_mgr.h"


//static NOPH_List_t fs_list;
static char roots[5][40];
char* fs_root = NULL;
char* game_root = NULL;

/*static void select_fs_callback(void *unused)
{
  int nr = NOPH_List_getSelectedIndex(fs_list);

  fs_root = roots[nr];
}*/

const char *cibyl_select_fs_root(void)
{
#if 0
	NOPH_Display_t display = NOPH_Display_getDisplay(NOPH_MIDlet_get());
	NOPH_Displayable_t cur = NOPH_Display_getCurrent(display);
	//NOPH_CommandMgr_t cm = NOPH_CommandMgr_getInstance();
	NOPH_Enumeration_t en = NOPH_FileSystemRegistry_listRoots();
	FILE *conf;
	int i = 0;

	fs_root = NULL;

	fs_list = NOPH_List_new("Select fs root", NOPH_Choice_IMPLICIT);

	while (NOPH_Enumeration_hasMoreElements(en))
	{
		NOPH_Object_t o = NOPH_Enumeration_nextElement(en);

		NOPH_String_toCharPtr(o, roots[i], 40);
		NOPH_List_append(fs_list, roots[i], 0);
		NOPH_delete(o);
		i++;
	}
	NOPH_delete(en);
	NOPH_Display_setCurrent(display, fs_list);
	//NOPH_CommandMgr_setList(cm, fs_list, select_fs_callback, NULL);

	while(fs_root == NULL)
	{
		NOPH_Thread_sleep(250);
	}
#if 0
	conf = fopen("recordstore://sarien-conf:1", "w");
	if (conf)
	{
		char buf[40];

		strncpy(buf, fs_root, 40);
		fwrite(buf, 1, 40, conf);
		fclose(conf);
	}
#endif
	NOPH_Display_setCurrent(display, cur);
	NOPH_delete(fs_list);
#endif
    char* ans = (char*)malloc(NOPH_MyXlet_strlenVFSRoot(1)+1);
    NOPH_MyXlet_getVFSRoot(1, ans);
    return ans;
}

void cibyl_set_fs_root(char *fsr)
{
    fs_root = fsr;
    if(game_root)
        free(game_root);
    int i;
    for(i = 0; fs_root[i]; i++);
    game_root = (char*)malloc(i+7);
    for(i = 0; fs_root[i]; i++)
        game_root[i] = fs_root[i];
    for(int j = 0; j <= 7; j++)
        game_root[i++] = "/games\0"[j];
}


//static NOPH_List_t game_list;
static char *selected_game = NULL;

static char *get_current_directory(void)
{
        //static char root[40];
        //snprintf(root, 40, "file:///%sfrodo", fs_root);
        return fs_root;
}

static char *get_game_directory(const char *game)
{
        static char root[512];
        snprintf(root, 512, "%s/%s", game_root, game);
        return root;
}


static char **read_directory(char *base_dir)
{
    //functions from string.h seem to be buggy on Cibyl
    if(base_dir[0] == 'f' && base_dir[1] == 'i' && base_dir[2] == 'l' && base_dir[3] == 'e' && base_dir[4] == ':' && base_dir[5] == '/' && base_dir[6] == '/')
        base_dir += 7;
    int nentries = NOPH_MyXlet_pathNEntries(base_dir);
    if(nentries < 0)
        return NULL;
    char** ans = (char**)calloc(nentries+1, sizeof(char*));
    for(int i = 0; i < nentries; i++)
    {
        NOPH_String_t s = NOPH_MyXlet_pathGetEntry(base_dir, i);
        if(!s)
        {
            for(int j = 0; j < i; j++)
                free(ans[j]);
            free(ans);
            return NULL;
        }
        int l = 1;
        char* p = (char*)malloc(1);
        while(NOPH_String_toCharPtr(s, p, l) >= l - 1)
        {
            free(p);
            l *= 2;
            p = (char*)malloc(l);
        }
        ans[i] = p;
    }
    return ans;
}

char *cibyl_select_game(char *base_dir)
{
    char** all_games = read_directory(game_root);
    if(!all_games)
        return NULL;
    char** backp = all_games;
    for(char** fwdp = all_games; *fwdp; fwdp++)
    {
        int i;
        for(i = 0; fwdp[0][i]; i++);
        if(i >= 4 && fwdp[0][i-4] == '.' && (fwdp[0][i-3] == 'd' || fwdp[0][i-3] == 't') && fwdp[0][i-2] == '6' && fwdp[0][i-1] == '4')
            *backp++ = *fwdp;
        else
            free(*fwdp);
    }
    *backp = NULL;
    if(!*all_games)
    {
        printf("No games found!\n");
        return NULL;
    }
    int cur = 0;
    printf("Select a game:\n");
    char* ans = NULL;
    while(!ans)
    {
        for(int i = 0; all_games[i]; i++)
            printf((i==cur?"%s *\n":"%s\n"), all_games[i]);
        printf(all_games[cur]?"BASIC\n":"BASIC *\n");
        for(;;)
        {
            int key = NOPH_MyXlet_pollInput();
            if(key == 38 && cur)
            {
                cur--;
                break;
            }
            else if(key == 40 && all_games[cur])
            {
                cur++;
                break;
            }
            else if(key == 10)
            {
                if(!all_games[cur]) // BASIC
                    ans = (char*)1;
                else
                    ans = get_game_directory(all_games[cur]);
                break;
            }
        }
        for(int i = 0; all_games[i]; i++)
            NOPH_MyXlet_popMessage();
        NOPH_MyXlet_popMessage();
    }
    NOPH_MyXlet_popMessage();
    NOPH_MyXlet_popMessage();
    if(ans == (char*)1)
        ans = NULL;
    printf("\nimage path = %s\n", ans);
    for(int i = 0; all_games[i]; i++)
        free(all_games[i]);
    free(all_games);
    return ans;
}


typedef struct
{
	int selected;
	//NOPH_List_t menu_list;
} mm_args_t;

typedef struct key_seq_item
{
  int kc;
  bool shift;
} key_seq_item_t;
#define MATRIX(a,b) (((a) << 3) | (b))

extern int autostart;
extern int autostart_type;
extern key_seq_item_t game_b_key;

static void input_menu_callback(void *p)
{
	mm_args_t *a = (mm_args_t*)p;
        int nr = -1;//NOPH_List_getSelectedIndex(a->menu_list);
        key_seq_item_t items[] = {
                (key_seq_item_t){ MATRIX(0, 4), false },
        	(key_seq_item_t){ MATRIX(0, 5), false },
                (key_seq_item_t){ MATRIX(0, 6), false },
                (key_seq_item_t){ MATRIX(0, 3), false },
                (key_seq_item_t){ MATRIX(4, 3), false },
                (key_seq_item_t){ MATRIX(7, 0), false },
                (key_seq_item_t){ MATRIX(7, 3), false },
        	(key_seq_item_t){ MATRIX(1, 0), false },
        	(key_seq_item_t){ MATRIX(1, 3), false },
        	(key_seq_item_t){ MATRIX(2, 0), false },
        	(key_seq_item_t){ MATRIX(2, 3), false },
        	(key_seq_item_t){ MATRIX(3, 0), false },
        	(key_seq_item_t){ MATRIX(3, 3), false },
        	(key_seq_item_t){ MATRIX(4, 0), false },
        	(key_seq_item_t){ MATRIX(1, 2), false },
        	(key_seq_item_t){ MATRIX(3, 4), false },
        	(key_seq_item_t){ MATRIX(2, 4), false },
        	(key_seq_item_t){ MATRIX(2, 2), false },
        	(key_seq_item_t){ MATRIX(1, 6), false },
        	(key_seq_item_t){ MATRIX(2, 5), false },
        	(key_seq_item_t){ MATRIX(3, 2), false },
        	(key_seq_item_t){ MATRIX(3, 5), false },
        	(key_seq_item_t){ MATRIX(4, 1), false },
        	(key_seq_item_t){ MATRIX(4, 2), false },
        	(key_seq_item_t){ MATRIX(4, 5), false },
        	(key_seq_item_t){ MATRIX(5, 2), false },
        	(key_seq_item_t){ MATRIX(4, 4), false },
        	(key_seq_item_t){ MATRIX(4, 7), false },
        	(key_seq_item_t){ MATRIX(4, 6), false },
        	(key_seq_item_t){ MATRIX(5, 1), false },
        	(key_seq_item_t){ MATRIX(7, 6), false },
        	(key_seq_item_t){ MATRIX(2, 1), false },
        	(key_seq_item_t){ MATRIX(1, 5), false },
        	(key_seq_item_t){ MATRIX(2, 6), false },
        	(key_seq_item_t){ MATRIX(3, 6), false },
        	(key_seq_item_t){ MATRIX(3, 7), false },
        	(key_seq_item_t){ MATRIX(1, 1), false },
        	(key_seq_item_t){ MATRIX(2, 7), false },
        	(key_seq_item_t){ MATRIX(3, 1), false },
                (key_seq_item_t){ MATRIX(1, 4), false },
        };

        game_b_key = items[nr];

        a->selected = 1;
}

void input_menu(void)
{
	//NOPH_List_t input_menu_list;
        //NOPH_Display_t display = NOPH_Display_getDisplay(NOPH_MIDlet_get());
        //NOPH_Displayable_t cur = NOPH_Display_getCurrent(display);
        //NOPH_CommandMgr_t cm = NOPH_CommandMgr_getInstance();
        mm_args_t args;
        int i;
        const char *list[] = { "F1", "F3", "F5", "F7",
                               "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
                               "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                               "N", "O", "P", "Q", "R", "S", "T", "U", "V", "X", "Y", "Z" };

        //input_menu_list = NOPH_List_new("Choose option", NOPH_Choice_IMPLICIT);
        //NOPH_Display_setCurrent(display, input_menu_list);

        for (i = 0; i < sizeof(list) / sizeof(const char *); i++)
                /*NOPH_List_append(input_menu_list, list[i], 0)*/;


        /* Setup the callback args */
        args.selected = 0;
        //args.menu_list = input_menu_list;
        //NOPH_CommandMgr_setList(cm, input_menu_list, input_menu_callback, &args);

        while(args.selected == 0)
        {
                NOPH_Thread_sleep(250);
        }

        //NOPH_Display_setCurrent(display, cur);
        //NOPH_delete(input_menu_list);
}

static void main_menu_callback(void *p)
{
	mm_args_t *a = (mm_args_t*)p;
        int nr = -1;//NOPH_List_getSelectedIndex(a->menu_list);

        switch(nr)
        {
        case 0:
        	ThePrefs.JoystickSwap = !ThePrefs.JoystickSwap;
        	break;
        case 1:
		/* Space */
        	game_b_key = (key_seq_item_t){ MATRIX(7, 4), false };
        	break;
        case 2:
		/* Run/stop */
        	game_b_key = (key_seq_item_t){ MATRIX(7, 7), false };
        	break;
        case 3:
		/* Return */
        	game_b_key = (key_seq_item_t){ MATRIX(0, 1), false };
        	break;
	case 4:
		a->selected = 2;
		return;
        case 5:
        	autostart = 1;
        	autostart_type = 0;
        	break;
        case 6:
        	autostart = 1;
        	autostart_type = 2;
        	break;
        default:
        	break;
        }

        a->selected = 1;
}

extern unsigned char keyboard_cfg[512];
void cibyl_read_keyboard_cfg(void)
{
    int l = NOPH_MyXlet_strlenVFSRoot(0);
    char* cfg = (char*)malloc(l+11);
    NOPH_MyXlet_getVFSRoot(0, cfg);
    for(int i = 0; i <= 10; i++)
        cfg[l+i] = "/frodo.cfg\0"[i];
    FILE* f = fopen(cfg, "r");
    free(cfg);
    if(!f)
    {
        keyboard_cfg[37] = 0102 + 1;
        keyboard_cfg[38] = 0100 + 1;
        keyboard_cfg[39] = 0103 + 1;
        keyboard_cfg[40] = 0101 + 1;
        keyboard_cfg[10] = 0104 + 1;
    }
    else
    {
        int sz = 0;
        while(sz < 512)
            sz += fread(keyboard_cfg+sz, 1, 512-sz, f);
        fclose(f);
    }
}

static void cibyl_write_keyboard_cfg(void)
{
    int l = NOPH_MyXlet_strlenVFSRoot(0);
    char* cfg = (char*)malloc(l+11);
    NOPH_MyXlet_getVFSRoot(0, cfg);
    for(int i = 0; i <= 10; i++)
        cfg[l+i] = "/frodo.cfg\0"[i];
    FILE* f = fopen(cfg, "w");
    if(!f)
    {
        printf("Failed to save keyboard configuration!\n");
        return;
    }
    int sz = 0;
    while(sz < 512)
        sz += fwrite(keyboard_cfg+sz, 1, 512-sz, f);
    fclose(f);
}

static const char* c64_keyboard[8][18] = {
    {"<-", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "+", "-", "POUND", "HOME", "DEL", "F1", NULL},
    {"CTRL", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "@", "*", "^", "F3", NULL, NULL, NULL},
    {"R/S", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", ";", "=", "RETURN", "F5", NULL, NULL, NULL},
    {"C=", "LSHIFT", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "RSHIFT", "UD", "LR", "F7", NULL, NULL},
    {"SPACE", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"JOYSTICK UP", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"JOYSTICK LEFT", "JOYSTICK FIRE", "JOYSTICK RIGHT", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"JOYSTICK DOWN", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

static const int c64_keyboard_matrix[8][18] = {
    {071, 070, 073, 010, 013, 020, 023, 030, 033, 040, 043, 050, 053, 060, 063, 000, 004, 0},
    {072, 076, 011, 016, 021, 026, 031, 036, 041, 046, 051, 056, 061, 066, 005, 0, 0, 0},
    {077, 012, 015, 022, 025, 032, 035, 042, 045, 052, 055, 062, 065, 001, 006, 0, 0, 0},
    {075, 017, 014, 027, 024, 037, 034, 047, 044, 057, 054, 067, 064, 007, 002, 003, 0, 0},
    {074, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0102, 0104, 0103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
    
void cibyl_keyboard_cfg(void)
{
    NOPH_MyXlet_closeGUI();
    printf("Keyboard configuration\n");
    int last_bound = 0;
    int last_bound_to = 0;
    int cur_y = 0;
    int cur_x = 0;
    bool done = false;
    while(!done)
    {
        for(int i = 0; i < 8; i++)
        {
            for(int j = 0; c64_keyboard[i][j]; j++)
                printf(((cur_y==i&&cur_x==j)?"[%s] ":"%s "), c64_keyboard[i][j]);
            printf("\n");
        }
        if(last_bound != 0)
        {
            if(last_bound_to & 128)
                printf("Bound key %d to SHIFT+%s\n", last_bound, c64_keyboard[cur_y][cur_x]);
            else
                printf("Bound key %d to %s\n", last_bound, c64_keyboard[cur_y][cur_x]);
        }
        else
            printf("\n");
        for(;;)
        {
            int key = NOPH_MyXlet_pollInput();
            if(key <= 0 || key >= 512)
                continue;
            int lb = last_bound;
            last_bound = 0;
            int lbt = last_bound_to;
            last_bound_to = 0;
            if(key == 37)
            {
                if(!cur_x)
                    while(c64_keyboard[cur_y][cur_x])
                        cur_x++;
                cur_x--;
            }
            else if(key == 38)
            {
                cur_y = (cur_y + 7) % 8;
                while(!c64_keyboard[cur_y][cur_x])
                    cur_x--;
            }
            else if(key == 39)
            {
                cur_x++;
                if(!c64_keyboard[cur_y][cur_x])
                    cur_x = 0;
            }
            else if(key == 40)
            {
                cur_y = (cur_y + 1) % 8;
                while(!c64_keyboard[cur_y][cur_x])
                    cur_x--;
            }
            else if(key == 19 || key == 415)
                done = true;
            else
            {
                int cur_key = c64_keyboard_matrix[cur_y][cur_x];
                if(lb == key && lbt == cur_key)
                    cur_key |= 128;
                keyboard_cfg[key] = cur_key + 1;
                last_bound = key;
                last_bound_to = cur_key;
            }
            break;
        }
        for(int i = 0; i <= 8; i++)
            NOPH_MyXlet_popMessage();
    }
    NOPH_MyXlet_popMessage();
    NOPH_MyXlet_popMessage();
    printf("\n");
    cibyl_write_keyboard_cfg();
}

void cibyl_main_menu(void)
{
	//NOPH_List_t main_menu_list;
        //NOPH_Display_t display = NOPH_Display_getDisplay(NOPH_MIDlet_get());
        //NOPH_Displayable_t cur = NOPH_Display_getCurrent(display);
        //NOPH_CommandMgr_t cm = NOPH_CommandMgr_getInstance();
        mm_args_t args;
        char buf[80];

        //main_menu_list = NOPH_List_new("Choose option", NOPH_Choice_IMPLICIT);

        snprintf(buf, 80, "Swap joysticks (now port %d)", ThePrefs.JoystickSwap ? 1 : 2);
        //NOPH_List_append(main_menu_list, buf, 0);
        //NOPH_List_append(main_menu_list, "Bind space to GAME_B", 0);
        //NOPH_List_append(main_menu_list, "Bind RunStop to GAME_B", 0);
        //NOPH_List_append(main_menu_list, "Bind Return to GAME_B", 0);
        //NOPH_List_append(main_menu_list, "Bind other to GAME_B", 0);
        //NOPH_List_append(main_menu_list, "Load from disk", 0);
        //NOPH_List_append(main_menu_list, "Load from tape", 0);
        //NOPH_Display_setCurrent(display, main_menu_list);

        /* Setup the callback args */
        args.selected = 0;
        //args.menu_list = main_menu_list;
        //NOPH_CommandMgr_setList(cm, main_menu_list, main_menu_callback, &args);

        while(args.selected == 0)
        {
                NOPH_Thread_sleep(250);
        }

	if (args.selected == 2)
		input_menu();

        //NOPH_Display_setCurrent(display, cur);
        //NOPH_delete(main_menu_list);
}
