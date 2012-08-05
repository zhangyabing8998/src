/* Slope-based velocity-independent data attributes. */
/*
  Copyright (C) 2004 University of Texas at Austin
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>
#include <rsf.h>

int main (int argc, char* argv[])
{
    bool half;
    char *what;
    int it,ix,ih, nt,nx, nh, CDPtype;
    float dt, t0, h, h0, t, tm, tp, tx, tq, dh, dx, x0, x;
    float *px, *ph, *at;
    sf_file xdip, hdip, out;

    sf_init (argc,argv);
    xdip = sf_input("in");
    hdip = sf_input("hdip");
    out = sf_output("out");

    if (SF_FLOAT != sf_gettype(xdip)) sf_error("Need float input");
    if (!sf_histint(xdip,"n1",&nt)) sf_error("No n1= in input");
    if (!sf_histfloat(xdip,"d1",&dt)) sf_error("No d1= in input");
    if (!sf_histfloat(xdip,"o1",&t0)) sf_error("No o1= in input");

    if (!sf_histint(xdip,"n3",&nh)) sf_error("No n3= in input");
    if (!sf_histfloat(xdip,"d3",&dh)) sf_error("No d3= in input");
    if (!sf_histfloat(xdip,"o3",&h0)) sf_error("No o3= in input");

    if (!sf_histint(xdip,"n2",&nx)) sf_error("No n2= in input");
    if (!sf_histfloat(xdip,"d2",&dx)) sf_error("No d2= in input");
    if (!sf_histfloat(xdip,"o2",&x0)) sf_error("No o2= in input");

    if (!sf_getbool("half",&half)) half=true;
    /* if y, the second axis is half-offset instead of full offset */

    if (NULL==(what=sf_getstring("what"))) what="time";
    /* what attribute to compute */
	
    if (!half) {
	dh *= 0.5;
	h0 *= 0.5;
    }

    CDPtype=0.5+dh/dx;
    if (1 != CDPtype) sf_histint(xdip,"CDPtype",&CDPtype);
    sf_warning("CDPtype=%d",CDPtype);

    px = sf_floatalloc(nt);
    ph = sf_floatalloc(nt);
    at = sf_floatalloc(nt);  

    for (ih = 0; ih < nh; ih++) {
	for (ix = 0; ix < nx; ix++) {
	    x = x0 + ix*dx;
	    h = h0 + (ih+0.5)*dh + (dh/CDPtype)*(ix%CDPtype); 

	    sf_floatread (px, nt, xdip);
	    sf_floatread (ph, nt, hdip);
	    
	    for (it=0; it < nt; it++) {
		t = t0 + it*dt;
		tm = t - ph[it]*h*dt/dh;
		tx = h*px[it]*dt/dh;
		tp = tm*ph[it] + px[it]*tx;
		tq = tm*tm-tx*tx;

		switch(what[0]) {
		    case 't':
			at[it] = 
			    sqrtf(fabsf(t*ph[it]*tq*tq)/
				  (fabsf(tm*tm*tp)+SF_EPS));
			break;
		    case 'x':
			at[it] = x - t*h*px[it]/(tp+SF_EPS);
			break;
		    default:
			sf_error("Unknow attribute \"%s\"",what);
			break;
		}
	    }

	    sf_floatwrite (at,nt,out);
	}
    }

    exit (0);
}

