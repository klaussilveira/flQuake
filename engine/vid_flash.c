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
// vid_flash.c -- Flash video driver

#include "quakedef.h"
#include "d_local.h"

viddef_t	vid;

#define SURFCACHE_SIZE_AT_640X480	4108800

#define	BASEWIDTH	640
#define	BASEHEIGHT	480
short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[SURFCACHE_SIZE_AT_640X480];

unsigned int _vidBuffer4b[BASEWIDTH*BASEHEIGHT];

unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];

void	VID_SetPalette(unsigned char *palette)
{
    int a,b;
    unsigned int Pal[256];
    for (a=0; a<256; a++) {
        b		=	(*(int*)&(palette[a*3]))&0xffffff;
        b		=	((b&0xff)<<16)|((b&0xff00))|((b&0xff0000)>>16);
        Pal[a]	=	b;
    }
    Pal[255]	=	0xdeadbeaf; // transparency
    memcpy(d_8to24table,Pal,sizeof(d_8to24table));
}

void	VID_ShiftPalette(unsigned char *palette)
{
    VID_SetPalette(palette);
}

void	VID_Init(unsigned char *palette)
{
    vid.width = vid.conwidth = BASEWIDTH;
    vid.height = vid.conheight = BASEHEIGHT;
    vid.maxwarpwidth = vid.width;
    vid.maxwarpheight = vid.height;
    vid.numpages = 1;
    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
    vid.buffer = vid.conbuffer = _vidBuffer4b;
    vid.rowbytes = vid.conrowbytes = BASEWIDTH;

    d_pzbuffer = zbuffer;
    D_InitCaches(surfcache, sizeof(surfcache));
    vid.recalc_refdef = 1;

    VID_SetPalette(palette);
}

void VID_Shutdown(void)
{
}

void VID_Update(vrect_t *rects)
{
}

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect(int x, int y, int width, int height)
{
}

void VID_HandlePause(qboolean pause)
{
}
