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
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

pixel_t			*r_turb_pbase, *r_turb_pdest;
fixed16_t		r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int				*r_turb_turb;
int				r_turb_spancount;

void D_DrawTurbulent8Span(void);


/*
=============
D_WarpScreen

// this performs a slight compression of the screen at the same time as
// the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen(void)
{
    int		w, h;
    int		u,v;
    pixel_t	*dest;
    int		*turb;
    int		*col;
    pixel_t	**row;
    pixel_t	*rowptr[MAXHEIGHT+(AMP2*2)];
    int		column[MAXWIDTH+(AMP2*2)];
    float	wratio, hratio;

    w = r_refdef.vrect.width;
    h = r_refdef.vrect.height;

    wratio = w / (float)scr_vrect.width;
    hratio = h / (float)scr_vrect.height;

    for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++) {
        rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
                    (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
    }

    for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++) {
        column[u] = r_refdef.vrect.x +
                    (int)((float)u * wratio * w / (w + AMP2 * 2));
    }

    turb = intsintable + ((int)(cl.time*SPEED)&(CYCLE-1));
    dest = vid.buffer + scr_vrect.y * vid.width + scr_vrect.x;

    for (v=0 ; v<scr_vrect.height ; v++, dest += vid.width) {
        col = &column[turb[v]];
        row = &rowptr[v];

        for (u=0 ; u<scr_vrect.width ; u+=4) {
            dest[u+0] = row[turb[u+0]][col[u+0]];
            dest[u+1] = row[turb[u+1]][col[u+1]];
            dest[u+2] = row[turb[u+2]][col[u+2]];
            dest[u+3] = row[turb[u+3]][col[u+3]];
        }
    }
}


#if	!id386

/*
=============
D_DrawTurbulent8Span
=============
*/
void D_DrawTurbulent8Span(void)
{
//	int		sturb, tturb;

    do {
#define sturb (((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63) // edited
#define tturb (((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63) // edited
        *r_turb_pdest++ = *(r_turb_pbase + (tturb<<6) + sturb);
        r_turb_s += r_turb_sstep;
        r_turb_t += r_turb_tstep;
    } while (--r_turb_spancount > 0);
#undef sturb
#undef tturb
}

#endif	// !id386


