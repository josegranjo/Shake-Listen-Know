/* Pure data extern to implement an resonator that is used 
 * in the inverse filtering of the excitation noise of a hand
 * clapping synthesis
 *
 * The input signal is filtered with an resonator (2-pole 2-zero IIR-
 * filter. Arguments a1, a2, b0, b1, b2 are given when the object is 
 * created.
 *
 * Example: twopz~ 0.2 0.22 1 0 -1 creates a bandpass filter that 
 *                                 is used to filter the excitation 
 *                                 noise in clapping synthesis
 *
 * Copyright (C) Leevi Peltola, 07.12.2004
 * $Id: twopz~.c,v 1.1 2006/03/16 23:04:43 cerkut Exp $
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

typedef struct twopzctl {
    float c_x1;
    float c_x2;
    float c_y1;
    float c_y2;
    float c_b0;
    float c_b1;
    float c_b2;
    float c_a1;
    float c_a2;
} t_twopzctl;

typedef struct twopz {
    t_object x_obj;
    float x_f;
    t_twopzctl x_cspace;
    t_twopzctl *x_ctl;
} t_twopz;

t_class *twopz_class;

static void twopz_list(t_twopz *x, t_symbol *s, int argc, t_atom *argv);

static void *twopz_new(t_symbol *s, int argc, t_atom *argv) {
    t_twopz *x = (t_twopz *)pd_new(twopz_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x1 = x->x_cspace.c_x2 = 0;
    twopz_list(x, s, argc, argv);
    x->x_f = 0;
    return (x);
}

static t_int *twopz_perform(t_int *w) {
    float *in = (float *)(w[1]);
    float *out = (float *)(w[2]);
    t_twopzctl *c = (t_twopzctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    float x1 = c->c_x1;
    float x2 = c->c_x2;
    float yy1 = c->c_y1;
    float yy2 = c->c_y2;

    float a1 = c->c_a1;
    float a2 = c->c_a2;
    float b0 = c->c_b0;
    float b1 = c->c_b1;
    float b2 = c->c_b2;

    for (i = 0; i < n; i++) {
			float input = *in++;
    	float output =  input*b0 + x1*b1 + x2*b2 + yy1*a1 + yy2*a2;
    	*out++ = output;
			yy2 = yy1;
			yy1 = output;
			x2 = x1;
			x1 = input;
    }
    c->c_x1 = x1;
    c->c_x2 = x2;
    c->c_y1 = yy1;
    c->c_y2 = yy2;
    return (w+5);
}

static void twopz_list(t_twopz *x, t_symbol *s, int argc, t_atom *argv) {
    float a1 = atom_getfloatarg(0, argc, argv);
    float a2 = atom_getfloatarg(1, argc, argv);
    float b0 = atom_getfloatarg(2, argc, argv);
    float b1 = atom_getfloatarg(3, argc, argv);
    float b2 = atom_getfloatarg(4, argc, argv);
    t_twopzctl *c = x->x_ctl;    
	
    c->c_a1 = a1;
    c->c_a2 = a2;
    c->c_b0 = b0;
    c->c_b1 = b1;
    c->c_b2 = b2;
}

static void twopz_set(t_twopz *x, t_symbol *s, int argc, t_atom *argv) {
    t_twopzctl *c = x->x_ctl;
    c->c_x1 = atom_getfloatarg(0, argc, argv);
    c->c_x2 = atom_getfloatarg(1, argc, argv);
}

static void twopz_dsp(t_twopz *x, t_signal **sp) {
    dsp_add(twopz_perform, 4,
						sp[0]->s_vec, sp[1]->s_vec, 
						x->x_ctl, sp[0]->s_n);
}

void twopz_tilde_setup(void){
    twopz_class = class_new(gensym("twopz~"), (t_newmethod)twopz_new,
    	0, sizeof(t_twopz), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(twopz_class, t_twopz, x_f);
    class_addmethod(twopz_class, (t_method)twopz_dsp, gensym("dsp"), 0);
    class_addlist(twopz_class, twopz_list);
    class_addmethod(twopz_class, (t_method)twopz_set, gensym("set"),
    	A_GIMME, 0);
    class_addmethod(twopz_class, (t_method)twopz_set, gensym("clear"),
    	A_GIMME, 0);
}










