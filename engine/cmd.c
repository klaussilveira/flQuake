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
// cmd.c -- Quake script command processing module

#include "quakedef.h"

void Cmd_ForwardToServer(void);

#define	MAX_ALIAS_NAME	32

//Edited qrack complete command end
extern	char	key_lines[64][MAXCMDLINE];
extern	int	edit_line;
extern	int	key_linepos;

static	char	compl_common[MAX_FILELENGTH];
static	int	compl_len;
static	int	compl_clen;

//Edited qrack complete command end

typedef struct cmdalias_s {
    struct cmdalias_s	*next;
    char	name[MAX_ALIAS_NAME];
    char	*value;
} cmdalias_t;

cmdalias_t	*cmd_alias;

int trashtest;
int *trashspot;

qboolean	cmd_wait;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f(void)
{
    cmd_wait = true;
}

/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

sizebuf_t	cmd_text;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init(void)
{
    SZ_Alloc(&cmd_text, 8192);		// space for commands and script files
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText(char *text)
{
    int		l;

    l = Q_strlen(text);

    if (cmd_text.cursize + l >= cmd_text.maxsize) {
        Con_Printf("Cbuf_AddText: overflow\n");
        return;
    }

    SZ_Write(&cmd_text, text, Q_strlen(text));
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText(char *text)
{
    char	*temp;
    int		templen;

// copy off any commands still remaining in the exec buffer
    templen = cmd_text.cursize;
    if (templen) {
        temp = Z_Malloc(templen);
        Q_memcpy(temp, cmd_text.data, templen);
        SZ_Clear(&cmd_text);
    } else {
        temp = NULL;    // shut up compiler
    }

// add the entire text of the file
    Cbuf_AddText(text);

// add the copied off data
    if (templen) {
        SZ_Write(&cmd_text, temp, templen);
        Z_Free(temp);
    }
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute(void)
{
    int		i;
    char	*text;
    char	line[1024];
    int		quotes;

    while (cmd_text.cursize) {
// find a \n or ; line break
        text = (char *)cmd_text.data;

        quotes = 0;
        for (i=0 ; i< cmd_text.cursize ; i++) {
            if (text[i] == '"') {
                quotes++;
            }
            if (!(quotes&1) &&  text[i] == ';') {
                break;    // don't break if inside a quoted string
            }
            if (text[i] == '\n') {
                break;
            }
        }


        memcpy(line, text, i);
        line[i] = 0;

// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer

        if (i == cmd_text.cursize) {
            cmd_text.cursize = 0;
        } else {
            i++;
            cmd_text.cursize -= i;
            Q_memcpy(text, text+i, cmd_text.cursize);
        }

// execute the command line
        Cmd_ExecuteString(line, src_command);

        if (cmd_wait) {
            // skip out while text still remains in buffer, leaving it
            // for next frame
            cmd_wait = false;
            break;
        }
    }
}

/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f(void)
{
    int		i, j;
    int		s;
    char	*text, *build, c;

    if (Cmd_Argc() != 1) {
        Con_Printf("stuffcmds : execute command line parameters\n");
        return;
    }

// build the combined string to parse from
    s = 0;
    for (i=1 ; i<com_argc ; i++) {
        if (!com_argv[i]) {
            continue;    // NEXTSTEP nulls out -NXHost
        }
        s += Q_strlen(com_argv[i]) + 1;
    }
    if (!s) {
        return;
    }

    text = Z_Malloc(s+1);
    text[0] = 0;
    for (i=1 ; i<com_argc ; i++) {
        if (!com_argv[i]) {
            continue;    // NEXTSTEP nulls out -NXHost
        }
        Q_strcat(text,com_argv[i]);
        if (i != com_argc-1) {
            Q_strcat(text, " ");
        }
    }

// pull out the commands
    build = Z_Malloc(s+1);
    build[0] = 0;

    for (i=0 ; i<s-1 ; i++) {
        if (text[i] == '+') {
            i++;

            for (j=i ; (text[j] != '+') && (text[j] != '-') && (text[j] != 0) ; j++)
                ;

            c = text[j];
            text[j] = 0;

            Q_strcat(build, text+i);
            Q_strcat(build, "\n");
            text[j] = c;
            i = j-1;
        }
    }

    if (build[0]) {
        Cbuf_InsertText(build);
    }

    Z_Free(text);
    Z_Free(build);
}


/*
===============
Cmd_Exec_f
===============
*/
#define	MAX_ARGS		80 // config.cfg replacement
static	char		*cmd_argv[MAX_ARGS]; // config.cfg replacement
void Cmd_Exec_f(void)
{
    char	*f;
    int		mark;
    loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

    if (Cmd_Argc() != 2) {
        Con_Printf("exec <filename> : execute a script file\n");
        return;
    }

    mark = Hunk_LowMark();
#ifdef FLASH
    as3ReadFileSharedObject(va("%s/%s", com_gamedir, Cmd_Argv(1)));//config.cfg is stored in the flash shared objects
#endif
    // config.cfg replacement - begin
    if (!Q_strcmp(Cmd_Argv(1), "config.cfg"))
//		f = (char *)COM_LoadHunkFile ("makaqu.cfg"); // it doesn't work here! argh!
    {
        int h;
// VMU saves - begin
#ifdef _arch_dreamcast
        extern cvar_t savename;
        if (VMU_Load("/ram/nxmakaqu.cfg", va("%s.CFG", savename.string))) {
            return;
        }
        /*
        		{
        			Z_Free (cmd_argv[1]);
        			cmd_argv[1] = Z_Malloc (18);//(Q_strlen("/ram/nxmakaqu.cfg")+1);
        			Q_strcpy(cmd_argv[1], "/ram/nxmakaqu.cfg");
        		}
        */
        else {
            Con_DPrintf("Config file not found in VMU; Loading from disc...\n");
#endif
// VMU saves - end
            COM_OpenFile("makaqu.cfg", &h, NULL);
            if (h != -1) {
                Q_strcpy(cmd_argv[1], "makaqu.cfg"); // "config.cfg" and "makaqu.cfg" have the same size, so there's no need to realloc the buffer
                COM_CloseFile(h);
            }
// VMU saves - begin
#ifdef _arch_dreamcast
        }
#endif
// VMU saves - end
    }
//	if (!f)
    // config.cfg replacement - end
// 2001-09-12 Returning information about loaded file by Maddes  start
    /*
    	f = (char *)COM_LoadHunkFile (Cmd_Argv(1));
    	if (!f)
    */
    fileinfo = COM_LoadHunkFile(Cmd_Argv(1));
    if (!fileinfo)
// 2001-09-12 Returning information about loaded file by Maddes  end
    {
        Con_Printf("couldn't exec %s\n",Cmd_Argv(1));
        return;
    }
    f = (char *)fileinfo->data;	// 2001-09-12 Returning information about loaded file by Maddes
    Con_DPrintf("execing %s\n",Cmd_Argv(1));  // edited

    Cbuf_InsertText("\n");  // for files that doesn't end with a newline
    Cbuf_InsertText(f);
    // begin
    // skip a frame to prevent possible crashes when the engine is started with the "-map" parameter
//	if (!Q_strcmp(Cmd_Argv(1), "quake.rc"))
    Cbuf_InsertText("wait\n");
    // end
    Hunk_FreeToLowMark(mark);
// VMU saves - begin
#ifdef _arch_dreamcast
    if (!Q_strcmp(cmd_argv[1], "/ram/nxmakaqu.cfg")) {
        fs_unlink("/ram/nxmakaqu.cfg");
    }
#endif
// VMU saves - end
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f(void)
{
    int		i;

    for (i=1 ; i<Cmd_Argc() ; i++) {
        Con_Printf("%s ",Cmd_Argv(i));
    }
    Con_Printf("\n");
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/

char *CopyString(char *in)
{
    char	*out;

    out = Z_Malloc(strlen(in)+1);
    strcpy(out, in);
    return out;
}

void Cmd_Alias_f(void)
{
    cmdalias_t	*a;
    char		cmd[1024];
    int			i, c;
    char		*s;

    if (Cmd_Argc() == 1) {
        Con_Printf("Current alias commands:\n");
        for (a = cmd_alias ; a ; a=a->next) {
            Con_Printf("%s : %s\n", a->name, a->value);
        }
        return;
    }

    s = Cmd_Argv(1);
    if (strlen(s) >= MAX_ALIAS_NAME) {
        Con_Printf("Alias name is too long\n");
        return;
    }

    // if the alias allready exists, reuse it
    for (a = cmd_alias ; a ; a=a->next) {
        if (!strcmp(s, a->name)) {
            Z_Free(a->value);
            break;
        }
    }

    if (!a) {
        a = Z_Malloc(sizeof(cmdalias_t));
        a->next = cmd_alias;
        cmd_alias = a;
    }
    strcpy(a->name, s);

// copy the rest of the command line
    cmd[0] = 0;		// start out with a null string
    c = Cmd_Argc();
    for (i=2 ; i< c ; i++) {
        strcat(cmd, Cmd_Argv(i));
        if (i != c) {
            strcat(cmd, " ");
        }
    }
    strcat(cmd, "\n");

    a->value = CopyString(cmd);
}


// auto-completion of aliases - begin
/*
============
Cmd_CompleteAlias
============
*/
char *Cmd_CompleteAlias(char *partial)
{
    cmdalias_t		*a;
    int				len;

    len = Q_strlen(partial);

    if (!len) {
        return NULL;
    }

// check aliases
    for (a = cmd_alias ; a ; a=a->next)
        if (!Q_strncmp(partial,a->name, len)) {
            return a->name;
        }

    return NULL;
}
// auto-completion of aliases - end

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s {
    struct cmd_function_s	*next;
    char					*name;
    xcommand_t				function;
} cmd_function_t;


#define	MAX_ARGS		80

static	int			cmd_argc;
static	char		*cmd_argv[MAX_ARGS];
static	char		*cmd_null_string = "";
static	char		*cmd_args = NULL;

cmd_source_t	cmd_source;


static	cmd_function_t	*cmd_functions;		// possible commands to execute

// cvarlist & cmdlist - begin
/*
============
cvarlist
============
*/
void Cvar_CvarList_f(void)
{
    cvar_t		*cvar;
    int i, j, len = Q_strlen(Cmd_Argv(1));

    Con_Printf("\nAttributes: A=Archive, S=Server, C=Callback\n");
    if (!len) {
        for (i=33 ; i<256; i++)
            for (j=0 ; j<256; j++)
                for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
                    if (cvar->name[0] == i && cvar->name[1] == j) {
                        if (!Q_strcmp(Cmd_Argv(0), "cvarlist_s") && !cvar->server) {
                            continue;
                        } else if (!Q_strcmp(Cmd_Argv(0), "cvarlist_a") && !cvar->archive) {
                            continue;
                        }
                        Con_Printf(va("%c%c%c %s  %s\n"), 128+32 + ('A'-32)*cvar->archive, 128+32 + ('S'-32)*cvar->server, 128+32 + ('C'-32)*(cvar->Cvar_Changed!=NULL), cvar->name, cvar->string);
                    }
    } else {
        for (j=0 ; j<256; j++)
            for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
                if (!Q_strncmp(Cmd_Argv(1), cvar->name, len))
                    if (cvar->name[len] == j) {
                        if (!Q_strcmp(Cmd_Argv(0), "cvarlist_s") && !cvar->server) {
                            continue;
                        } else if (!Q_strcmp(Cmd_Argv(0), "cvarlist_a") && !cvar->archive) {
                            continue;
                        }
                        Con_Printf(va("%c%c%c %s  %s\n"), 128+32 + ('A'-32)*cvar->archive, 128+32 + ('S'-32)*cvar->server, 128+32 + ('C'-32)*(cvar->Cvar_Changed!=NULL), cvar->name, cvar->string);
                    }
    }
}

/*
============
cmdlist
============
*/
void Cmd_CmdList_f(void)
{
    cmd_function_t	*cmd;
    int i, j, len = Q_strlen(Cmd_Argv(1));

    if (!len) {
        for (i=33 ; i<256; i++)
            for (j=0 ; j<256; j++)
                for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
                    if (cmd->name[0] == i && cmd->name[1] == j) {
                        Con_Printf(cmd->name);
                        Con_Printf("\n");
                    }
    } else {
        char *c = Cmd_Argv(1);

        for (j=0 ; j<256; j++)
            for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
                if (!Q_strncmp(Cmd_Argv(1), cmd->name, len))
                    if (cmd->name[len] == j) {
                        Con_Printf(cmd->name);
                        Con_Printf("\n");
                    }

        if (c[0] == '*') {
            for (j=0 ; j<256; j++)
                for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
                    if (Q_strlen(cmd->name) >= len-1) // to prevent a negative pointer value on cmd->name
                        if (!Q_strncmp(Cmd_Argv(1)+1, cmd->name + Q_strlen(cmd->name) - len + 1, len-1))
                            if (cmd->name[0] == j) {
                                Con_Printf(cmd->name);
                                Con_Printf("\n");
                            }
        }
    }
}
// cvarlist & cmdlist - end

/*
============
Cmd_Init
============
*/
void Cmd_Init(void)
{
//
// register our commands
//
    // cvarlist & cmdlist - begin
    Cmd_AddCommand("cmdlist", Cmd_CmdList_f);
    Cmd_AddCommand("cvarlist_a", Cvar_CvarList_f);
    Cmd_AddCommand("cvarlist_s", Cvar_CvarList_f);
    Cmd_AddCommand("cvarlist", Cvar_CvarList_f);
    // cvarlist & cmdlist - end
    Cmd_AddCommand("stuffcmds",Cmd_StuffCmds_f);
    Cmd_AddCommand("exec",Cmd_Exec_f);
    Cmd_AddCommand("echo",Cmd_Echo_f);
    Cmd_AddCommand("alias",Cmd_Alias_f);
    Cmd_AddCommand("cmd", Cmd_ForwardToServer);
    Cmd_AddCommand("wait", Cmd_Wait_f);
}

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc(void)
{
    return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_Argv(int arg)
{
    if ((unsigned)arg >= cmd_argc) {
        return cmd_null_string;
    }
    return cmd_argv[arg];
}

/*
============
Cmd_Args
============
*/
char		*Cmd_Args(void)
{
    return cmd_args;
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString(char *text)
{
    int		i;

// clear the args from the last string
    for (i=0 ; i<cmd_argc ; i++) {
        Z_Free(cmd_argv[i]);
    }

    cmd_argc = 0;
    cmd_args = NULL;

    while (1) {
// skip whitespace up to a /n
        while (*text && *text <= ' ' && *text != '\n') {
            text++;
        }

        if (*text == '\n') {
            // a newline seperates commands in the buffer
            text++;
            break;
        }

        if (!*text) {
            return;
        }

        if (cmd_argc == 1) {
            cmd_args = text;
        }

        text = COM_Parse(text);
        if (!text) {
            return;
        }

        if (cmd_argc < MAX_ARGS) {
            cmd_argv[cmd_argc] = Z_Malloc(Q_strlen(com_token)+1);
            Q_strcpy(cmd_argv[cmd_argc], com_token);
            cmd_argc++;
        }
    }

}


/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand(char *cmd_name, xcommand_t function)
{
    cmd_function_t	*cmd;

    if (host_initialized) {	// because hunk allocation would get stomped
        Sys_Error("Cmd_AddCommand after host_initialized");
    }

// fail if the command is a variable name
    if (Cvar_VariableString(cmd_name)[0]) {
        Con_Printf("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
        return;
    }

// fail if the command already exists
    for (cmd=cmd_functions ; cmd ; cmd=cmd->next) {
        if (!Q_strcmp(cmd_name, cmd->name)) {
            Con_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
            return;
        }
    }

    cmd = Hunk_Alloc(sizeof(cmd_function_t));
    cmd->name = cmd_name;
    cmd->function = function;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
}

/*
============
Cmd_Exists
============
*/
qboolean	Cmd_Exists(char *cmd_name)
{
    cmd_function_t	*cmd;

    for (cmd=cmd_functions ; cmd ; cmd=cmd->next) {
        if (!Q_strcmp(cmd_name,cmd->name)) {
            return true;
        }
    }

    return false;
}


/*
============
Cmd_CompleteCommand
============
*/
char *Cmd_CompleteCommand(char *partial)
{
    cmd_function_t	*cmd;
    int		len;

    if (!(len = strlen(partial))) {
        return NULL;
    }

// check functions
    for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
        if (!Q_strncasecmp(partial, cmd->name, len)) {
            return cmd->name;
        }

    return NULL;
}

//Edited qrack command line begin

/*
============
Cmd_CompleteCountPossible
============
*/
int Cmd_CompleteCountPossible(char *partial)
{
    cmd_function_t	*cmd;
    int		len, c = 0;

    if (!(len = strlen(partial))) {
        return 0;
    }

    for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
        if (!Q_strncasecmp(partial, cmd->name, len)) {
            c++;
        }

    return c;
}


static void FindCommonSubString(char *s)
{
    if (!compl_clen) {
        strncpy(compl_common, s, sizeof(compl_common));
        compl_clen = strlen(compl_common);
    } else {
        while (compl_clen > compl_len && Q_strncasecmp(s, compl_common, compl_clen)) {
            compl_clen--;
        }
    }
}


/*
==================
CompleteCommand

Advanced command completion

Main body and many features imported from ZQuake	-- joe
==================
*/
void CompleteCommand(void)
{
    int	c, v;
    char	*s, *cmd;

    s = key_lines[edit_line] + 1;
    if (!(compl_len = strlen(s))) {
        return;
    }
    compl_clen = 0;

    c = Cmd_CompleteCountPossible(s);
    v = Cvar_CompleteCountPossible(s);

    if (c + v > 1) {
        Con_Printf("\n");

        if (c) {
            cmd_function_t	*cmd;

            Con_Printf("\x02" "commands:\n");
            // check commands
            for (cmd = cmd_functions ; cmd ; cmd = cmd->next) {
                if (!Q_strncasecmp(s, cmd->name, compl_len)) {
                    Con_Printf("%s\n",cmd->name);
                    FindCommonSubString(cmd->name);
                }
            }
            Con_Printf("\n");
        }

        if (v) {
            cvar_t		*var;

            Con_Printf("\x02" "variables:\n");
            // check variables
            for (var = cvar_vars ; var ; var = var->next) {
                if (!Q_strncasecmp(s, var->name, compl_len)) {
                    Con_Printf("%s\n",var->name);
                    FindCommonSubString(var->name);
                }
            }
            Con_Printf("\n");
        }
    }

    if (c + v == 1) {
        if (!(cmd = Cmd_CompleteCommand(s))) {
            cmd = Cvar_CompleteVariable(s);
        }
    } else if (compl_clen) {
        compl_common[compl_clen] = 0;
        cmd = compl_common;
    } else {
        return;
    }

    strcpy(key_lines[edit_line]+1, cmd);
    key_linepos = strlen(cmd) + 1;
    if (c + v == 1) {
        key_lines[edit_line][key_linepos++] = ' ';
    }
    key_lines[edit_line][key_linepos] = 0;
}
//Edited qrack command line end


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void	Cmd_ExecuteString(char *text, cmd_source_t src)
{
    cmd_function_t	*cmd;
    cmdalias_t		*a;

    cmd_source = src;
    Cmd_TokenizeString(text);

// execute the command line
    if (!Cmd_Argc()) {
        return;    // no tokens
    }

// check functions
    for (cmd=cmd_functions ; cmd ; cmd=cmd->next) {
        if (!Q_strcasecmp(cmd_argv[0],cmd->name)) {
            cmd->function();
            return;
        }
    }

// check alias
    for (a=cmd_alias ; a ; a=a->next) {
        if (!Q_strcasecmp(cmd_argv[0], a->name)) {
            Cbuf_InsertText(a->value);
            return;
        }
    }

// check cvars
    if (!Cvar_Command()) {
        Con_Printf("Unknown command \"%s\"\n", Cmd_Argv(0));
    }

}


/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer(void)
{
    if (cls.state != ca_connected) {
        Con_Printf("Can't \"%s\", not connected\n", Cmd_Argv(0));
        return;
    }

    if (cls.demoplayback) {
        return;    // not really connected
    }

    MSG_WriteByte(&cls.message, clc_stringcmd);
    if (Q_strcasecmp(Cmd_Argv(0), "cmd") != 0) {
        SZ_Print(&cls.message, Cmd_Argv(0));
        SZ_Print(&cls.message, " ");
    }
    if (Cmd_Argc() > 1) {
        SZ_Print(&cls.message, Cmd_Args());
    } else {
        SZ_Print(&cls.message, "\n");
    }
}


/*
================
Cmd_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/

int Cmd_CheckParm(char *parm)
{
    int i;

    if (!parm) {
        Sys_Error("Cmd_CheckParm: NULL");
    }

    for (i = 1; i < Cmd_Argc(); i++)
        if (! Q_strcasecmp(parm, Cmd_Argv(i))) {
            return i;
        }

    return 0;
}