/*
=============
Turbulent8
=============
*/
void Turbulent8(espan_t *pspan)
{
    int				count;
    int				izi, izistep, izistep2, sturb, tturb, teste; // translucent water
    short			*pz; // translucent water
    fixed16_t		snext, tnext;
    float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
    float			sdivz16stepu, tdivz16stepu, zi16stepu;

    r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));

    r_turb_sstep = 0;	// keep compiler happy
    r_turb_tstep = 0;	// ditto

    r_turb_pbase = cacheblock;

    sdivz16stepu = d_sdivzstepu * 16;
    tdivz16stepu = d_tdivzstepu * 16;
    zi16stepu = d_zistepu * 16;

    // translucent water - begin
	// we count on FP exceptions being turned off to avoid range problems
    izistep = (int)(d_zistepu * 0x8000 * 0x10000);
    izistep2 = izistep*2;
    // translucent water - end

    do {
        r_turb_pdest = d_viewbuffer + (screenwidth * pspan->v) + pspan->u;
        pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u; // translucent water

        count = pspan->count;

        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
        du = (float)pspan->u;
        dv = (float)pspan->v;

        sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
        tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
        zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
        z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
        // we count on FP exceptions being turned off to avoid range problems // translucent water
        izi = (int)(zi * 0x8000 * 0x10000); // translucent water

        r_turb_s = (int)(sdivz * z) + sadjust;
        if (r_turb_s > bbextents) {
            r_turb_s = bbextents;
        } else if (r_turb_s < 0) {
            r_turb_s = 0;
        }

        r_turb_t = (int)(tdivz * z) + tadjust;
        if (r_turb_t > bbextentt) {
            r_turb_t = bbextentt;
        } else if (r_turb_t < 0) {
            r_turb_t = 0;
        }

        do {
            // calculate s and t at the far end of the span
            if (count >= 16) {
                r_turb_spancount = 16;
            } else {
                r_turb_spancount = count;
            }

            count -= r_turb_spancount;

            if (count) {
                // calculate s/z, t/z, zi->fixed s and t at far end of span,
                // calculate s and t steps across span by shifting
                sdivz += sdivz16stepu;
                tdivz += tdivz16stepu;
                zi += zi16stepu;
                z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

                snext = (int)(sdivz * z) + sadjust;
                if (snext > bbextents) {
                    snext = bbextents;
                } else if (snext < 16) {
                    snext = 16;    // prevent round-off error on <0 steps from
                }
                //  from causing overstepping & running off the
                //  edge of the texture

                tnext = (int)(tdivz * z) + tadjust;
                if (tnext > bbextentt) {
                    tnext = bbextentt;
                } else if (tnext < 16) {
                    tnext = 16;    // guard against round-off error on <0 steps
                }

                r_turb_sstep = (snext - r_turb_s) >> 4;
                r_turb_tstep = (tnext - r_turb_t) >> 4;
            } else {
                // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
                // can't step off polygon), clamp, calculate s and t steps across
                // span by division, biasing steps low so we don't run off the
                // texture
                spancountminus1 = (float)(r_turb_spancount - 1);
                sdivz += d_sdivzstepu * spancountminus1;
                tdivz += d_tdivzstepu * spancountminus1;
                zi += d_zistepu * spancountminus1;
                z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
                snext = (int)(sdivz * z) + sadjust;
                if (snext > bbextents) {
                    snext = bbextents;
                } else if (snext < 16) {
                    snext = 16;    // prevent round-off error on <0 steps from
                }
                //  from causing overstepping & running off the
                //  edge of the texture

                tnext = (int)(tdivz * z) + tadjust;
                if (tnext > bbextentt) {
                    tnext = bbextentt;
                } else if (tnext < 16) {
                    tnext = 16;    // guard against round-off error on <0 steps
                }

                if (r_turb_spancount > 1) {
                    r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
                    r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
                }
            }

            r_turb_s = r_turb_s & ((CYCLE<<16)-1);
            r_turb_t = r_turb_t & ((CYCLE<<16)-1);

            // translucent water - begin
            if (r_drawwater) {
                // r_wateralpha*10: 1&2=25% 3&4=33% 5&6=50% 7&8&9=66%
                if (r_wateralpha.value <= .25) { // 25%
                    teste = ((((int)r_turb_pdest-(int)d_viewbuffer) / screenwidth)+1) & 4;
                    if (teste) { // 25% transparency
stipple:
                        if (!(((int)r_turb_pdest + teste) & 4)) { // if we are in the wrong pixel,
                            if (r_turb_spancount == 1) {
                                goto end_of_loop;
                            }
                            teste=0;
                            do {
                                if (teste)
                                    if (*pz <= (izi >> 16)) {
#ifdef BILINEAR
#define sturb2 (r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])
#define tturb2 (r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])
                                        int x,y,x1,y1,c00,c01,c10,c11,xm,ym,xmi,ymi;
                                        x	=	sturb2;
                                        y	=	tturb2;
                                        xm	=	x&0xffff;
                                        ym	=	y&0xffff;
                                        xmi	=	0xffff-xm;
                                        ymi	=	0xffff-ym;
                                        x	=	(x>>16)&63;
                                        y	=	(y>>16)&63;
                                        x1	=	(x+1)&63;
                                        y1	=	(y+1)&63;

                                        c00	=	*(r_turb_pbase+x +(y<<6));
                                        c01	=	*(r_turb_pbase+x1+(y<<6));
                                        c10	=	*(r_turb_pbase+x +(y1<<6));
                                        c11	=	*(r_turb_pbase+x1+(y1<<6));

                                        c00	=	LRP(c00,c01,xm,xmi,0)|LRP(c00,c01,xm,xmi,8)|LRP(c00,c01,xm,xmi,16);
                                        c10	=	LRP(c10,c11,xm,xmi,0)|LRP(c10,c11,xm,xmi,8)|LRP(c10,c11,xm,xmi,16);

                                        *r_turb_pdest = LRP(c00,c10,ym,ymi,0)|LRP(c00,c10,ym,ymi,8)|LRP(c00,c10,ym,ymi,16);
#else
#define sturb2 (((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63)
#define tturb2 (((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63)
                                        *r_turb_pdest = *(r_turb_pbase + (tturb2<<6) + sturb2);
#endif
                                    }
                                *r_turb_pdest++;
                                izi += izistep;
                                pz++;
                                r_turb_s += r_turb_sstep;
                                r_turb_t += r_turb_tstep;
                                teste = !teste;
                            } while (--r_turb_spancount > 0);
                            goto end_of_loop;
                        } else {
                            r_turb_spancount = (r_turb_spancount+1)/2;
                        }
                        // multiply steps by 2
                        r_turb_sstep*=2;
                        r_turb_tstep*=2;
                        do {
                            if (*pz <= (izi >> 16)) {
#ifdef BILINEAR
                                int x,y,x1,y1,c00,c01,c10,c11,xm,ym,xmi,ymi;
                                x	=	sturb2;
                                y	=	tturb2;
                                xm	=	x&0xffff;
                                ym	=	y&0xffff;
                                xmi	=	0xffff-xm;
                                ymi	=	0xffff-ym;
                                x	=	(x>>16)&63;
                                y	=	(y>>16)&63;
                                x1	=	(x+1)&63;
                                y1	=	(y+1)&63;

                                c00	=	*(r_turb_pbase+x +(y<<6));
                                c01	=	*(r_turb_pbase+x1+(y<<6));
                                c10	=	*(r_turb_pbase+x +(y1<<6));
                                c11	=	*(r_turb_pbase+x1+(y1<<6));

                                c00	=	LRP(c00,c01,xm,xmi,0)|LRP(c00,c01,xm,xmi,8)|LRP(c00,c01,xm,xmi,16);
                                c10	=	LRP(c10,c11,xm,xmi,0)|LRP(c10,c11,xm,xmi,8)|LRP(c10,c11,xm,xmi,16);

                                *r_turb_pdest = LRP(c00,c10,ym,ymi,0)|LRP(c00,c10,ym,ymi,8)|LRP(c00,c10,ym,ymi,16);
#else
                                *r_turb_pdest = *(r_turb_pbase + (tturb2<<6) + sturb2);
#endif
                            }
                            // advance two pixels
                            r_turb_pdest += 2;
                            pz += 2;
                            izi += izistep2;
                            r_turb_s += r_turb_sstep;
                            r_turb_t += r_turb_tstep;
                        } while (--r_turb_spancount > 0);
                    }
                } else if (r_wateralpha.value <= 0.41 && alphamap) { // 33%
                    do {
                        if (*pz <= (izi >> 16)) {
                            sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
                            tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
#define temp *(r_turb_pbase + (tturb<<6) + sturb)
                            *r_turb_pdest = alphamap[temp + *r_turb_pdest*256];
                        }
                        *r_turb_pdest++;
                        izi += izistep;
                        pz++;
                        r_turb_s += r_turb_sstep;
                        r_turb_t += r_turb_tstep;
                    } while (--r_turb_spancount > 0);
                } else if (r_wateralpha.value < 0.66 || !alphamap) { // 50%
                    teste = ((((int)r_turb_pdest-(int)d_viewbuffer) / screenwidth)+1) & 4;
                    goto stipple;
                } else { //if (r_wateralpha.value >= 0.66 && alphamap) // 66%
                    do {
                        if (*pz <= (izi >> 16)) {
                            sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
                            tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
#define temp *(r_turb_pbase + (tturb<<6) + sturb)
                            *r_turb_pdest = alphamap[*r_turb_pdest + temp*256];
                        }
                        *r_turb_pdest++;
                        izi += izistep;
                        pz++;
                        r_turb_s += r_turb_sstep;
                        r_turb_t += r_turb_tstep;
                    } while (--r_turb_spancount > 0);
                }
            } else
                // translucent water - end
            {
                D_DrawTurbulent8Span();
            }

end_of_loop: // translucent water
            r_turb_s = snext;
            r_turb_t = tnext;

        } while (count > 0);

    } while ((pspan = pspan->pnext) != NULL);
}


