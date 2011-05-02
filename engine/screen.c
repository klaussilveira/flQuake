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
// screen.c -- master for refresh, status bar, console, chat, notify, etc

#include "quakedef.h"
#include "r_local.h"

// only the refresh window will be updated unless these variables are flagged
int			scr_copytop;
int			scr_copyeverything;

float		scr_con_current;
float		scr_conlines;		// lines of console to display

float		oldfov; // edited
cvar_t		scr_viewsize = {"viewsize","100", true};
cvar_t		scr_fov = {"fov","90"};	// 10 - 170
cvar_t		scr_conspeed = {"scr_conspeed","999999"};
cvar_t		scr_centertime = {"scr_centertime","2"};
cvar_t		scr_showram = {"showram","0"};
cvar_t		scr_showturtle = {"showturtle","0"};
cvar_t		scr_showpause = {"showpause","1"};
cvar_t		scr_printspeed = {"scr_printspeed","16"}; // 8 // edited

qboolean	scr_initialized;		// ready to draw

qpic_t		*scr_ram;
qpic_t		*scr_net;
//qpic_t		*scr_turtle; // removed

int			scr_fullupdate;

int			clearconsole;
int			clearnotify;

viddef_t	vid;				// global video state

vrect_t		*pconupdate;
vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;
qboolean	scr_drawloading;
float		scr_disabled_time;
qboolean	scr_skipupdate;

qboolean	block_drawing;

void SCR_ScreenShot_f(void);

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_lines;
int			scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint(char *str)
{
    strncpy(scr_centerstring, str, sizeof(scr_centerstring)-1);
    scr_centertime_off = scr_centertime.value;
    scr_centertime_start = cl.time;

// count the number of lines for centering
    scr_center_lines = 1;
    while (*str) {
        if (*str == '\n') {
            scr_center_lines++;
        }
        str++;
    }
}

void SCR_EraseCenterString(void)
{
    int		y;
    int		h; // Edited

    if (scr_erase_center++ > vid.numpages) {
        scr_erase_lines = 0;
        return;
    }

    // svc_letterbox - begin
    if (cl.letterbox) {
        y = scr_vrect.y + scr_vrect.height + 4;
    } else
        // svc_letterbox - end
        if (scr_center_lines <= 4) {
            y = vid.height*0.35;
        } else {
            y = screen_top + 44;    //28+16;//48; // edited
        }

    scr_copytop = 1;
    // begin
    // limit the height to prevent crashes with huge centerprints
    h = 8*scr_erase_lines;
    if (y+h > vid.height) {
        h = vid.height - y;
    }
    Draw_Fill(0, y,vid.width, h, 0);
    // end
}

void SCR_DrawCenterString(void)
{
    char	*start;
    int		l;
    int		j;
    int		x, y;
    int		remaining;

// the finale prints the characters one at a time
    if (cl.intermission) {
        remaining = scr_printspeed.value * (cl.time - scr_centertime_start);
    } else {
        remaining = 9999;
    }

    scr_erase_center = 0;
    start = scr_centerstring;

    // svc_letterbox - begin
    if (cl.letterbox) {
        y = scr_vrect.y + scr_vrect.height + 4;
    } else
        // svc_letterbox - end
        if (scr_center_lines <= 4) {
            y = vid.height*0.35;
        } else {
            y = screen_top + 44;    //28+16;//48; // edited
        }

    do {
        // scan the width of the line
        for (l=0 ; l<40 ; l++)
            if (start[l] == '\n' || !start[l]) {
                break;
            }
        x = (vid.width - l*8)/2;
        for (j=0 ; j<l ; j++, x+=8) {
            Draw_Character(x, y, start[j]);
            if (!remaining--) {
                return;
            }
        }

        y += 8;

        while (*start && *start != '\n') {
            start++;
        }

        if (!*start) {
            break;
        }
        start++;		// skip the \n
    } while (1);
}

void SCR_CheckDrawCenterString(void)
{
    scr_copytop = 1;
    if (scr_center_lines > scr_erase_lines) {
        scr_erase_lines = scr_center_lines;
    }

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	scr_centertime_off -= host_frametime;
    scr_centertime_off -= host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end

    if (scr_centertime_off <= 0 && !cl.intermission) {
        return;
    }
    if (key_dest != key_game) {
        return;
    }

    SCR_DrawCenterString();
}

