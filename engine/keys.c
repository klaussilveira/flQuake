/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "quakedef.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256
char	key_lines[32][MAXCMDLINE];
int		key_linepos;
int		shift_down=false;
int		key_lastpress;

int		edit_line=0;
int		history_line=0;

keydest_t	key_dest;

int		key_count;			// incremented every key event

char	*keybindings[256];
char	*shiftbindings[256];	// function shift
qboolean	shift_function;		// function shift
qboolean	consolekeys[256];	// if true, can't be rebound while in console
qboolean	menubound[256];	// if true, can't be rebound while in menu
qboolean	demokeys[256];					// Edited
qboolean	key_menurepeats[256];			// Edited
int		keyboard_active;					// on-screen keyboard
int		M_OnScreenKeyboard_Key(int key);	// on-screen keyboard
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
qboolean	keydown[256];

typedef struct {
    char	*name;
    int		keynum;
} keyname_t;

keyname_t keynames[] = {
    {"TAB", K_TAB},
    {"ENTER", K_ENTER},
    {"ESCAPE", K_ESCAPE},
    {"SPACE", K_SPACE},
    {"BACKSPACE", K_BACKSPACE},
    {"UPARROW", K_UPARROW},
    {"DOWNARROW", K_DOWNARROW},
    {"LEFTARROW", K_LEFTARROW},
    {"RIGHTARROW", K_RIGHTARROW},

    {"ALT", K_ALT},
    {"CTRL", K_CTRL},
    {"SHIFT", K_SHIFT},

    {"F1", K_F1},
    {"F2", K_F2},
    {"F3", K_F3},
    {"F4", K_F4},
    {"F5", K_F5},
    {"F6", K_F6},
    {"F7", K_F7},
    {"F8", K_F8},
    {"F9", K_F9},
    {"F10", K_F10},
    {"F11", K_F11},
    {"F12", K_F12},

    {"INS", K_INS},
    {"DEL", K_DEL},
    {"PGDN", K_PGDN},
    {"PGUP", K_PGUP},
    {"HOME", K_HOME},
    {"END", K_END},

    {"MOUSE1", K_MOUSE1},
    {"MOUSE2", K_MOUSE2},
    {"MOUSE3", K_MOUSE3},
//	{"MOUSE4", K_MOUSE4}, // Edited
//	{"MOUSE5", K_MOUSE5}, // Edited

#ifdef _arch_dreamcast // BlackAura (09-12-2002) - DC controller - begin
    {"DC_A", K_DC_A},
    {"DC_B", K_DC_B},
    {"DC_C", K_DC_C},
    {"DC_D", K_DC_D},
    {"DC_X", K_DC_X},
    {"DC_Y", K_DC_Y},
    {"DC_Z", K_DC_Z},
    {"DC_START",	K_DC_START},
    {"DC_DUP",		K_DPAD1_UP},
    {"DC_DDOWN",	K_DPAD1_DOWN},
    {"DC_DLEFT",	K_DPAD1_LEFT},
    {"DC_DRIGHT",	K_DPAD1_RIGHT},
    {"DC_DUP2",		K_DPAD2_UP},
    {"DC_DDOWN2",	K_DPAD2_DOWN},
    {"DC_DLEFT2",	K_DPAD2_LEFT},
    {"DC_DRIGHT2",	K_DPAD2_RIGHT},
    {"DC_TRIGL",	K_DC_LTRIG},
    {"DC_TRIGR",	K_DC_RTRIG},
    // begin
    {"DC_AXIS_L",	K_DC_AXISL},
    {"DC_AXIS_R",	K_DC_AXISR},
    {"DC_AXIS_U",	K_DC_AXISU},
    {"DC_AXIS_D",	K_DC_AXISD},

    {"DC2_A",		K_DC2_A},
    {"DC2_B",		K_DC2_B},
    {"DC2_C",		K_DC2_C},
    {"DC2_D",		K_DC2_D},
    {"DC2_X",		K_DC2_X},
    {"DC2_Y",		K_DC2_Y},
    {"DC2_Z",		K_DC2_Z},
    {"DC2_START",	K_DC2_START},
    {"DC2_DUP",		K_DC2_DPAD1_UP},
    {"DC2_DDOWN",	K_DC2_DPAD1_DOWN},
    {"DC2_DLEFT",	K_DC2_DPAD1_LEFT},
    {"DC2_DRIGHT",	K_DC2_DPAD1_RIGHT},
    {"DC2_DUP2",	K_DC2_DPAD2_UP},
    {"DC2_DDOWN2",	K_DC2_DPAD2_DOWN},
    {"DC2_DLEFT2",	K_DC2_DPAD2_LEFT},
    {"DC2_DRIGHT2",	K_DC2_DPAD2_RIGHT},
    {"DC2_TRIGL",	K_DC2_LTRIG},
    {"DC2_TRIGR",	K_DC2_RTRIG},
    {"DC2_AXIS_L",	K_DC2_AXISL},
    {"DC2_AXIS_R",	K_DC2_AXISR},
    {"DC2_AXIS_U",	K_DC2_AXISU},
    {"DC2_AXIS_D",	K_DC2_AXISD},
    // end
#else // BlackAura (09-12-2002) - DC controller - end

    // testing - begin
#if 0
    {"DC_A", K_DC_A},
    {"DC_B", K_DC_B},
    {"DC_C", K_DC_C},
    {"DC_D", K_DC_D},
    {"DC_X", K_DC_X},
    {"DC_Y", K_DC_Y},
    {"DC_Z", K_DC_Z},
    {"DC_START",	K_DC_START},
    {"DC_DUP",		K_DPAD1_UP},
    {"DC_DDOWN",	K_DPAD1_DOWN},
    {"DC_DLEFT",	K_DPAD1_LEFT},
    {"DC_DRIGHT",	K_DPAD1_RIGHT},
    {"DC_TRIGL",	K_DC_LTRIG},
    {"DC_TRIGR",	K_DC_RTRIG},
#endif
    // testing - end

    {"JOY1", K_JOY1},
    {"JOY2", K_JOY2},
    {"JOY3", K_JOY3},
    {"JOY4", K_JOY4},

    {"AUX1", K_AUX1},
    {"AUX2", K_AUX2},
    {"AUX3", K_AUX3},
    {"AUX4", K_AUX4},
    {"AUX5", K_AUX5},
    {"AUX6", K_AUX6},
    {"AUX7", K_AUX7},
    {"AUX8", K_AUX8},
    {"AUX9", K_AUX9},
    {"AUX10", K_AUX10},
    {"AUX11", K_AUX11},
    {"AUX12", K_AUX12},
    {"AUX13", K_AUX13},
    {"AUX14", K_AUX14},
    {"AUX15", K_AUX15},
    {"AUX16", K_AUX16},
    {"AUX17", K_AUX17},
    {"AUX18", K_AUX18},
    {"AUX19", K_AUX19},
    {"AUX20", K_AUX20},
    {"AUX21", K_AUX21},
    {"AUX22", K_AUX22},
    {"AUX23", K_AUX23},
    {"AUX24", K_AUX24},
    {"AUX25", K_AUX25},
    {"AUX26", K_AUX26},
    {"AUX27", K_AUX27},
    {"AUX28", K_AUX28},
    {"AUX29", K_AUX29},
    {"AUX30", K_AUX30},
    {"AUX31", K_AUX31},
    {"AUX32", K_AUX32},
#endif // BlackAura (09-12-2002) - DC controller

    {"PAUSE", K_PAUSE},

    {"MWHEELUP", K_MWHEELUP},
    {"MWHEELDOWN", K_MWHEELDOWN},

    {"SEMICOLON", ';'},	// because a raw semicolon seperates commands

    {NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/


/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console(int key)
{
    char	*cmd;

    key_dest = key_console; // on-screen keyboard

    key = M_OnScreenKeyboard_Key(key);  // on-screen keyboard

    // enter command
    if (key == K_ENTER || key == K_DC_Y) { // controller functionality to the console - edited
        Cbuf_AddText(key_lines[edit_line]+1);	// skip the >
        Cbuf_AddText("\n");
        Con_Printf("%s\n",key_lines[edit_line]);
        edit_line = (edit_line + 1) & 31;
        history_line = edit_line;
        key_lines[edit_line][0] = ']';
        key_linepos = 1;
        if (cls.state == ca_disconnected) {
            SCR_UpdateScreen();    // force an update, because the command
        }
        // may take some time
        return;
    }

    // autocomplete
    if (key == K_TAB) {
        // various parameter completions -- by joe
        cmd = key_lines[edit_line] + 1;
        CompleteCommand();
    }



    /* Edited old command completion

    {	// command completion
    		cmd = Cmd_CompleteCommand (key_lines[edit_line]+1);
    		if (!cmd)
    			cmd = Cvar_CompleteVariable (key_lines[edit_line]+1);
    		if (!cmd) // auto-completion of aliases
    			cmd = Cmd_CompleteAlias (key_lines[edit_line]+1); // auto-completion of aliases
    		if (cmd)
    		{
    			Q_strcpy (key_lines[edit_line]+1, cmd);
    			key_linepos = Q_strlen(cmd)+1;
    			key_lines[edit_line][key_linepos] = ' ';
    			key_linepos++;
    			key_lines[edit_line][key_linepos] = 0;
    			return;
    		}
    	}
    */
    if (key == K_BACKSPACE || key == K_LEFTARROW) {
        if (key_linepos > 1) {
            key_linepos--;
        }
        return;
    }

    // previous commands
    if (key == K_UPARROW || key == K_DPAD1_LEFT) { // controller functionality to the console - edited
        do {
            history_line = (history_line - 1) & 31;
        } while (history_line != edit_line
                 && !key_lines[history_line][1]);
        if (history_line == edit_line) {
            history_line = (edit_line+1)&31;
        }
        Q_strcpy(key_lines[edit_line], key_lines[history_line]);
        key_linepos = Q_strlen(key_lines[edit_line]);
        return;
    }
    if (key == K_DOWNARROW || key == K_DPAD1_RIGHT) { // controller functionality to the console - edited
        if (history_line == edit_line) {
            return;
        }
        do {
            history_line = (history_line + 1) & 31;
        } while (history_line != edit_line
                 && !key_lines[history_line][1]);
        if (history_line == edit_line) {
            key_lines[edit_line][0] = ']';
            key_linepos = 1;
        } else {
            Q_strcpy(key_lines[edit_line], key_lines[history_line]);
            key_linepos = Q_strlen(key_lines[edit_line]);
        }
        return;
    }

    // scrolling
    if (key == K_PGUP || key==K_MWHEELUP || key == K_DPAD1_UP) { // controller functionality to the console - edited
        con_backscroll += 2;
        if (con_backscroll > con_totallines - (vid.height>>3) - 1) {
            con_backscroll = con_totallines - (vid.height>>3) - 1;
        }
        return;
    }

    if (key == K_PGDN || key==K_MWHEELDOWN || key == K_DPAD1_DOWN) { // controller functionality to the console - edited
        con_backscroll -= 2;
        if (con_backscroll < 0) {
            con_backscroll = 0;
        }
        return;
    }

    if (key == K_HOME || key == K_DC_RTRIG) { // controller functionality to the console - edited
        con_backscroll = con_totallines - (vid.height>>3) - 1;
        return;
    }

    if (key == K_END || key == K_DC_LTRIG) { // controller functionality to the console - edited
        con_backscroll = 0;
        return;
    }

    // commandline editing
    if (key < 32 || key > 127) {
        return;    // non printable
    }

    if (key_linepos < MAXCMDLINE-1) {
        key_lines[edit_line][key_linepos] = key;
        key_linepos++;
        key_lines[edit_line][key_linepos] = 0;
    }

}

//============================================================================

char chat_buffer[32];
qboolean team_message = false;

void Key_Message(int key)
{
    static int chat_bufferlen = 0;

    key = M_OnScreenKeyboard_Key(key);  // on-screen keyboard

    if (key == K_ENTER) {
        if (team_message) {
            Cbuf_AddText("say_team \"");
        } else {
            Cbuf_AddText("say \"");
        }
        Cbuf_AddText(chat_buffer);
        Cbuf_AddText("\"\n");

        key_dest = key_game;
        chat_bufferlen = 0;
        chat_buffer[0] = 0;
        return;
    }

    if (key == K_ESCAPE || key == K_DC_B || key == K_DC_START) { // on-screen keyboard - edited
        M_OnScreenKeyboard_Reset(); // on-screen keyboard
        key_dest = key_game;
        chat_bufferlen = 0;
        chat_buffer[0] = 0;
        return;
    }

    if (key < 32 || key > 127) {
        return;    // non printable
    }

    if (key == K_BACKSPACE) {
        if (chat_bufferlen) {
            chat_bufferlen--;
            chat_buffer[chat_bufferlen] = 0;
        }
        return;
    }

    if (chat_bufferlen == 31) {
        return;    // all full
    }

    chat_buffer[chat_bufferlen++] = key;
    chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum(char *str)
{
    keyname_t	*kn;

    if (!str || !str[0]) {
        return -1;
    }
    if (!str[1]) {
        return str[0];
    }

    for (kn=keynames ; kn->name ; kn++) {
        if (!Q_strcasecmp(str,kn->name)) {
            return kn->keynum;
        }
    }
    return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString(int keynum)
{
    keyname_t	*kn;
    static	char	tinystr[2];

    if (keynum == -1) {
        return "<KEY NOT FOUND>";
    }
    if (keynum > 32 && keynum < 127) {
        // printable ascii
        tinystr[0] = keynum;
        tinystr[1] = 0;
        return tinystr;
    }

    for (kn=keynames ; kn->name ; kn++)
        if (keynum == kn->keynum) {
            return kn->name;
        }

    return va("UNKNOWN KEYNUM (%i)", keynum); // edited
}

//=============================================================================
// function shift - begin
//=============================================================================
// These are modified versions of the binding functions
void Key_SetShiftBinding(int keynum, char *binding)
{
    char	*new;
    int		l;

    if (keynum == -1) {
        return;
    }

// free old bindings
    if (shiftbindings[keynum]) {
        Z_Free(shiftbindings[keynum]);
        shiftbindings[keynum] = NULL;
    }

// allocate memory for new binding
    l = Q_strlen(binding);
    if (!l) {
        return;    // true unbinding
    }
    new = Z_Malloc(l+1);
    Q_strcpy(new, binding);
    new[l] = 0;
    shiftbindings[keynum] = new;
}
void Key_UnbindShift_f(void)
{
    int		b;

    if (Cmd_Argc() != 2) {
        Con_Printf("unbindshift <key> : remove commands from a shifted key\n");
        return;
    }

    b = Key_StringToKeynum(Cmd_Argv(1));
    if (b==-1) {
        Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        return;
    }

    Key_SetShiftBinding(b, "");
}
void Key_UnbindallShifts_f(void)
{
    int		i;
    for (i=0 ; i<256 ; i++)
        if (shiftbindings[i]) {
            Key_SetShiftBinding(i, "");
        }
}
void Key_BindShift_f(void)
{
    int			i, c, b;
    char		cmd[1024];

    c = Cmd_Argc();

    if (c == 1) {
        Con_Printf("bindshift <key> [command] : attach a command to a shifted key\n");
        return;
    }
    b = Key_StringToKeynum(Cmd_Argv(1));
    if (b==-1) {
        Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        return;
    }

    if (c == 2) {
        if (shiftbindings[b]) {
            Con_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), shiftbindings[b]);
        } else {
            Con_Printf("\"%s\" is not bound\n", Cmd_Argv(1));
        }
        return;
    }

// copy the rest of the command line
    cmd[0] = 0;		// start out with a null string
    for (i=2 ; i< c ; i++) {
        if (i > 2) {
            strcat(cmd, " ");
        }
        strcat(cmd, Cmd_Argv(i));
    }

    Key_SetShiftBinding(b, cmd);
}
//=============================================================================
// function shift - end
//=============================================================================

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding(int keynum, char *binding)
{
    char	*new;
    int		l;

    if (keynum == -1) {
        return;
    }

// free old bindings
    if (keybindings[keynum]) {
        Z_Free(keybindings[keynum]);
        keybindings[keynum] = NULL;
    }

// allocate memory for new binding
    l = Q_strlen(binding);
    if (!l) {
        return;    // true unbinding
    }
    new = Z_Malloc(l+1);
    Q_strcpy(new, binding);
    new[l] = 0;
    keybindings[keynum] = new;
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f(void)
{
    int		b;

    if (Cmd_Argc() != 2) {
        Con_Printf("unbind <key> : remove commands from a key\n");
        return;
    }

    b = Key_StringToKeynum(Cmd_Argv(1));
    if (b==-1) {
        Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        return;
    }

    Key_SetBinding(b, "");
}

void Key_Unbindall_f(void)
{
    int		i;

    for (i=0 ; i<256 ; i++)
        if (keybindings[i]) {
            Key_SetBinding(i, "");
        }
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f(void)
{
    int			i, c, b;
    char		cmd[1024];

    c = Cmd_Argc();

//	if (c != 2 && c != 3)
    if (c == 1) { // Edited
        Con_Printf("bind <key> [command] : attach a command to a key\n");
        return;
    }
    b = Key_StringToKeynum(Cmd_Argv(1));
    if (b==-1) {
        Con_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        return;
    }

    if (c == 2) {
        if (keybindings[b]) {
            Con_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b]);
        } else {
            Con_Printf("\"%s\" is not bound\n", Cmd_Argv(1));
        }
        return;
    }

// copy the rest of the command line
    cmd[0] = 0;		// start out with a null string
    for (i=2 ; i< c ; i++) {
        if (i > 2) {
            strcat(cmd, " ");
        }
        strcat(cmd, Cmd_Argv(i));
    }

    Key_SetBinding(b, cmd);
}

// reduced config file - begin
qboolean HaveSemicolon(char *s)
{
    while (*s) {
        if (*s == ';') {
            return true;
        }
        s++;
    }
    return false;
}
// reduced config file - end
/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings(FILE *f)
{
    int		i;

    for (i=0 ; i<256 ; i++)
        if (keybindings[i])
            if (*keybindings[i])
                // reduced config file - begin
            {
                fprintf(f, "bind ");
                if (i == ';') {
                    fprintf(f, "\";\" ");
                } else {
                    fprintf(f, "%s ", Key_KeynumToString(i));
                }
                if (HaveSemicolon(keybindings[i])) {
                    fprintf(f, "\"%s\"\n", keybindings[i]);
                } else {
                    fprintf(f, "%s\n", keybindings[i]);
                }
            }
    // reduced config file - end
    // function shift - begin
    for (i=0 ; i<256 ; i++)
        if (shiftbindings[i])
            if (*shiftbindings[i])
                // reduced config file - begin
            {
                fprintf(f, "bindshift ");
                if (i == ';') {
                    fprintf(f, "\";\" ");
                } else {
                    fprintf(f, "%s ", Key_KeynumToString(i));
                }
                if (HaveSemicolon(shiftbindings[i])) {
                    fprintf(f, "\"%s\"\n", shiftbindings[i]);
                } else {
                    fprintf(f, "%s\n", shiftbindings[i]);
                }
            }
    // reduced config file - end
    // function shift - end
}


/*
===================
Key_Init
===================
*/
void Key_Init(void)
{
    int		i;

    for (i=0 ; i<32 ; i++) {
        key_lines[i][0] = ']';
        key_lines[i][1] = 0;
    }
    key_linepos = 1;

//
// init ascii characters in console mode
//
    for (i=32 ; i<128 ; i++) {
        consolekeys[i] = true;
    }
    consolekeys[K_ENTER] = true;
    consolekeys[K_TAB] = true;
    consolekeys[K_LEFTARROW] = true;
    consolekeys[K_RIGHTARROW] = true;
    consolekeys[K_UPARROW] = true;
    consolekeys[K_DOWNARROW] = true;
    consolekeys[K_BACKSPACE] = true;
    consolekeys[K_HOME] = true; // Edited
    consolekeys[K_END] = true; // Edited
    consolekeys[K_PGUP] = true;
    consolekeys[K_PGDN] = true;
    consolekeys[K_SHIFT] = true;
    consolekeys[K_MWHEELUP] = true;
    consolekeys[K_MWHEELDOWN] = true;
    consolekeys['`'] = false;
    consolekeys['~'] = false;
    // controller functionality to the console - begin
    consolekeys[K_DC_A] = true;
    consolekeys[K_DC_B] = true;
    consolekeys[K_DC_X] = true;
    consolekeys[K_DC_Y] = true;
    consolekeys[K_DC_LTRIG] = true;
    consolekeys[K_DC_RTRIG] = true;
    consolekeys[K_DC_START] = true;
    consolekeys[K_DPAD1_LEFT] = true;
    consolekeys[K_DPAD1_DOWN] = true;
    consolekeys[K_DPAD1_RIGHT] = true;
    consolekeys[K_DPAD1_UP] = true;
    // controller functionality to the console - end

    for (i=0 ; i<256 ; i++) {
        keyshift[i] = i;
    }
    for (i='a' ; i<='z' ; i++) {
        keyshift[i] = i - 'a' + 'A';
    }
    keyshift['1'] = '!';
    keyshift['2'] = '@';
    keyshift['3'] = '#';
    keyshift['4'] = '$';
    keyshift['5'] = '%';
    keyshift['6'] = '^';
    keyshift['7'] = '&';
    keyshift['8'] = '*';
    keyshift['9'] = '(';
    keyshift['0'] = ')';
    keyshift['-'] = '_';
    keyshift['='] = '+';
    keyshift[','] = '<';
    keyshift['.'] = '>';
    keyshift['/'] = '?';
    keyshift[';'] = ':';
    keyshift['\''] = '"';
    keyshift['['] = '{';
    keyshift[']'] = '}';
    keyshift['`'] = '~';
    keyshift['\\'] = '|';

    menubound[K_ESCAPE] = true;
    for (i=0 ; i<12 ; i++) {
        menubound[K_F1+i] = true;
    }

    // begin
    for (i=0 ; i<200 ; i++) {
        demokeys[i] = true;
    }
    demokeys[K_PAUSE] = true;
    demokeys[K_DC_START] = true;
    for (i=0 ; i<12 ; i++) {
        demokeys[K_F1+i] = false;
    }
    demokeys['`'] = false;
    demokeys['~'] = false;

    key_menurepeats[K_UPARROW] = true;
    key_menurepeats[K_DOWNARROW] = true;
    key_menurepeats[K_LEFTARROW] = true;
    key_menurepeats[K_RIGHTARROW] = true;
    key_menurepeats[K_DPAD1_LEFT] = true;
    key_menurepeats[K_DPAD1_DOWN] = true;
    key_menurepeats[K_DPAD1_RIGHT] = true;
    key_menurepeats[K_DPAD1_UP] = true;
#ifdef _arch_dreamcast
    key_menurepeats[K_DC2_DPAD1_LEFT] = true;
    key_menurepeats[K_DC2_DPAD1_DOWN] = true;
    key_menurepeats[K_DC2_DPAD1_RIGHT] = true;
    key_menurepeats[K_DC2_DPAD1_UP] = true;
#endif
    // end

//
// register our functions
//
    Cmd_AddCommand("bind",Key_Bind_f);
    Cmd_AddCommand("unbind",Key_Unbind_f);
    Cmd_AddCommand("unbindall",Key_Unbindall_f);


    // function shift - begin
    Cmd_AddCommand("bindshift",Key_BindShift_f);
    Cmd_AddCommand("unbindshift",Key_UnbindShift_f);
    Cmd_AddCommand("unbindallshifts",Key_UnbindallShifts_f);
    // function shift - end
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
#ifdef _arch_dreamcast // Edited
extern int In_IsAxis(int key);
extern int In_AnalogCommand(int key);
#endif // Edited
void Key_Event(int key, qboolean down)
{
    char	*kb;
    char	cmd[1024];

    keydown[key] = down;

    if (!down) {
        key_repeats[key] = 0;
    }

    key_lastpress = key;
    key_count++;
    if (key_count <= 0) {
        return;		// just catching keys for Con_NotifyBox
    }

// update auto-repeat status
    if (down) {
        key_repeats[key]++;
        if (key_repeats[key] > 1) // edited
            if (!(key_dest == key_menu && key_menurepeats[key])) // Edited
                if (!(key_dest == key_console && consolekeys[key])) { // Edited
                    return;	// ignore most autorepeats
                }

//		if (key >= 200 && !keybindings[key]) // removed
//			Con_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString (key) ); // removed
    }

    if (key == K_SHIFT) {
        shift_down = down;
    }

//
// handle escape specialy, so the user can never unbind it
//
    if (key == K_ESCAPE) {
        if (!down) {
            return;
        }
        switch (key_dest) {
        case key_message:
            Key_Message(key);
            break;
        case key_menu:
            M_Keydown(key);
            break;
        case key_game:
        case key_console:
            M_ToggleMenu_f();
            break;
        default:
            Sys_Error("Bad key_dest");
        }
        return;
    }
    // on-screen keyboard - begin
    if (key_dest == key_console)
        if (((key == K_DC_B && !keyboard_active) || key == K_DC_START) && down) {
            keyboard_active	= 0;
            M_ToggleMenu_f();
            return;
        }
    // on-screen keyboard - end

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
    if (!down) {
        // begin
#ifdef _arch_dreamcast
        if (In_AnalogCommand(In_IsAxis(key))) {
            return;
        }
#endif
        // end
        // function shift - begin
        kb = shiftbindings[key];
        if (kb && kb[0] == '+') {
            sprintf(cmd, "-%s %i\n", kb+1, key);
            Cbuf_AddText(cmd);
        }
        // function shift - end
        kb = keybindings[key];
        if (kb && kb[0] == '+') {
            sprintf(cmd, "-%s %i\n", kb+1, key);
            Cbuf_AddText(cmd);
        }
        if (keyshift[key] != key) {
            kb = keybindings[keyshift[key]];
            if (kb && kb[0] == '+') {
                sprintf(cmd, "-%s %i\n", kb+1, key);
                Cbuf_AddText(cmd);
            }
        }
        return;
    }

//
// during demo playback, most keys bring up the main menu
//
    if (cls.demoplayback && down && demokeys[key] && key_dest == key_game) { // edited
        M_ToggleMenu_f();
        return;
    }

//
// if not a consolekey, send to the interpreter no matter what mode is
//
// begin
#ifdef _arch_dreamcast
    if (In_AnalogCommand(In_IsAxis(key))) {
        if (key_dest == key_game) {
            return;
        }
    } else
#endif
// end
        if ((key_dest == key_menu && menubound[key])
                || (key_dest == key_console && !consolekeys[key])
                || (key_dest == key_game && (!con_forcedup || !consolekeys[key]))) {
            // function shift - begin
            if (shift_function && shiftbindings[key]) {
                kb = shiftbindings[key];
            } else
                // function shift - end
            {
                kb = keybindings[key];
            }
            if (kb) {
                if (kb[0] == '+') {
                    // button commands add keynum as a parm
                    sprintf(cmd, "%s %i\n", kb, key);
                    Cbuf_AddText(cmd);
                } else {
                    Cbuf_AddText(kb);
                    Cbuf_AddText("\n");
                }
            }
            // begin
            // if the start button isn't bound to any command, toggle the menu
#ifdef _arch_dreamcast
            else if (key == K_DC_START || key == K_DC2_START) {
                Cbuf_AddText("togglemenu");
                Cbuf_AddText("\n");
            }
#endif
            // end
            return;
        }

    // function shift - begin
    if (key_dest == key_menu)
        if (keybindings[key] && !Q_strcmp(keybindings[key], "+shift")) {
            Cbuf_AddText(va("+shift %i\n", key));    // hack for the "customize controls" menu
        }
    // function shift - end

//	if (!down)	// removed - there's a return earlier on this function
//		return;		// other systems only care about key down events

    if (shift_down) {
        key = keyshift[key];
    }

    switch (key_dest) {
    case key_message:
        Key_Message(key);
        break;

    case key_menu:
        M_Keydown(key);
        break;

    case key_game:
    case key_console:
        Key_Console(key);
        break;

    default:
        Sys_Error("Bad key_dest");
    }
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates(void)
{
    int		i;

    for (i=0 ; i<256 ; i++) {
        keydown[i] = false;
        key_repeats[i] = 0;
    }
}
