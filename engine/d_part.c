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
// d_part.c: software driver module for drawing particles

#include "quakedef.h"
#include "d_local.h"


/*
==============
D_EndParticles
==============
*/
void D_EndParticles(void)
{
// not used by software driver
}


/*
==============
D_StartParticles
==============
*/
void D_StartParticles(void)
{
// not used by software driver
}


//#if	!id386 // enabled C particle renderer

// begin
byte	dottexture3[3][3] = {
    {0,1,0},
    {1,1,1},
    {0,1,0},
};

byte	dottexture4[4][4] = {
    {0,1,1,0},
    {1,1,1,1},
    {1,1,1,1},
    {0,1,1,0},
};

byte	dottexture5[5][5] = {
    {0,1,1,1,0},
    {1,1,1,1,1},
    {1,1,1,1,1},
    {1,1,1,1,1},
    {0,1,1,1,0},
};

byte	dottexture6[6][6] = {
    {0,0,1,1,0,0},
    {0,1,1,1,1,0},
    {1,1,1,1,1,1},
    {1,1,1,1,1,1},
    {0,1,1,1,1,0},
    {0,0,1,1,0,0},
};

byte	dottexture7[7][7] = {
    {0,0,1,1,1,0,0},
    {0,1,1,1,1,1,0},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},
};

byte	dottexture8[8][8] = {
    {0,0,0,1,1,0,0,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,0,0,1,1,0,0,0},
};

byte	dottexture9[9][9] = {
    {0,0,0,0,1,0,0,0,0},
    {0,0,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,0,0},
    {0,0,0,0,1,0,0,0,0},
};

byte	dottexture10[10][10] = {
    {0,0,0,0,1,1,0,0,0,0},
    {0,0,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,0,0},
    {0,0,0,0,1,1,0,0,0,0},
};

byte	dottexture11[11][11] = {
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,0},

    {1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
};

byte	dottexture12[12][12] = {
    {0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},

    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},

    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,1,1,1,1,0,0,0,0},
};

byte	dottexture13[13][13] = {
    {0,0,0,0,1,1,1,1,1,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,0},

    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,1,0,0},

    {0,0,0,0,1,1,1,1,1,0,0,0,0},
};

byte	dottexture14[14][14] = {
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,0,0},

    {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0},
};

byte	dottexture15[15][15] = {
    {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0},
};

byte	dottexture16[16][16] = {
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
};

byte	dottexture17[17][17] = {
    {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0},

    {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},
};

byte	dottexture18[18][18] = {
    {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},

    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},
};
// end

/*
==============
D_DrawParticle
==============
*/

