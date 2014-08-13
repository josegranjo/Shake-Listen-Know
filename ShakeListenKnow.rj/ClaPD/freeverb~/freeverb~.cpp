/* -------------------------- freeverb~ --------------------------------------- */
/*                                                                              */
/* Tilde object that implements the Schroeder/Moorer reverb model.              */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>.                               */
/* Get source at http://www.akustische-kunst.org/                               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/* Also compiles for Max/MSP.                                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#ifdef NT
#pragma warning( disable : 4091 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef PD
#include "m_pd.h"
#else	// Max/MSP
#include "ext.h"
#include "z_dsp.h"
#define t_floatarg double
#endif

#include <math.h>
#include <string.h>

#define LOGTEN 2.302585092994

#define	numcombs		8
#define	numallpasses	4
#define	muted			0
#define	fixedgain		0.015
#define scalewet		3.0
#define scaledry		2.0
#define scaledamp		0.4
#define scaleroom		0.28
#define offsetroom		0.7
#define initialroom		0.5
#define initialdamp		0.5
#define initialwet		1.0/scalewet
#define initialdry		0.0
#define initialwidth	1.0
#define initialmode		0
#define initialbypass   0
#define freezemode		0.5
#define	stereospread	23

/* these values assume 44.1KHz sample rate
   they will probably be OK for 48KHz sample rate
   but would need scaling for 96KHz (or other) sample rates.
   the values were obtained by listening tests.                */
static const int combtuningL[numcombs]
                     = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
static const int combtuningR[numcombs]
                     = { 1116+stereospread, 1188+stereospread, 1277+stereospread, 1356+stereospread,
					     1422+stereospread, 1491+stereospread, 1557+stereospread, 1617+stereospread };

static const int allpasstuningL[numallpasses]
                     = { 556, 441, 341, 225 };
static const int allpasstuningR[numallpasses]
                     = { 556+stereospread, 441+stereospread, 341+stereospread, 225+stereospread };

static char *version = "freeverb~ V1.1";

#ifdef PD
static t_class *freeverb_class;

