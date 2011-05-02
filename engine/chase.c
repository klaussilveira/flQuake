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
// chase.c -- chase camera code

#include "quakedef.h"

extern qboolean SV_RecursiveHullCheck(hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);

cvar_t	chase_back = {"chase_back", "100", true}; // edited
cvar_t	chase_up = {"chase_up", "16", true}; // edited
cvar_t	chase_right = {"chase_right", "0"};
cvar_t	chase_active = {"chase_active", "0", true}; // edited

/* // removed - begin
vec3_t	chase_pos;
vec3_t	chase_angles;

vec3_t	chase_dest;
vec3_t	chase_dest_angles;
*/ // removed - end

void Chase_Init(void)
{
    Cvar_RegisterVariable(&chase_back);
    Cvar_RegisterVariable(&chase_up);
    Cvar_RegisterVariable(&chase_right);
    Cvar_RegisterVariable(&chase_active);
}

/* // removed - begin
void Chase_Reset (void)
{
	// for respawning and teleporting
//	start position 12 units behind head
}

void TraceLine (vec3_t start, vec3_t end, vec3_t impact)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
}
*/ // removed - end

void Chase_Update(void)
{
    int		i;
//	float	dist; // removed
    vec3_t	forward, up, right;
//	vec3_t	dest, stop; // removed
    // begin
    vec3_t	chase_origin1, chase_origin2, chase_origin3, chase_origin4;
    vec3_t	chase_dest1, chase_dest2, chase_dest3, chase_dest4;
    vec3_t	chase_vec;
    trace_t	trace1, trace2, trace3, trace4, *trace;
    // end


    // if can't see player, reset
    AngleVectors(cl.viewangles, forward, right, up);

    // calc exact destination
    /*
    for (i=0 ; i<3 ; i++)
    	chase_dest[i] = r_refdef.vieworg[i]
    	+ up[i]*chase_up.value // Edited
    	- forward[i]*chase_back.value
    	- right[i]*chase_right.value;
    */
// begin
    for (i=0 ; i<3 ; i++) {
        chase_origin1[i] = r_refdef.vieworg[i] - right[i] + up[i];
        chase_origin2[i] = r_refdef.vieworg[i] + right[i] - up[i];
        chase_origin3[i] = r_refdef.vieworg[i] + right[i] + up[i];
        chase_origin4[i] = r_refdef.vieworg[i] - right[i] - up[i];

        chase_dest1[i] = r_refdef.vieworg[i]
                         - forward[i]*(chase_back.value)
                         - right[i]	*(chase_right.value	+1)
                         + up[i]		*(chase_up.value	+1);

        chase_dest2[i] = r_refdef.vieworg[i]
                         - forward[i]*(chase_back.value)
                         - right[i]	*(chase_right.value	-1)
                         + up[i]		*(chase_up.value	-1);

        chase_dest3[i] = r_refdef.vieworg[i]
                         - forward[i]*(chase_back.value)
                         - right[i]	*(chase_right.value	-1)
                         + up[i]		*(chase_up.value	+1);

        chase_dest4[i] = r_refdef.vieworg[i]
                         - forward[i]*(chase_back.value)
                         - right[i]	*(chase_right.value	+1)
                         + up[i]		*(chase_up.value	-1);
    }

    if (sv.active) {
        // hack to prevent the camera from seeing through solid objects.
        // not all solid objects are detected, and it crashes on demo play (hence the "if (sv.active)").
        // start, mins, maxs, end, nomonsters, ignore_ent
        trace1 = SV_Move(chase_origin1, vec3_origin, vec3_origin, chase_dest1, false, G_EDICT(cl.viewentity));
        trace2 = SV_Move(chase_origin2, vec3_origin, vec3_origin, chase_dest2, false, G_EDICT(cl.viewentity));
        trace3 = SV_Move(chase_origin3, vec3_origin, vec3_origin, chase_dest3, false, G_EDICT(cl.viewentity));
        trace4 = SV_Move(chase_origin4, vec3_origin, vec3_origin, chase_dest4, false, G_EDICT(cl.viewentity));
    } else {
        memset(&trace1, 0, sizeof(trace1));
        memset(&trace2, 0, sizeof(trace2));
        memset(&trace3, 0, sizeof(trace3));
        memset(&trace4, 0, sizeof(trace4));
        SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, chase_origin1, chase_dest1, &trace1);
        SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, chase_origin2, chase_dest2, &trace2);
        SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, chase_origin3, chase_dest3, &trace3);
        SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, chase_origin4, chase_dest4, &trace4);
    }

    // get the one that's closer to the camera
    if (trace1.fraction < trace2.fraction) {
        trace = &trace1;
    } else {
        trace = &trace2;
    }
    if (trace3.fraction < trace->fraction) {
        trace = &trace3;
    }
    if (trace4.fraction < trace->fraction) {
        trace = &trace4;
    }

    if (Length(trace->endpos) == 0) // didn't hit anything
        for (i=0 ; i<3 ; i++)
            r_refdef.vieworg[i] +=
                - forward[i]*chase_back.value
                - right[i]	*chase_right.value
                + up[i]		*chase_up.value;
    else {
        chase_vec[0] = chase_back.value;
        chase_vec[1] = chase_right.value;
        chase_vec[2] = chase_up.value;
        VectorScale(chase_vec, trace->fraction, chase_vec);
        for (i=0 ; i<3 ; i++)
            r_refdef.vieworg[i] +=
                - forward[i]*chase_vec[0]
                - right[i]	*chase_vec[1]
                + up[i]		*chase_vec[2];
    }
    /*
    	if (Length(trace.endpos) == 0) // didn't hit anything
    	{
    		VectorCopy (chase_dest, r_refdef.vieworg);
    	}
    	else
    		VectorCopy (trace.endpos, r_refdef.vieworg);
    */
// end

    /* // removed - begin
    	chase_dest[2] = r_refdef.vieworg[2] + chase_up.value;

    	// find the spot the player is looking at
    	VectorMA (r_refdef.vieworg, 4096, forward, dest);
    	TraceLine (r_refdef.vieworg, dest, stop);

    	// calculate pitch to look at the same spot from camera
    	VectorSubtract (stop, r_refdef.vieworg, stop);
    	dist = DotProduct (stop, forward);
    	if (dist < 1)
    		dist = 1;
    	r_refdef.viewangles[PITCH] = -atan(stop[2] / dist) / M_PI * 180;

    	// move towards destination
    	VectorCopy (chase_dest, r_refdef.vieworg);

    */ // removed - end
}

