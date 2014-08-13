/* Pure data extern to create noise envelopes that are
 * used as an excitation in hand clapping synthesis
 *
 * Takes (white noise) signal as an input and multiplies 
 * it with a x_namp coefficient. When bang message is received
 * the x_namp first increases exponentially during the first 
 * 140 samples and then goes to zero or starts decaying 
 * exponentially depending if the second input is 0 or 1.
 *
 * Copyright (C) Leevi Peltola, 07.12.2004
 * 
 * $Id: noiseq~.c,v 1.1 2006/03/16 23:04:43 cerkut Exp $
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

static t_class *noiseq_class;

typedef struct _noiseq {
  t_object  x_obj;
  t_int x_count;		//counts samples after a bang
  t_float x_donoff;	//switch on/off the exponentially decaying decay part of the excitation signal
	t_float x_env;		//show if there is already a clap inside when a new one comes. (used to decide if the reson~ coefficients can be changed)
	t_float x_namp;		//amplitude of the noise excitation
	t_float x_decay;	//constant for exponential decay/rise
	t_outlet *f_out;
	t_sample x_f;
} t_noiseq;

/* restart counter and increase noise amplitude */
void noiseq_bang(t_noiseq *x) {
	if (x->x_count > 140)
		x->x_namp = (0.02f);
	else{
		x->x_namp = (0.03f);
	}
  x->x_count=0;
	outlet_float(x->f_out, x->x_env);
}

void *noiseq_new(t_floatarg f) {
  t_noiseq *x = (t_noiseq *)pd_new(noiseq_class);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("onoff"));
  x->x_count=0;
  x->x_donoff=0;
	x->x_decay=(float)0.99;
	x->x_namp=0;
	x->x_env=1;
  outlet_new(&x->x_obj, gensym("signal"));
	x->f_out = outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

static void noiseq_onoff(t_noiseq *x, t_floatarg donoff){
	x->x_donoff=donoff;
}

t_int *noiseq_perform(t_int *w) {
	t_float *in = (t_float *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	t_noiseq *x = (t_noiseq *)(w[3]);
	int n = (int)(w[4]);
	int i;	
	/* increase or decrease noise amplitude namp and multiply input with it */
	while(n--) {
		float f = *(in++);
		f=f*x->x_namp;
		*out++ = f;	
	
		if (x->x_count < 140) {
			x->x_count++;
			x->x_env = 0;
			x->x_namp =	x->x_namp / x->x_decay;
		}
		else if (x->x_count < 600) {
			x->x_count++;
			x->x_env = 0;
			x->x_namp =	x->x_namp * x->x_decay *x->x_donoff;
		}
		else{
			x->x_env = 1;
			x->x_namp =	x->x_namp * x->x_decay *x->x_donoff;
		}
	}
	return (w+5);
}

void noiseq_dsp(t_noiseq *x, t_signal **sp) {
	dsp_add(noiseq_perform, 4, 
					sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);	
}

void noiseq_tilde_setup(void) {
  noiseq_class = class_new(gensym("noiseq~"),
									(t_newmethod)noiseq_new, 
									0, sizeof(t_noiseq),
									CLASS_DEFAULT, 0);
  class_addmethod(noiseq_class,(t_method)noiseq_dsp, gensym("dsp"), 0);
  class_addmethod(noiseq_class,
						(t_method)noiseq_onoff,gensym("onoff"), A_FLOAT, 0);
  CLASS_MAINSIGNALIN(noiseq_class, t_noiseq, x_f);
  class_addbang(noiseq_class, noiseq_bang);
}