typedef struct _freeverb
{
    t_object x_obj;
#else	// Max/MSP
void *freeverb_class;

typedef struct _freeverb
{
	t_pxobject x_obj;
#endif
		/* freeverb stuff */
	t_float	x_gain;
	t_float	x_roomsize,x_roomsize1;
	t_float	x_damp,x_damp1;
	t_float	x_wet,x_wet1,x_wet2;
	t_float	x_dry;
	t_float	x_width;
	t_float	x_mode;
	t_float x_bypass;
	int     x_skip;

	t_float	x_allpassfeedback;			/* feedback of allpass filters */
	t_float	x_combfeedback;				/* feedback of comb filters */
	t_float x_combdamp1;
	t_float x_combdamp2;
	t_float x_filterstoreL[numcombs];	/* stores last sample value */
	t_float x_filterstoreR[numcombs];

		/* buffers for the combs */
	t_float	*x_bufcombL[numcombs];
	t_float	*x_bufcombR[numcombs];
	int x_combidxL[numcombs];
	int x_combidxR[numcombs];

		/* buffers for the allpasses */
	t_float	*x_bufallpassL[numallpasses];
	t_float	*x_bufallpassR[numallpasses];
	int x_allpassidxL[numallpasses];
	int x_allpassidxR[numallpasses];

		/* we'll make local copies adjusted to fit our sample rate */
	int x_combtuningL[numcombs];
	int x_combtuningR[numcombs];

	int x_allpasstuningL[numallpasses];
	int x_allpasstuningR[numallpasses];

#ifdef PD
	t_float x_float;
#endif
} t_freeverb;

#ifdef PD
#define undenormalise(sample) if(((*(unsigned int*)&sample)&0x7f800000)==0) sample=0.0
#else
#define undenormalise(sample);
#endif

	/* we need prototypes for Mac for everything */
static void comb_setdamp(t_freeverb *x, t_floatarg val);
static void comb_setfeedback(t_freeverb *x, t_floatarg val);
static inline t_float comb_processL(t_freeverb *x, int filteridx, t_float input);
static inline t_float comb_processR(t_freeverb *x, int filteridx, t_float input);
static void allpass_setfeedback(t_freeverb *x, t_floatarg val);
static inline t_float allpass_processL(t_freeverb *x, int filteridx, t_float input);
static inline t_float allpass_processR(t_freeverb *x, int filteridx, t_float input);
t_int *freeverb_perform(t_int *w);
static void freeverb_dsp(t_freeverb *x, t_signal **sp);
static void freeverb_update(t_freeverb *x);
static void freeverb_setroomsize(t_freeverb *x, t_floatarg value);
static float freeverb_getroomsize(t_freeverb *x);
static void freeverb_setdamp(t_freeverb *x, t_floatarg value);
static float freeverb_getdamp(t_freeverb *x);
static void freeverb_setwet(t_freeverb *x, t_floatarg value);
static float freeverb_getwet(t_freeverb *x);
static void freeverb_setdry(t_freeverb *x, t_floatarg value);
static float freeverb_getdry(t_freeverb *x);
static void freeverb_setwidth(t_freeverb *x, t_floatarg value);
static float freeverb_getwidth(t_freeverb *x);
static void freeverb_setmode(t_freeverb *x, t_floatarg value);
static float freeverb_getmode(t_freeverb *x);
static void freeverb_setbypass(t_freeverb *x, t_floatarg value);
static void freeverb_mute(t_freeverb *x);
static float freeverb_getdb(float f);
static void freeverb_print(t_freeverb *x);
#ifndef PD
void freeverb_assist(t_freeverb *x, void *b, long m, long a, char *s);
#endif
static void freeverb_free(t_freeverb *x);
static void *freeverb_new(t_floatarg val);

/* -------------------- comb filter stuff ----------------------- */
static void comb_setdamp(t_freeverb *x, t_floatarg val) 
{
	x->x_combdamp1 = val; 
	x->x_combdamp2 = 1-val;
}

static void comb_setfeedback(t_freeverb *x, t_floatarg val) 
{
	x->x_combfeedback = val;
}

// Big to inline - but crucial for speed
static inline t_float comb_processL(t_freeverb *x, int filteridx, t_float input)
{
	t_float output;
	int bufidx = x->x_combidxL[filteridx];

	output = x->x_bufcombL[filteridx][bufidx];
	undenormalise(output);

	x->x_filterstoreL[filteridx] = (output*x->x_combdamp2) + (x->x_filterstoreL[filteridx]*x->x_combdamp1);
	undenormalise(x->x_filterstoreL[filteridx]);

	x->x_bufcombL[filteridx][bufidx] = input + (x->x_filterstoreL[filteridx]*x->x_combfeedback);

	if(++x->x_combidxL[filteridx] >= x->x_combtuningL[filteridx]) x->x_combidxL[filteridx] = 0;

	return output;
}

static inline t_float comb_processR(t_freeverb *x, int filteridx, t_float input)
{
	t_float output;
	int bufidx = x->x_combidxR[filteridx];

	output = x->x_bufcombR[filteridx][bufidx];
	undenormalise(output);

	x->x_filterstoreR[filteridx] = (output*x->x_combdamp2) + (x->x_filterstoreR[filteridx]*x->x_combdamp1);
	undenormalise(x->x_filterstoreR[filteridx]);

	x->x_bufcombR[filteridx][bufidx] = input + (x->x_filterstoreR[filteridx]*x->x_combfeedback);

	if(++x->x_combidxR[filteridx] >= x->x_combtuningR[filteridx]) x->x_combidxR[filteridx] = 0;

	return output;
}

/* -------------------- allpass filter stuff ----------------------- */
static void allpass_setfeedback(t_freeverb *x, t_floatarg val) 
{
	x->x_allpassfeedback = val;
}

// Big to inline - but crucial for speed
static inline t_float allpass_processL(t_freeverb *x, int filteridx, t_float input)
{
	t_float output;
	t_float bufout;
	int bufidx = x->x_allpassidxL[filteridx];
	
	bufout = (t_float)x->x_bufallpassL[filteridx][bufidx];
	undenormalise(bufout);
	
	output = -input + bufout;
	x->x_bufallpassL[filteridx][bufidx] = input + (bufout*x->x_allpassfeedback);

	if(++x->x_allpassidxL[filteridx] >= x->x_allpasstuningL[filteridx])
		x->x_allpassidxL[filteridx] = 0;

	return output;
}

static inline t_float allpass_processR(t_freeverb *x, int filteridx, t_float input)
{
	t_float output;
	t_float bufout;
	int bufidx = x->x_allpassidxR[filteridx];
	
	bufout = (t_float)x->x_bufallpassR[filteridx][bufidx];
	undenormalise(bufout);
	
	output = -input + bufout;
	x->x_bufallpassR[filteridx][bufidx] = input + (bufout*x->x_allpassfeedback);

	if(++x->x_allpassidxR[filteridx] >= x->x_allpasstuningR[filteridx])
		x->x_allpassidxR[filteridx] = 0;

	return output;
}

/* -------------------- general DSP stuff ----------------------- */
t_int *freeverb_perform(t_int *w)
{
	// assign from parameters
    t_freeverb *x = (t_freeverb *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[5]);
    int n = (int)(w[6]);
	int i;
	t_float outL, outR, inL, inR, input;
    
#ifndef PD
    if (x->x_obj.z_disabled)
    	goto out;    	 
#endif

	if(x->x_bypass)
	{
		// Bypass, so just copy input to output
		while(n--)
		{
			inL = *in1++;	// We have to copy first before we can write to output
			inR = *in2++;	// since this might be at the same memory position
			*out1++ = inL;
			*out2++ = inR;
		}
	}
	else
	{
    	// DSP loop
		while(n--)
		{
			outL = outR = 0.;
			inL = *in1++;
			inR = *in2++;
			input = (inL + inR) * x->x_gain;

			// Accumulate comb filters in parallel
			for(i=0; i < numcombs; i++)
			{
				outL += comb_processL(x, i, input);
				outR += comb_processR(x, i, input);
			}

			// Feed through allpasses in series
			for(i=0; i < numallpasses; i++)
			{
				outL = allpass_processL(x, i, outL);
				outR = allpass_processR(x, i, outR);
			}

			// Calculate output REPLACING anything already there
			*out1++ = outL*x->x_wet1 + outR*x->x_wet2 + inL*x->x_dry;
			*out2++ = outR*x->x_wet1 + outL*x->x_wet2 + inR*x->x_dry;
		}
	}
#ifndef PD
out:
#endif
	return(w + 7);
}

static void freeverb_dsp(t_freeverb *x, t_signal **sp)
{
    dsp_add(freeverb_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec, 
		sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

// ----------- general parameter & calculation stuff -----------

	// recalculate internal values after parameter change
static void freeverb_update(t_freeverb *x)
{

	int i;

	x->x_wet1 = x->x_wet*(x->x_width/2 + 0.5);
	x->x_wet2 = x->x_wet*((1-x->x_width)/2);

	if (x->x_mode >= freezemode)
	{
		x->x_roomsize1 = 1.;
		x->x_damp1 = 0.;
		x->x_gain = muted;
	}
	else
	{
		x->x_roomsize1 = x->x_roomsize;
		x->x_damp1 = x->x_damp;
		x->x_gain = fixedgain;
	}

	comb_setfeedback(x, x->x_roomsize1);
	comb_setdamp(x, x->x_damp1);
}

	// the following functions set / get the parameters
static void freeverb_setroomsize(t_freeverb *x, t_floatarg value)
{
	x->x_roomsize = (value*scaleroom) + offsetroom;
	freeverb_update(x);
}

static float freeverb_getroomsize(t_freeverb *x)
{
	return (x->x_roomsize-offsetroom)/scaleroom;
}

static void freeverb_setdamp(t_freeverb *x, t_floatarg value)
{
	x->x_damp = value*scaledamp;
	freeverb_update(x);
}

static float freeverb_getdamp(t_freeverb *x)
{
	return x->x_damp/scaledamp;
}

static void freeverb_setwet(t_freeverb *x, t_floatarg value)
{
	x->x_wet = value*scalewet;
	freeverb_update(x);
}

static float freeverb_getwet(t_freeverb *x)
{
	return (x->x_wet/scalewet);
}

static void freeverb_setdry(t_freeverb *x, t_floatarg value)
{
	x->x_dry = value*scaledry;
}

static float freeverb_getdry(t_freeverb *x)
{
	return (x->x_dry/scaledry);
}

static void freeverb_setwidth(t_freeverb *x, t_floatarg value)
{
	x->x_width = value;
	freeverb_update(x);
}

static float freeverb_getwidth(t_freeverb *x)
{
	return x->x_width;
}

static void freeverb_setmode(t_freeverb *x, t_floatarg value)
{
	x->x_mode = value;
	freeverb_update(x);
}

static float freeverb_getmode(t_freeverb *x)
{
	if (x->x_mode >= freezemode)
		return 1;
	else
		return 0;
}

static void freeverb_setbypass(t_freeverb *x, t_floatarg value)
{
	x->x_bypass = value;
	if(x->x_bypass)freeverb_mute(x);
}

	// fill delay lines with silence
static void freeverb_mute(t_freeverb *x)
{
	int i;

	if (freeverb_getmode(x) >= freezemode)
		return;

	for (i=0;i<numcombs;i++)
	{
		memset(x->x_bufcombL[i], 0x0, x->x_combtuningL[i]*sizeof(t_float));
		memset(x->x_bufcombR[i], 0x0, x->x_combtuningR[i]*sizeof(t_float));
	}
	for (i=0;i<numallpasses;i++)
	{
		memset(x->x_bufallpassL[i], 0x0, x->x_allpasstuningL[i]*sizeof(t_float));
		memset(x->x_bufallpassR[i], 0x0, x->x_allpasstuningR[i]*sizeof(t_float));
	}
}

	// convert gain factor into dB
static float freeverb_getdb(float f)
{
    if (f <= 0)	// equation does not work for 0...
	{
		return (-96);	// ...so we output max. damping
	}
    else
    {
    	float val = (20./LOGTEN * log(f));
    	return (val);
    }
}

static void freeverb_print(t_freeverb *x)
{
	post("freeverb~:");
	if(x->x_bypass) {
		post("  bypass: on");
	} else post("  bypass: off");
	if(!freeverb_getmode(x)) {
		post("  mode: normal");
	} else post("  mode: freeze");
	post("  roomsize: %g", freeverb_getroomsize(x)*scaleroom+offsetroom);
	post("  damping: %g %%", freeverb_getdamp(x)*100);
	post("  width: %g %%", x->x_width * 100);
	post("  wet level: %g dB", freeverb_getdb(freeverb_getwet(x)*scalewet));
	post("  dry level: %g dB", freeverb_getdb(freeverb_getdry(x)*scaledry));
}

	// clean up
static void freeverb_free(t_freeverb *x)    
{
	int i;
	// free memory used by delay lines
	for(i = 0; i < numcombs; i++)
	{
		freebytes(x->x_bufcombL[i], x->x_combtuningL[i]*sizeof(t_float));
		freebytes(x->x_bufcombR[i], x->x_combtuningR[i]*sizeof(t_float));
	}

	for(i = 0; i < numallpasses; i++)
	{
		freebytes(x->x_bufallpassL[i], x->x_allpasstuningL[i]*sizeof(t_float));
		freebytes(x->x_bufallpassR[i], x->x_allpasstuningR[i]*sizeof(t_float));
	}
#ifndef PD
	dsp_free((t_pxobject *)x);		// Free the object
#endif
}

static void *freeverb_new(t_floatarg f)
{
	int i;
	int sr = (int)sys_getsr();

#ifdef PD
    t_freeverb *x = (t_freeverb *)pd_new(freeverb_class);

	// add additional signal inlets and signal outlets
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
#else	// Max/MSP
    t_freeverb *x = (t_freeverb *)newobject(freeverb_class);
    
    // zero out the struct, to be careful
    if(x)
    {
    	for(i = sizeof(t_pxobject); i < sizeof(t_freeverb); i++)
    		((char*)x)[i] = 0;
    }
    
    dsp_setup((t_pxobject *)x,2);	// two signal inlets
    
    // two signal outlets
    outlet_new((t_object *)x, "signal");
    outlet_new((t_object *)x, "signal");
#endif	
	// recalculate the reverb parameters in case we don't run at 44.1kHz
	for(i = 0; i < numcombs; i++)
	{
		x->x_combtuningL[i] = (int)(combtuningL[i] * sr / 44100);
		x->x_combtuningR[i] = (int)(combtuningR[i] * sr / 44100);
	}
	for(i = 0; i < numallpasses; i++)
	{
		x->x_allpasstuningL[i] = (int)(allpasstuningL[i] * sr / 44100);
		x->x_allpasstuningR[i] = (int)(allpasstuningL[i] * sr / 44100);
	}

	// get memory for delay lines
	for(i = 0; i < numcombs; i++)
	{
		x->x_bufcombL[i] = (t_float*) getbytes(x->x_combtuningL[i]*sizeof(t_float));
		x->x_bufcombR[i] = (t_float*) getbytes(x->x_combtuningR[i]*sizeof(t_float));
		x->x_combidxL[i] = 0;
		x->x_combidxR[i] = 0;
	}
	for(i = 0; i < numallpasses; i++)
	{
		x->x_bufallpassL[i] = (t_float*) getbytes(x->x_allpasstuningL[i]*sizeof(t_float));
		x->x_bufallpassR[i] = (t_float*) getbytes(x->x_allpasstuningR[i]*sizeof(t_float));
		x->x_allpassidxL[i] = 0;
		x->x_allpassidxR[i] = 0;
	}

	// set default values
	x->x_allpassfeedback = 0.5;
	x->x_skip = 1;	// we use every sample
	freeverb_setwet(x, initialwet);
	freeverb_setroomsize(x, initialroom);
	freeverb_setdry(x, initialdry);
	freeverb_setdamp(x, initialdamp);
	freeverb_setwidth(x, initialwidth);
	freeverb_setmode(x, initialmode);
	freeverb_setbypass(x, initialbypass);

	// buffers will be full of rubbish - so we MUST mute them
	freeverb_mute(x);

    return (x);
}

#ifdef PD
extern "C" void freeverb_tilde_setup(void)
{
    freeverb_class = class_new(gensym("freeverb~"), (t_newmethod)freeverb_new, (t_method)freeverb_free,
    	sizeof(t_freeverb), 0, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(freeverb_class, t_freeverb, x_float);
    class_addmethod(freeverb_class, (t_method)freeverb_dsp, gensym("dsp"), A_NULL);
    class_addmethod(freeverb_class, (t_method)freeverb_setroomsize, gensym("roomsize"), A_FLOAT, A_NULL);
    class_addmethod(freeverb_class, (t_method)freeverb_setdamp, gensym("damping"), A_FLOAT, A_NULL);
    class_addmethod(freeverb_class, (t_method)freeverb_setwidth, gensym("width"), A_FLOAT, A_NULL);
	class_addmethod(freeverb_class, (t_method)freeverb_setwet, gensym("wet"), A_FLOAT, A_NULL);
	class_addmethod(freeverb_class, (t_method)freeverb_setdry, gensym("dry"), A_FLOAT, A_NULL);
	class_addmethod(freeverb_class, (t_method)freeverb_setmode, gensym("freeze"), A_FLOAT, A_NULL);
	class_addmethod(freeverb_class, (t_method)freeverb_setbypass, gensym("bypass"), A_FLOAT, A_NULL);
	class_addmethod(freeverb_class, (t_method)freeverb_mute, gensym("clear"), A_NULL);
    class_addmethod(freeverb_class, (t_method)freeverb_print, gensym("print"), A_NULL);
    class_sethelpsymbol(freeverb_class, gensym("help-freeverb~.pd"));
	post(version);
}

#else
// ----------- Max/MSP -----------
void freeverb_assist(t_freeverb *x, void *b, long m, long a, char *s)
{
	switch(m) {
		case 1: // inlet
			switch(a) {
				case 0:
				sprintf(s, "(signal/message) Left Input & Control Messages");
				break;
				case 1:
				sprintf(s, "(signal) Right Input");
				break;
			}
		break;
		case 2: // outlet
			switch(a) {
				case 0:
				sprintf(s, "(signal) Left Output");
				break;
				case 1:
				sprintf(s, "(signal) Right Output");
				break;
			}
		break;
	}

}

extern "C" void main(void)
{
	setup((t_messlist **)&freeverb_class,(method)freeverb_new, (method)freeverb_free, 
		(short)sizeof(t_freeverb), 0L, A_DEFFLOAT, 0);
	addmess((method)freeverb_dsp, "dsp", A_CANT, 0);
	addmess((method)freeverb_assist, "assist", A_CANT, 0);
	addmess((method)freeverb_setroomsize, "roomsize", A_FLOAT, 0);
	addmess((method)freeverb_setdamp, "damping", A_FLOAT, 0);
	addmess((method)freeverb_setwidth, "width", A_FLOAT, 0);
	addmess((method)freeverb_setwet, "wet", A_FLOAT, 0);
	addmess((method)freeverb_setdry, "dry", A_FLOAT, 0);
	addmess((method)freeverb_setmode, "freeze", A_FLOAT, 0);
	addmess((method)freeverb_setbypass, "bypass", A_FLOAT, 0);
	addmess((method)freeverb_mute, "clear", 0);
	addmess((method)freeverb_print, "print", 0);
	dsp_initclass();
	finder_addclass("MSP Delays","freeverb~");
	post(version);
}
#endif