//=============================================================================

/*
====================
CalcFov
====================
*/
float CalcFov(float fov_x, float width, float height)
{
    float   a;
    float   x;

    if (fov_x < 1 || fov_x > 179) {
        Sys_Error("Bad fov: %f", fov_x);
    }

    x = width/tan(fov_x/360*M_PI);

    a = atan(height/x);

    a = a*360/M_PI;

    return a;
}

/*
=================
SCR_CalcRefdef

Must be called whenever vid changes
Internal use only
=================
*/
static void SCR_CalcRefdef(void)
{
    vrect_t		vrect;
//	float		size; // removed

    scr_fullupdate = 0;		// force a background redraw
    vid.recalc_refdef = 0;

// force the status bar to redraw
    Sbar_Changed();

//========================================

    r_refdef.fov_x = scr_fov.value;
    r_refdef.fov_y = CalcFov(r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);

    fovscale = 90.0f/scr_fov.value; // FOV-based scaling
    /*/// removed - begin
    // intermission is always full screen
    	if (cl.intermission)
    		size = 120;
    	else
    		size = scr_viewsize.value;

    	if (size >= 120)
    		sb_lines = 0;		// no status bar at all
    	else if (size >= 110)
    		sb_lines = 24;		// no inventory
    	else
    		sb_lines = 24+16+8;
    /*/// removed - end

// these calculations mirror those in R_Init() for r_refdef, but take no
// account of water warping
    vrect.x = 0;
    vrect.y = 0;
    vrect.width = vid.width;
    vrect.height = vid.height;

    R_SetVrect(&vrect, &scr_vrect, sb_lines);

// guard against going from one mode to another that's less than half the
// vertical resolution
    if (scr_con_current > vid.height) {
        scr_con_current = vid.height;
    }

// notify the refresh of the change
    R_ViewChanged(&vrect, sb_lines, vid.aspect);
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f(void)
{
    Cvar_SetValue("viewsize",scr_viewsize.value+5);  // edited
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f(void)
{
    Cvar_SetValue("viewsize",scr_viewsize.value-5);  // edited
}

//============================================================================
/*
==================
SCR_Adjust
==================
*/
void SCR_AdjustFOV(void)
{
// bound field of view
    if (scr_fov.value < 10) {
        Cvar_Set("fov","10");
    }
    if (scr_fov.value > 170) {
        Cvar_Set("fov","170");
    }

    if (oldfov != scr_fov.value) {
        oldfov = scr_fov.value;
        vid.recalc_refdef = true;
    }
}
void SCR_Adjust(void)
{
    static float	oldscr_viewsize;
    static float	oldlcd_x;

    if (oldlcd_x != lcd_x.value) {
        oldlcd_x = lcd_x.value;
        vid.recalc_refdef = true;
    }

// bound viewsize
    // edited - begin
    if (scr_viewsize.value < 50) {	// 30
        Cvar_Set("viewsize","50");    // 30
    } else if (scr_viewsize.value > 100) {	// 120
        Cvar_Set("viewsize","100");    // 120
    }
    // edited - end
    if (scr_viewsize.value != oldscr_viewsize) {
        oldscr_viewsize = scr_viewsize.value;
        vid.recalc_refdef = 1;
    }

//
// check for vid changes
//
    // screen positioning - begin
    {
        int i = (int)((float)(vid.width) * (scr_left.value/100.0));
        if (scr_left.value > 100) { //100
            i = vid.width;
        }
        if (screen_left != i) {
            screen_left = i;
            vid.recalc_refdef = true;
        }

        i = (int)((float)(vid.width) * (scr_right.value/100.0));
        if (scr_right.value > 100) { //100
            i = vid.width;
        }
        if (screen_right != i) {
            screen_right = i;
            vid.recalc_refdef = true;
        }

        i = (int)((float)(vid.height) * (scr_top.value/100.0));
        if (scr_top.value > 100) { //100
            i = vid.height;
        }
        if (screen_top != i) {
            screen_top = i;
            vid.recalc_refdef = true;
        }

        i = (int)((float)(vid.height) * (scr_bottom.value/100.0));
        if (scr_bottom.value > 100) { //100
            i = vid.height;
        }
        if (screen_bottom != i) {
            screen_bottom = i;
            vid.recalc_refdef = true;
        }
    }
//	if (scr_con_current)
//		vid.recalc_refdef = true;
    // screen positioning - end
    /*	// r_letterbox - start
    	{
    	//	static float old_r_letterbox;
    		extern cvar_t r_letterbox;
    		if (r_letterbox.value < 0)
    			r_letterbox.value = 0;
    		else if (r_letterbox.value > 100)
    			r_letterbox.value = 100;
    	//	if (old_r_letterbox != r_letterbox.value)
    		{
    	//		old_r_letterbox = r_letterbox.value;
    			cl.letterbox = r_letterbox.value/100.0; // svc_letterbox
    			vid.recalc_refdef = true;
    		}
    	}
    */	// r_letterbox - end
}
/*
==================
SCR_Init
==================
*/
void SCR_Init(void)
{
    Cvar_RegisterVariableWithCallback(&scr_fov, SCR_AdjustFOV);  // Edited
    Cvar_RegisterVariableWithCallback(&scr_viewsize, SCR_Adjust);  // Edited
//	Cvar_RegisterVariable (&scr_fov);
//	Cvar_RegisterVariable (&scr_viewsize);
    Cvar_RegisterVariable(&scr_conspeed);
    Cvar_RegisterVariable(&scr_showram);
    Cvar_RegisterVariable(&scr_showturtle);
    Cvar_RegisterVariable(&scr_showpause);
    Cvar_RegisterVariable(&scr_centertime);
    Cvar_RegisterVariable(&scr_printspeed);
    // screen positioning - begin
    Cvar_RegisterVariableWithCallback(&scr_left, SCR_Adjust);
    Cvar_RegisterVariableWithCallback(&scr_right, SCR_Adjust);
    Cvar_RegisterVariableWithCallback(&scr_top, SCR_Adjust);
    Cvar_RegisterVariableWithCallback(&scr_bottom, SCR_Adjust);
    // screen positioning - end

//
// register our commands
//
    Cmd_AddCommand("screenshot",SCR_ScreenShot_f);
    Cmd_AddCommand("sizeup",SCR_SizeUp_f);
    Cmd_AddCommand("sizedown",SCR_SizeDown_f);

    scr_ram = Draw_PicFromWad("ram");
    scr_net = Draw_PicFromWad("net");
//	scr_turtle = Draw_PicFromWad ("turtle"); // removed

    scr_initialized = true;
}



/*
==============
SCR_DrawRam
==============
*/
void SCR_DrawRam(void)
{
    if (!scr_showram.value) {
        return;
    }

    if (!r_cache_thrash) {
        return;
    }

    Draw_TransPic(scr_vrect.x+32, scr_vrect.y, scr_ram);  // edited
}

/*
==============
SCR_DrawTurtle
==============
*/
/* // removed - begin
void SCR_DrawTurtle (void)
{
	static int	count;

	if (!scr_showturtle.value)
		return;

	if (host_frametime < 0.1)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}
*/ // removed - end

/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet(void)
{
    if (realtime - cl.last_received_message < 0.3) {
        return;
    }
    if (cls.demoplayback) {
        return;
    }

    Draw_TransPic(scr_vrect.x+64, scr_vrect.y, scr_net);  // edited
}

// 2001-11-31 FPS display by QuakeForge/Muff  start
/*
==============
SCR_DrawFPS
==============
*/
//muff - hacked out of SourceForge implementation + modified
void SCR_DrawFPS(void)
{
    static double lastframetime;
    double t;
    static int lastfps;
    int x;//, y; // edited
    char st[80];

    t = Sys_FloatTime();
    if ((t - lastframetime) >= 1.0) {
        lastfps = fps_count;
        fps_count = 0;
        lastframetime = t;
    }

    sprintf(st, "%3d FPS", lastfps);

    x = vid.width - strlen(st) * 8 - screen_right; // edited
    if (scr_vrect.y > screen_top) { // Edited
        Draw_Fill(x, screen_top, strlen(st) * 8, 8, 0);    // Edited
    }
    Draw_String(x, screen_top, st); // x, y, st // edited
}
// 2001-11-31 FPS display by QuakeForge/Muff  end

/*
==============
DrawPause
==============
*/
void SCR_DrawPause(void)
{
    if (!scr_showpause.value) {	// turn off for screenshots
        return;
    }

    if (!cl.paused) {
        return;
    }

    M_DrawPlaque("gfx/pause.lmp", false);  // Edited
}



/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading(void)
{
    qpic_t	*pic;

    if (!scr_drawloading) {
        return;
    }

    pic = Draw_CachePic("gfx/loading.lmp");
    Draw_TransPic((vid.width - pic->width)/2,    // edited
                  (vid.height - 48 - pic->height)/2, pic);
}



//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
void SCR_SetUpToDrawConsole(void)
{
    extern cvar_t	con_alpha; // transparent console
    Con_CheckResize();

    if (scr_drawloading) {
        return;    // never a console with loading plaque
    }

// decide on the height of the console
    con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

    if (con_forcedup) {
        scr_conlines = vid.height;		// full screen
        scr_con_current = scr_conlines;
    } else if (key_dest == key_console) {
        scr_conlines = vid.height/2;    // half screen
    } else {
        scr_conlines = 0;    // none visible
    }

    if (scr_conlines < scr_con_current) {
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//		scr_con_current -= scr_conspeed.value*host_frametime;
        scr_con_current -= scr_conspeed.value*host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
        if (scr_conlines > scr_con_current) {
            scr_con_current = scr_conlines;
        }

    } else if (scr_conlines > scr_con_current) {
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//		scr_con_current += scr_conspeed.value*host_frametime;
        scr_con_current += scr_conspeed.value*host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
        if (scr_conlines < scr_con_current) {
            scr_con_current = scr_conlines;
        }
    }

    if (clearconsole++ < vid.numpages) {
        scr_copytop = 1;
        if (con_alpha.value == 1.0) { // transparent console
            Draw_Fill(0, (int)scr_con_current, vid.width, vid.height-(int)scr_con_current, 0);    // edited
        } else { // transparent console
            Draw_Fill(0, 0, vid.width, vid.height, 0);    // transparent console
        }
        Sbar_Changed();
    } else if (clearnotify++ < vid.numpages && scr_vrect.y > screen_top) { // edited
        scr_copytop = 1;
        Draw_Fill(0, screen_top, vid.width, con_notifylines, 0);  // edited
    } else {
        con_notifylines = 0;
    }
}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole(void)
{
    if (scr_con_current) {
        scr_copyeverything = 1;
        Con_DrawConsole(scr_con_current, true);
        clearconsole = 0;
    } else {
        if (key_dest == key_game || key_dest == key_message) {
            Con_DrawNotify();    // only draw notify in game
        }
    }
}


/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/


typedef struct {
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data;			// unbounded
} pcx_t;

// skyboxes - begin
/*
============
LoadPCX
============
*/
void LoadPCX(char *filename, byte **pic, int *width, int *height)
{
    pcx_t	*pcx;
    byte	*pcxbuf, *out, *pix;
    int		x, y;
    int		dataByte, runLength;
    loadedfile_t	*fileinfo; // Edited

    *pic = NULL;
    // begin
//	pcxbuf = COM_LoadTempFile (filename); // removed
//	if (!pcxbuf) // removed
    fileinfo = COM_LoadTempFile(filename);
    if (!fileinfo)
        // end
    {
        return;
    }
    pcxbuf = (char *)fileinfo->data; // Edited

//
// parse the PCX file
//
    pcx = (pcx_t *)pcxbuf;
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

    pix = &pcx->data;

    if (pcx->manufacturer != 0x0a
            || pcx->version != 5
            || pcx->encoding != 1
            || pcx->bits_per_pixel != 8) // edited
        //	|| pcx->xmax >= 640 // removed
        //	|| pcx->ymax >= 480) // removed
    {
        Con_Printf("Bad pcx file\n");
        return;
    }

    if (width) {
        *width = pcx->xmax+1;
    }
    if (height) {
        *height = pcx->ymax+1;
    }

    *pic = out = malloc((pcx->xmax+1) * (pcx->ymax+1));

    for (y=0 ; y<=pcx->ymax ; y++, out += pcx->xmax+1) {
        for (x=0 ; x<=pcx->xmax ;) {
            dataByte = *pix++;

            if ((dataByte & 0xC0) == 0xC0) {
                runLength = dataByte & 0x3F;
                dataByte = *pix++;
            } else {
                runLength = 1;
            }

            while (runLength-- > 0) {
                out[x++] = dataByte;
            }
        }
    }
}
// skyboxes - end

/*
==============
WritePCXfile
==============
*/
void WritePCXfile(char *filename, byte *data, int width, int height,
                  int rowbytes, byte *palette)
{
    int		i, j, length;
    pcx_t	*pcx;
    byte		*pack;

    pcx = Hunk_TempAlloc(width*height*2+1000);
    if (pcx == NULL) {
        Con_Printf("SCR_ScreenShot_f: not enough memory\n");
        return;
    }

    pcx->manufacturer = 0x0a;	// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;		// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = LittleShort((short)(width-1));
    pcx->ymax = LittleShort((short)(height-1));
    pcx->hres = LittleShort((short)width);
    pcx->vres = LittleShort((short)height);
    Q_memset(pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = LittleShort((short)width);
    pcx->palette_type = LittleShort(2);		// not a grey scale
    Q_memset(pcx->filler,0,sizeof(pcx->filler));

// pack the image
    pack = &pcx->data;

    for (i=0 ; i<height ; i++) {
        for (j=0 ; j<width ; j++) {
            if ((*data & 0xc0) != 0xc0) {
                *pack++ = *data++;
            } else {
                *pack++ = 0xc1;
                *pack++ = *data++;
            }
        }

        data += rowbytes - width;
    }

// write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++) {
        *pack++ = *palette++;
    }

// write output file
    length = pack - (byte *)pcx;
    COM_WriteFile(filename, pcx, length);
}



/*
==================
SCR_ScreenShot_f
==================
*/
void SCR_ScreenShot_f(void)
{
    return;
}


//=============================================================================


/*
===============
SCR_BeginLoadingPlaque

================
*/
void SCR_BeginLoadingPlaque(void)
{
    S_StopAllSounds(true);

    if (cls.state != ca_connected) {
        return;
    }
    if (cls.signon != SIGNONS) {
        return;
    }

    Vibration_Stop(0);  // Edited
    Vibration_Stop(1);  // Edited
// redraw with no console and the loading plaque
    Con_ClearNotify();
    scr_centertime_off = 0;
    scr_con_current = 0;

    scr_drawloading = true;
    scr_fullupdate = 0;
    Sbar_Changed();
    SCR_UpdateScreen();
    scr_drawloading = false;

    scr_disabled_for_loading = true;
    scr_disabled_time = realtime;
    scr_fullupdate = 0;
}

/*
===============
SCR_EndLoadingPlaque

================
*/
void SCR_EndLoadingPlaque(void)
{
    scr_disabled_for_loading = false;
    scr_fullupdate = 0;
    Con_ClearNotify();
}

//=============================================================================

char	*scr_notifystring;
qboolean	scr_drawdialog;

void SCR_DrawNotifyString(void)
{
    char	*start;
    int		l;
    int		j;
    int		x, y;

    start = scr_notifystring;

    y = vid.height*0.35;

    do {
        // scan the width of the line
        for (l=0 ; l<40 ; l++)
            if (start[l] == '\n' || !start[l]) {
                break;
            }
        x = (vid.width - l*8)/2;
        for (j=0 ; j<l ; j++, x+=8) {
            Draw_Character(x, y, start[j]);
        }

        y += 8;

        while (*start && *start != '\n') {
            start++;
        }

        if (!*start) {
            break;
        }
        start++;		// skip the \n
    } while (1);
}

/*
==================
SCR_ModalMessage

Displays a text string in the center of the screen and waits for a Y or N
keypress.
==================
*/
int SCR_ModalMessage(char *text)
{
#ifdef FLASH
    return true;
#endif

    if (cls.state == ca_dedicated) {
        return true;
    }

    scr_notifystring = text;

// draw a fresh screen
    scr_fullupdate = 0;
    scr_drawdialog = true;
    SCR_UpdateScreen();
    scr_drawdialog = false;

    S_ClearBuffer();		// so dma doesn't loop current sound

    do {
        key_count = -1;		// wait for a key down and up
        Sys_SendKeyEvents();
    } while (key_lastpress != 'y' && key_lastpress != 'n' && key_lastpress != K_ESCAPE);

    scr_fullupdate = 0;
    SCR_UpdateScreen();

    return key_lastpress == 'y';
}


//=============================================================================

/*
===============
SCR_BringDownConsole

Brings the console down and fades the palettes back to normal
================
*/
void SCR_BringDownConsole(void)
{
    int		i;

    scr_centertime_off = 0;

    for (i=0 ; i<20 && scr_conlines != scr_con_current ; i++) {
        SCR_UpdateScreen();
    }

    cl.cshifts[0].percent = 0;		// no area contents palette on next frame
    VID_SetPalette(host_basepal);
}


/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.

WARNING: be very careful calling this from elsewhere, because the refresh
needs almost the entire 256k of stack space!
==================
*/
void SCR_UpdateScreen(void)
{
    vrect_t		vrect;

    if (scr_skipupdate || block_drawing) {
        return;
    }

    scr_copytop = 0;
    scr_copyeverything = 0;

    if (scr_disabled_for_loading) {
        if (realtime - scr_disabled_time > 60) {
            scr_disabled_for_loading = false;
            Con_Printf("load failed.\n");
        } else {
            return;
        }
    }

    if (cls.state == ca_dedicated) {
        return;    // stdout only
    }

    if (!scr_initialized || !con_initialized) {
        return;    // not initialized yet
    }

    // removi código daqui

    // start
    {
        static byte old_sb_lines;
        if (sb_lines != old_sb_lines) {
            old_sb_lines = sb_lines;
            vid.recalc_refdef = true;
        }
    }
    // end

    if (vid.recalc_refdef) {
        // something changed, so reorder the screen
        SCR_CalcRefdef();
    }

//
// do 3D refresh drawing, and then update the screen
//
    D_EnableBackBufferAccess();	// of all overlay stuff if drawing directly

    if (scr_fullupdate++ < vid.numpages) {
        // clear the entire screen
        scr_copyeverything = 1;
        Draw_Fill(0,0,vid.width,vid.height, 0);  // edited
        //	Sbar_Changed ();
    }

    pconupdate = NULL;


    SCR_SetUpToDrawConsole();
    SCR_EraseCenterString();

    D_DisableBackBufferAccess();	// for adapters that can't stay mapped in
    //  for linear writes all the time

    VID_LockBuffer();

    V_RenderView();

    VID_UnlockBuffer();

    D_EnableBackBufferAccess();	// of all overlay stuff if drawing directly

    if (scr_drawdialog) {
        Sbar_Draw();
        Draw_FadeScreen();
        SCR_DrawNotifyString();
        scr_copyeverything = true;
    } else if (scr_drawloading) {
        SCR_DrawLoading();
//		Sbar_Draw (); // removed
    } else if (cl.intermission == 1 && key_dest == key_game) {
        Sbar_IntermissionOverlay();
    } else if (cl.intermission == 2 && key_dest == key_game) {
        Sbar_FinaleOverlay();
        SCR_CheckDrawCenterString();
    } else if (cl.intermission == 3 && key_dest == key_game) {
        SCR_CheckDrawCenterString();
    } else {
        // SCR_DrawRam ();
        // SCR_DrawNet ();
        //	SCR_DrawTurtle (); // removed
        SCR_DrawPause();
        SCR_CheckDrawCenterString();
        Sbar_Draw();
        SCR_DrawConsole();
        M_Draw();
        if (cl_showfps.value) {
            SCR_DrawFPS();    // 2001-11-31 FPS display by QuakeForge/Muff
        }
    }

    D_DisableBackBufferAccess();	// for adapters that can't stay mapped in
    //  for linear writes all the time
    if (pconupdate) {
        D_UpdateRects(pconupdate);
    }

    V_UpdatePalette();

//
// update one of three areas
//
    if (scr_copyeverything) {
        vrect.x = 0;
        vrect.y = 0;
        vrect.width = vid.width;
        vrect.height = vid.height;
        vrect.pnext = 0;

        VID_Update(&vrect);
    } else if (scr_copytop) {
        vrect.x = 0;
        vrect.y = 0;
        vrect.width = vid.width;
        vrect.height = vid.height - sb_lines - screen_bottom; // edited
        vrect.pnext = 0;

        VID_Update(&vrect);
    } else {
        vrect.x = scr_vrect.x;
        vrect.y = scr_vrect.y;
        vrect.width = scr_vrect.width;
        vrect.height = scr_vrect.height;
        vrect.pnext = 0;

        VID_Update(&vrect);
    }
}


/*
==================
SCR_UpdateWholeScreen
==================
*/
void SCR_UpdateWholeScreen(void)
{
    scr_fullupdate = 0;
    SCR_UpdateScreen();
}
