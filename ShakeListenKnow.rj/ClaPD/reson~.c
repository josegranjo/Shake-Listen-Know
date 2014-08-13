/* Pure data external to implement an resonator that is used in 
 * hand clapping synthesis.
 *
 * The input signal is filtered with an resonator (2-pole IIR-
 * filter. In addition to input signal, takes the center 
 * frequency (fc), -3dB bandwidth (b) and the gain factor (g)
 * as an input and implements the filter using these. The
 * filter coefficients are updated if the value of inputs chances.
 *
 * Copyright (C) Leevi Peltola, 07.12.2004
 * $Id: reson~.c,v 1.1 2006/03/16 23:04:43 cerkut Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "m_pd.h"
#include <math.h>

static t_class *reson_class;

typedef struct rctl {
  float c_y1;				// old outputs for filtering
  float c_y2;
  float c_a0;				// filter coefficients
  float c_a1;
  float c_a2;
} t_rctl;

typedef struct reson {
  t_object  x_obj;		
  float x_b;				// bandwidth
  float x_fc;				// center frequency	
  float x_g;				// gain
  float x_r;				// pole radius
  float x_theta;		// pole angle
  float x_fs;				// sampling rate
	t_rctl x_cspace;	// links to
	t_rctl *x_ctl;		// filter coefficients 
  t_float x_f;
} t_reson;

static void reson_docoef(t_reson *x, t_floatarg fs, t_floatarg b, t_floatarg g);

/* constructor */
void *reson_new(t_floatarg fc, t_floatarg b, t_floatarg g) {
  t_reson *x = (t_reson *)pd_new(reson_class);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft2"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft3"));
	outlet_new(&x->x_obj, gensym("signal"));
  x->x_fs = 44100;

	x->x_ctl = &x->x_cspace;
	x->x_cspace.c_y1 = 0;
	x->x_cspace.c_y2 = 0;
	reson_docoef(x,fc,b,g);
	x->x_f = 0;
  return (x);
}

/* update coefficients */
static void reson_docoef(t_reson *x, t_floatarg fc, t_floatarg b, t_floatarg g){
		x->x_fc=fc;	
		x->x_b=b;
		x->x_g=g;
		x->x_r = 1 - x->x_b/x->x_fs*(float)3.14259;
		x->x_theta = (float)acos(2*x->x_r/(1+pow(x->x_r,2))*(float)cos(2*(3.14259f)*x->x_fc/x->x_fs));
		x->x_ctl->c_a0 = (1 - (float)pow(x->x_r,2))*(float)sin(x->x_theta)*x->x_g/2;
		x->x_ctl->c_a1 = 2*x->x_r*(float)cos(x->x_theta);
	  x->x_ctl->c_a2 = -1*(float)pow(x->x_r,2);
}

static void reson_ft1(t_reson *x, t_floatarg fc){
    reson_docoef(x, fc, x->x_b, x->x_g);
}

static void reson_ft2(t_reson *x, t_floatarg b){
    reson_docoef(x, x->x_fc, b, x->x_g);
}

static void reson_ft3(t_reson *x, t_floatarg g){
    reson_docoef(x, x->x_fc, x->x_b, g);
}

t_int *reson_perform(t_int *w) {
  float *in1 = (float *)(w[1]);
  float *out = (float *)(w[2]);
  t_rctl *c = (t_rctl *)(w[3]);
  int n = (t_int)(w[4]);
	int i;
  float a0 = c->c_a0;
  float a1 = c->c_a1;
  float a2 = c->c_a2;
  float yy1 = c->c_y1;
  float yy2 = c->c_y2;

  for (i = 0; i < n; i++) {
    float yy0 = a0*(*in1++) + a1*yy1 + a2*yy2;
    *out++ = yy0;
    yy2=yy1;
    yy1=yy0;
  }

  c->c_y1=yy1;
  c->c_y2=yy2;
  return (w+5);
}


void reson_dsp(t_reson *x, t_signal **sp) {
	x->x_fs = sp[0]->s_sr;
	reson_docoef(x, x->x_fc, x->x_b, x->x_g);
  dsp_add(reson_perform, 4,
          sp[0]->s_vec, sp[1]->s_vec, 
					x->x_ctl, sp[0]->s_n);
}

void reson_tilde_setup(void) {
  reson_class = class_new(gensym("reson~"),
        (t_newmethod)reson_new,
        0, sizeof(t_reson),
        0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);	
  class_addmethod(reson_class,(t_method)reson_dsp, gensym("dsp"), 0);
  class_addmethod(reson_class, (t_method)reson_ft1,
    							gensym("ft1"), A_FLOAT, 0);
  class_addmethod(reson_class, (t_method)reson_ft2,
    							gensym("ft2"), A_FLOAT, 0);
  class_addmethod(reson_class, (t_method)reson_ft3,
    							gensym("ft3"), A_FLOAT, 0);
  CLASS_MAINSIGNALIN(reson_class, t_reson, x_f);
}