#if	!id386

/*
=============
D_DrawSpans8
=============
*/
void D_DrawSpans8(espan_t *pspan)
{
    int				count, spancount;
    pixel_t			*pdest,*pbase;
    fixed16_t		s, t, snext, tnext, sstep, tstep;
    float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
    float			sdivz8stepu, tdivz8stepu, zi8stepu;

    sstep = 0;	// keep compiler happy
    tstep = 0;	// ditto

    pbase = cacheblock;

    sdivz8stepu = d_sdivzstepu * 8;
    tdivz8stepu = d_tdivzstepu * 8;
    zi8stepu = d_zistepu * 8;

    do {
        pdest = d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

        count = pspan->count;

        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
        du = (float)pspan->u;
        dv = (float)pspan->v;

        sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
        tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
        zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
        z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

        s = (int)(sdivz * z) + sadjust;
        if (s > bbextents) {
            s = bbextents;
        } else if (s < 0) {
            s = 0;
        }

        t = (int)(tdivz * z) + tadjust;
        if (t > bbextentt) {
            t = bbextentt;
        } else if (t < 0) {
            t = 0;
        }

        do {
            // calculate s and t at the far end of the span
            if (count >= 8) {
                spancount = 8;
            } else {
                spancount = count;
            }

            count -= spancount;

            if (count) {
                // calculate s/z, t/z, zi->fixed s and t at far end of span,
                // calculate s and t steps across span by shifting
                sdivz += sdivz8stepu;
                tdivz += tdivz8stepu;
                zi += zi8stepu;
                z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

                snext = (int)(sdivz * z) + sadjust;
                if (snext > bbextents) {
                    snext = bbextents;
                } else if (snext < 8) {
                    snext = 8;    // prevent round-off error on <0 steps from
                }
                //  from causing overstepping & running off the
                //  edge of the texture

                tnext = (int)(tdivz * z) + tadjust;
                if (tnext > bbextentt) {
                    tnext = bbextentt;
                } else if (tnext < 8) {
                    tnext = 8;    // guard against round-off error on <0 steps
                }

                sstep = (snext - s) >> 3;
                tstep = (tnext - t) >> 3;
            } else {
                // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
                // can't step off polygon), clamp, calculate s and t steps across
                // span by division, biasing steps low so we don't run off the
                // texture
                spancountminus1 = (float)(spancount - 1);
                sdivz += d_sdivzstepu * spancountminus1;
                tdivz += d_tdivzstepu * spancountminus1;
                zi += d_zistepu * spancountminus1;
                z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
                snext = (int)(sdivz * z) + sadjust;
                if (snext > bbextents) {
                    snext = bbextents;
                } else if (snext < 8) {
                    snext = 8;    // prevent round-off error on <0 steps from
                }
                //  from causing overstepping & running off the
                //  edge of the texture

                tnext = (int)(tdivz * z) + tadjust;
                if (tnext > bbextentt) {
                    tnext = bbextentt;
                } else if (tnext < 8) {
                    tnext = 8;    // guard against round-off error on <0 steps
                }

                if (spancount > 1) {
                    sstep = (snext - s) / (spancount - 1);
                    tstep = (tnext - t) / (spancount - 1);
                }
            }

            s	=	s-(1<<15);
            t	=	t-(1<<15);

            do {

#ifdef BILINEAR
                int x,y,x1,y1,c00,c01,c10,c11,xm,ym,xmi,ymi;
                int r,g,b;
                xm	=	s&0xffff;
                ym	=	t&0xffff;
                xmi	=	0xffff-xm;
                ymi	=	0xffff-ym;
                x	=	(s >> 16);
                y	=	(t >> 16);
                x1	=	x+1;
                y1	=	y+1;

                if (x<0) {
                    x=0;
                }
                if (y<0) {
                    y=0;
                }
                if (x1>=cachewidth) {
                    x1=0;
                }
                if (y1>=cacheheight) {
                    y1=0;
                }

                c00	=	*(pbase + x +  y *cachewidth);
                c01	=	*(pbase + x1+  y *cachewidth);
                c10	=	*(pbase + x +  y1*cachewidth);
                c11	=	*(pbase + x1+  y1*cachewidth);

                c00	=	LRPM(c00,c01,xm,xmi,0xff)|LRPM(c00,c01,xm,xmi,0xff00)|LRP(c00,c01,xm,xmi,16);
                c10	=	LRPM(c10,c11,xm,xmi,0xff)|LRPM(c10,c11,xm,xmi,0xff00)|LRP(c10,c11,xm,xmi,16);

                *pdest++ = LRPM(c00,c10,ym,ymi,0xff)|LRPM(c00,c10,ym,ymi,0xff00)|LRP(c00,c10,ym,ymi,16);
#else
                *pdest++ = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
#endif
                s += sstep;
                t += tstep;
            } while (--spancount > 0);

            s = snext;
            t = tnext;

        } while (count > 0);

    } while ((pspan = pspan->pnext) != NULL);
}

