/* Pure data extern that passes "bang"-messages that 
 * come to left inlet if the creation argument is smaller 
 * than the right inlet.
 * 
 * This extern is used to close and open the control
 * signal of a single clapper when modeling chancing 
 * number of clappers.
 *
 * Copyright (C) Leevi Peltola, 07.12.2004
 * $Id: ppl.c,v 1.1 2006/03/16 23:04:43 cerkut Exp $
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
#include <string.h>
#include <stdio.h>

t_class *ppl_class;

typedef struct _ppl {
    t_object x_obj;
    float x_state;
    float x_no;
} t_ppl;

void *ppl_new(t_floatarg f) {
    t_ppl *x = (t_ppl *)pd_new(ppl_class);
    floatinlet_new(&x->x_obj, &x->x_state);
    outlet_new(&x->x_obj, 0);
    x->x_state = 0;
    x->x_no = f;
    return (x);
}

void ppl_bang(t_ppl *x) {
    if (x->x_state >= x->x_no) outlet_bang(x->x_obj.ob_outlet);
}

void ppl_float(t_ppl *x, t_float f) {
    if (x->x_state >= x->x_no) outlet_float(x->x_obj.ob_outlet, f);
}

void ppl_setup(void) {
    ppl_class = class_new(gensym("ppl"), (t_newmethod)ppl_new, 0,
    	sizeof(t_ppl), 0, A_DEFFLOAT, 0);
    class_addbang(ppl_class, ppl_bang);
    class_addfloat(ppl_class, ppl_float);

}