void D_DrawParticle_A(particle_t *pparticle,float alpha)  // Edited
{
    vec3_t	local, transformed;
    float	zi;
    pixel_t	*pdest;
    short	*pz;
    int		i, izi, pix, count, u, v;
    byte	*dottexture; // Edited
    pixel_t	PartCol;
    int		R,G,B;
    int		iA	=	(int)(255.f*alpha);

// transform point
    VectorSubtract(pparticle->org, r_origin, local);

    transformed[0] = DotProduct(local, r_pright);
    transformed[1] = DotProduct(local, r_pup);
    transformed[2] = DotProduct(local, r_ppn);

    if (transformed[2] < PARTICLE_Z_CLIP) {
        return;
    }

// project the point
// FIXME: preadjust xcenter and ycenter
    zi = 1.0 / transformed[2];
    u = (int)(xcenter + zi * transformed[0] + 0.5);
    v = (int)(ycenter - zi * transformed[1] + 0.5);

    izi = (int)(zi * 0x8000);

    pix = (int)(izi * fovscale) >> d_pix_shift; // FOV-based scaling - fixed

    if (pix < d_pix_min) {
        pix = d_pix_min;
    } else if (pix > (d_pix_max)) {
        pix = (int)(d_pix_max);
    }

    if ((v > (d_vrectbottom_particle - (pix << d_y_aspect_shift))) ||  // FOV-based scaling - fixed
            (u > (d_vrectright_particle - pix)) || // FOV-based scaling - fixed
            (v < d_vrecty) ||
            (u < d_vrectx)) {
        return;
    }

    PartCol	=	d_8to24table[(int)(pparticle->color)];
    R		=	(PartCol&0xff)*iA;
    G		=	((PartCol>>8)&0xff)*iA;
    B		=	((PartCol>>16)&0xff)*iA;
    iA		=	0xff-iA;

#define PART_BLEND(X)	(((X&0xff)*iA+R)>>8)|((((X>>8)&0xff)*iA+G)&0xff00)|(((((X>>16)&0xff)*iA+B)>>8)<<16)

    pz = d_pzbuffer + (d_zwidth * v) + u;
    pdest = d_viewbuffer + d_scantable[v] + u;

    // begin
    if (pix == 1) {
        if (pz[0] <= izi) {
            pz[0] = izi;
            pdest[0] = alphamap[(int)(pdest[0] + pparticle->color*256)];
            if (d_y_aspect_shift) {
                pz[d_zwidth] = izi;
//                pdest[screenwidth] = alphamap[(int)(pdest[screenwidth] + pparticle->color*256)];
                pdest[screenwidth] = PART_BLEND(pdest[screenwidth]);
            }
        }
    } else if (pix == 2) {
        if (pz[0] <= izi) {
            pz[0] = izi;
            pdest[0] = PART_BLEND(pdest[0]);
            pz[1] = izi;
            pdest[1] = PART_BLEND(pdest[1]);
            pz[d_zwidth] = izi;
            pdest[screenwidth] = PART_BLEND(pdest[screenwidth]);
            pz[d_zwidth+1] = izi;
            pdest[screenwidth+1] = PART_BLEND(pdest[screenwidth+1]);
            if (d_y_aspect_shift) {
                pz += d_zwidth*2;
                pdest += screenwidth*2;
                pz[0] = izi;
                pdest[0] = PART_BLEND(pdest[0]);
                pz[1] = izi;
                pdest[1] = PART_BLEND(pdest[1]);
                pz[d_zwidth] = izi;
                pdest[screenwidth] = PART_BLEND(pdest[screenwidth]);
                pz[d_zwidth+1] = izi;
                pdest[screenwidth+1] = PART_BLEND(pdest[screenwidth+1]);
            }
        }
    } else {
        switch (pix) {
        case  3:
            dottexture = dottexture3[0];
            break;
        case  4:
            dottexture = dottexture4[0];
            break;
        case  5:
            dottexture = dottexture5[0];
            break;
        case  6:
            dottexture = dottexture6[0];
            break;
        case  7:
            dottexture = dottexture7[0];
            break;
        case  8:
            dottexture = dottexture8[0];
            break;
        case  9:
            dottexture = dottexture9[0];
            break;
        case 10:
            dottexture = dottexture10[0];
            break;
        case 11:
            dottexture = dottexture11[0];
            break;
        case 12:
            dottexture = dottexture12[0];
            break;
        case 13:
            dottexture = dottexture13[0];
            break;
        case 14:
            dottexture = dottexture14[0];
            break;
        case 15:
            dottexture = dottexture15[0];
            break;
        case 16:
            dottexture = dottexture16[0];
            break;
        case 17:
            dottexture = dottexture17[0];
            break;
        default:
            pix = 18;// for 1600x1200, bleh :P
        case 18:
            dottexture = dottexture18[0];
            break;
        }
        if (pz[pix/2] <= izi) {
            count = pix;
            if (d_y_aspect_shift) {
                pixel_t	*pdest2;
                short	*pz2;
                byte	*dot_texture;
                pdest2 = pdest + screenwidth;
                pz2 = pz + d_zwidth;
                dot_texture = dottexture;
                for (; count ; count--, pz += d_zwidth*2, pdest += screenwidth*2)
                    for (i=0 ; i<pix ; i++)
                        if (*(dottexture++)) {
                            pz[i] = izi;
                            pdest[i] = PART_BLEND(pdest[i]);
                        }
                for (count = pix ; count ; count--, pz2 += d_zwidth*2, pdest2 += screenwidth*2)
                    for (i=0 ; i<pix ; i++)
                        if (*(dot_texture++)) {
                            pz2[i] = izi;
                            pdest2[i] = PART_BLEND(pdest2[i]);
                        }
            } else {
                for (; count ; count--, pz += d_zwidth, pdest += screenwidth)
                    for (i=0 ; i<pix ; i++)
                        if (*(dottexture++)) {
                            pz[i] = izi;
                            pdest[i] = PART_BLEND(pdest[i]);
                        }
            }
        }
    }
    // end
}
//void D_DrawParticle (particle_t *pparticle)
void D_DrawParticle_C(particle_t *pparticle)  // renamed to not conflict with the asm version
{
    vec3_t	local, transformed;
    float	zi;
    pixel_t	*pdest;
    short	*pz;
    int		i, izi, pix, count, u, v;
    byte	*dottexture; // Edited

// transform point
    VectorSubtract(pparticle->org, r_origin, local);

    transformed[0] = DotProduct(local, r_pright);
    transformed[1] = DotProduct(local, r_pup);
    transformed[2] = DotProduct(local, r_ppn);

    if (transformed[2] < PARTICLE_Z_CLIP) {
        return;
    }

// project the point
// FIXME: preadjust xcenter and ycenter
    zi = 1.0 / transformed[2];
    u = (int)(xcenter + zi * transformed[0] + 0.5);
    v = (int)(ycenter - zi * transformed[1] + 0.5);

    izi = (int)(zi * 0x8000);

    pix = (int)(izi * fovscale) >> d_pix_shift; // FOV-based scaling - fixed

    if (pix < d_pix_min) {
        pix = d_pix_min;
    } else if (pix > (d_pix_max)) {
        pix = (int)(d_pix_max);
    }

    if ((v > (d_vrectbottom_particle - (pix << d_y_aspect_shift))) ||  // FOV-based scaling - fixed
            (u > (d_vrectright_particle - pix)) || // FOV-based scaling - fixed
            (v < d_vrecty) ||
            (u < d_vrectx)) {
        return;
    }

    pz = d_pzbuffer + (d_zwidth * v) + u;
    pdest = d_viewbuffer + d_scantable[v] + u;

    // begin
    if (pix == 1) {
        if (pz[0] <= izi) {
            pz[0] = izi;
            pdest[0] = pparticle->color;
            if (d_y_aspect_shift) {
                pz[d_zwidth] = izi;
                pdest[screenwidth] = pparticle->color;
            }
        }
    } else if (pix == 2) {
        if (pz[0] <= izi) {
            pz[0] = izi;
            pdest[0] = pparticle->color;
            pz[1] = izi;
            pdest[1] = pparticle->color;
            pz[d_zwidth] = izi;
            pdest[screenwidth] = pparticle->color;
            pz[d_zwidth+1] = izi;
            pdest[screenwidth+1] = pparticle->color;
            if (d_y_aspect_shift) {
                pz += d_zwidth*2;
                pdest += screenwidth*2;
                pz[0] = izi;
                pdest[0] = pparticle->color;
                pz[1] = izi;
                pdest[1] = pparticle->color;
                pz[d_zwidth] = izi;
                pdest[screenwidth] = pparticle->color;
                pz[d_zwidth+1] = izi;
                pdest[screenwidth+1] = pparticle->color;
            }
        }
    } else {
        switch (pix) {
        case  3:
            dottexture = dottexture3[0];
            break;
        case  4:
            dottexture = dottexture4[0];
            break;
        case  5:
            dottexture = dottexture5[0];
            break;
        case  6:
            dottexture = dottexture6[0];
            break;
        case  7:
            dottexture = dottexture7[0];
            break;
        case  8:
            dottexture = dottexture8[0];
            break;
        case  9:
            dottexture = dottexture9[0];
            break;
        case 10:
            dottexture = dottexture10[0];
            break;
        case 11:
            dottexture = dottexture11[0];
            break;
        case 12:
            dottexture = dottexture12[0];
            break;
        case 13:
            dottexture = dottexture13[0];
            break;
        case 14:
            dottexture = dottexture14[0];
            break;
        case 15:
            dottexture = dottexture15[0];
            break;
        case 16:
            dottexture = dottexture16[0];
            break;
        case 17:
            dottexture = dottexture17[0];
            break;
        default:
            pix = 18;// for 1600x1200, bleh :P
        case 18:
            dottexture = dottexture18[0];
            break;
        }
        if (pz[pix/2] <= izi) {
            count = pix;
            if (d_y_aspect_shift) {
                pixel_t	*pdest2;
                short	*pz2;
                byte	*dot_texture;
                pdest2 = pdest + screenwidth;
                pz2 = pz + d_zwidth;
                dot_texture = dottexture;
                for (; count ; count--, pz += d_zwidth*2, pdest += screenwidth*2)
                    for (i=0 ; i<pix ; i++)
                        if (*(dottexture++)) {
                            pz[i] = izi;
                            pdest[i] = pparticle->color;
                        }
                for (count = pix ; count ; count--, pz2 += d_zwidth*2, pdest2 += screenwidth*2)
                    for (i=0 ; i<pix ; i++)
                        if (*(dot_texture++)) {
                            pz2[i] = izi;
                            pdest2[i] = pparticle->color;
                        }
            } else {
                for (; count ; count--, pz += d_zwidth, pdest += screenwidth)
                    for (i=0 ; i<pix ; i++)
                        if (*(dottexture++)) {
                            pz[i] = izi;
                            pdest[i] = pparticle->color;
                        }
            }
        }
    }
    // end
    /*
    //	if (*(dottexture + (((count-1)*pix)+(pix-i-1))))

    	case 3:
    		if (pz[1] <= izi)
    		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
    		{
    			for (i=0 ; i<pix ; i++)
    				if (dottexture3[count-1][pix-i-1])
    				{
    					pz[i] = izi;
    					pdest[i] = pparticle->color;
    				}
    			if (d_y_aspect_shift)
    			{
    				pz += d_zwidth;
    				pdest += screenwidth;
    				for (i=0 ; i<pix ; i++)
    					if (dottexture3[count-1][pix-i-1])
    					{
    						pz[i] = izi;
    						pdest[i] = pparticle->color;
    					}
    			}
    		}
    		break;

    	default:
    		if (pz[pix/2] <= izi)
    		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
    			for (i=0 ; i<pix ; i++)
    			{
    				pz[i] = izi;
    				pdest[i] = pparticle->color;
    			}
    		break;
    	}
    */
}

//#endif	// !id386