#endif


#if	!id386

/*
=============
D_DrawZSpans
=============
*/
void D_DrawZSpans(espan_t *pspan)
{
    int				count, doublecount, izistep;
    int				izi;
    short			*pdest;
    unsigned		ltemp;
    double			zi;
    float			du, dv;

	// FIXME: check for clamping/range problems
	// we count on FP exceptions being turned off to avoid range problems
    izistep = (int)(d_zistepu * 0x8000 * 0x10000);

    do {
        pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

        count = pspan->count;

        // calculate the initial 1/z
        du = (float)pspan->u;
        dv = (float)pspan->v;

        zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
        // we count on FP exceptions being turned off to avoid range problems
        izi = (int)(zi * 0x8000 * 0x10000);

        if ((long)pdest & 0x02) {
            *pdest++ = (short)(izi >> 16);
            izi += izistep;
            count--;
        }

        if ((doublecount = count >> 1) > 0) {
            do {
                ltemp = izi >> 16;
                izi += izistep;
                ltemp |= izi & 0xFFFF0000;
                izi += izistep;
                *(int *)pdest = ltemp;
                pdest += 2;
            } while (--doublecount > 0);
        }

        if (count & 1) {
            *pdest = (short)(izi >> 16);
        }

    } while ((pspan = pspan->pnext) != NULL);
}

#endif

