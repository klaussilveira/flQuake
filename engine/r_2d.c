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

// begin
// I was going to put all the 2D drawing functions here, but now this is file
// contains only some cvars common to both the software and hardware renderers.
#include "quakedef.h"

cvar_t	r_interpolation = {"r_interpolation", "1"}; // model interpolation
//cvar_t	r_wateralpha = {"r_wateralpha","0.6", true}; // translucent water
cvar_t	r_particlealpha = {"r_particlealpha","0", true};
cvar_t	sw_stipplealpha = {"sw_stipplealpha","1", true};
cvar_t	r_sprite_addblend = {"r_sprite_addblend","0", true};

// screen positioning - begin
#ifdef _arch_dreamcast
cvar_t		scr_left	= {"scr_left",	"4", true};		// 12 pixels in 320x240 (2.5 = 8)
cvar_t		scr_right	= {"scr_right",	"4", true};		// 12 pixels in 320x240 (2.5 = 8)
cvar_t		scr_top		= {"scr_top",	"5", true};		// 12 pixels in 320x240
cvar_t		scr_bottom	= {"scr_bottom","8.5", true};	// 20 pixels in 320x240
#else
cvar_t		scr_left	= {"scr_left",	"1", true};
cvar_t		scr_right	= {"scr_right",	"1", true};
cvar_t		scr_top		= {"scr_top",	"1", true};
cvar_t		scr_bottom	= {"scr_bottom","1", true};
#endif
// screen positioning - end
// end
