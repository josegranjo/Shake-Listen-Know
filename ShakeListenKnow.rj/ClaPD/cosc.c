/* Pure data extern which models one clapper in an audience as
 * a coupled oscilator. First inlet turns oscilator on and off
 * with messages 1 and 0. Second inlet is 1 if the clapping is 
 * synchronized an 0 if not. Third inlet is a float that describes
 * how synchronized the audience is (1 = bad synchronization and 
 * 0 = good synchronization. Fourth inlet takes the time difference 
 * between the oscilator and the metro that gives the 
 * synchronization signal. The phase of the oscilator is changed
 * based on this phase difference
 *
 * Copyright (C) Leevi Peltola, 07.12.2004
 *
 * $Id: cosc.c,v 1.1 2006/03/16 23:04:43 cerkut Exp $
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
#include <stdio.h>
#include <stdlib.h>

t_class *cosc_class;

typedef struct _cosc {
  t_object x_obj;
  t_clock *x_clock;
  int x_hit;
  double x_deltime;		//holds the currend preferred clapping rate
  float x_delay;        //current delay until the next clap
  t_float x_pshift;		//phase difference with leader oscilator
  t_float x_ooi_sync;   //preferred clapping rate in synchronizated state
  t_float x_ooi_enth;   //preferred clapping rate in unsynchronizated state
  t_float x_sync;			//synchronization on/off
  t_float x_order;		//parameter that describes how syncronizated the audience is
} t_cosc;

void cosc_sync(t_cosc *x, t_floatarg sync);
void cosc_order(t_cosc *x, t_floatarg order);
void cosc_tick(t_cosc *x);

void *cosc_new(t_floatarg f) {
    t_cosc *x = (t_cosc *)pd_new(cosc_class);
    outlet_new(&x->x_obj, gensym("bang"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("sync"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pshift"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("order"));

		x->x_pshift = 0;;
    x->x_clock = clock_new(x, (t_method)cosc_tick);
		x->x_ooi_sync = 440;
		x->x_ooi_enth = 150 + 70*rand()/RAND_MAX + 70*rand()/RAND_MAX;   //triangular distribution with average of 220
		cosc_sync(x, 0);
		cosc_order(x, 0);
    x->x_hit = 0;
    return (x);
}

void cosc_tick(t_cosc *x) {
    x->x_hit = 0;
    outlet_bang(x->x_obj.ob_outlet);

		//in syncronized mode, speed up the clapping rate if the oscilator is ahead of the control oscilator 
		if (x->x_pshift < 220 && x->x_pshift > (20+70*x->x_order) && x->x_sync == 1){
			x->x_delay = .5*(x->x_delay*x->x_order 
								  + x->x_ooi_sync*(1-x->x_order) 
								  + x->x_ooi_sync) 
			  - x->x_pshift/(3+4*x->x_order);
		}
		//in syncronized mode, slow down the clapping rate if the oscilator is trailing behind the control oscilator 
		else if (x->x_pshift < (420-70*x->x_order) && x->x_pshift >= 220 && x->x_sync == 1){
			x->x_delay = .5*(x->x_delay*x->x_order 
								  + x->x_ooi_sync*(1-x->x_order) 
								  + x->x_ooi_sync) 
			  + (x->x_ooi_sync - x->x_pshift)/(3+4*x->x_order);
		}
		//when changing to the unsyncronized mode, speed up the clapping rate until the own clapping rate is reached 
		else if (x->x_sync == 0 && x->x_delay > x->x_deltime*(1.05+0.25*x->x_order)){
			x->x_delay = x->x_delay/(1.3-0.25*x->x_order);
		}
		//if everything is ok, just keep on clapping
		else {
			x->x_delay = x->x_deltime;
		}

		//waiting time before the next clap has 10% triangular distribution
		x->x_delay = 0.9*x->x_delay + 0.1*x->x_delay*rand()/RAND_MAX + 0.1*x->x_delay*rand()/RAND_MAX;
		if (!x->x_hit) clock_delay(x->x_clock, x->x_delay);
}

void cosc_float(t_cosc *x, t_float f) {
    if (f != 0) cosc_tick(x);
    else clock_unset(x->x_clock);
    x->x_hit = 1;
}

void cosc_bang(t_cosc *x) {
    cosc_float(x, 1);
}

void cosc_stop(t_cosc *x) {
    cosc_float(x, 0);
}

void cosc_sync(t_cosc *x, t_floatarg sync){
	x->x_sync=sync;
	if (x->x_sync == 1) x->x_deltime = x->x_ooi_sync;
	else x->x_deltime = x->x_ooi_enth;
}

void cosc_order(t_cosc *x, t_floatarg order){
	x->x_order=order;
}

void cosc_pshift(t_cosc *x, t_floatarg pshift){
	x->x_pshift=pshift;
}

void cosc_free(t_cosc *x) {
    clock_free(x->x_clock);
}

void cosc_setup(void) {
    cosc_class = class_new(gensym("cosc"), (t_newmethod)cosc_new,
    	(t_method)cosc_free, sizeof(t_cosc), 0, A_DEFFLOAT, 0);
    class_addbang(cosc_class, cosc_bang);
    class_addmethod(cosc_class, (t_method)cosc_stop, gensym("stop"), 0);
    class_addmethod(cosc_class, (t_method)cosc_sync, gensym("sync"),
    	A_FLOAT, 0);
    class_addmethod(cosc_class, (t_method)cosc_order, gensym("order"),
    	A_FLOAT, 0);
    class_addmethod(cosc_class, (t_method)cosc_pshift, gensym("pshift"),
    	A_FLOAT, 0);
    class_addfloat(cosc_class, (t_method)cosc_float);
}
