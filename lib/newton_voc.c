//
//  General-purpose Photovoltaic Device Model gpvdm.com- a drift diffusion
//  base/Shockley-Read-Hall model for 1st, 2nd and 3rd generation solarcells.
// 
//  Copyright (C) 2012 Roderick C. I. MacKenzie <r.c.i.mackenzie@googlemail.com>
//
//	www.roderickmackenzie.eu
//	Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
//
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.


#include <dump.h>
#include <sim.h>
#include <exp.h>
#include <log.h>

void newton_aux_voc(struct device *in,gdouble V,gdouble* i,gdouble* didv,gdouble* didphi,gdouble* didxil,gdouble* didxipl,gdouble* didphir,gdouble* didxir,gdouble* didxipr)
{
return;
}


gdouble newton_sim_voc_fast(struct simulation *sim,struct device *in,int do_LC)
{
newton_sim_voc(sim,in);
return get_I(in)+in->C*(in->Vapplied-in->Vapplied_last)+in->Vapplied/in->Rshunt;
}

gdouble newton_sim_voc(struct simulation *sim, struct device *in)
{
printf_log(sim,"Looking for Voc\n");
gdouble C=in->C;
gdouble clamp=0.1;
gdouble step=0.01;
gdouble e0;
gdouble e1;
gdouble i0;
gdouble i1;
gdouble deriv;
gdouble Rdrain=in->Rload+in->Rcontact;

solve_all(sim,in);
i0=get_I(in);
e0=fabs(i0+in->Vapplied*(1.0/in->Rshunt-1.0/Rdrain));

in->Vapplied+=step;
solve_all(sim,in);
i1=get_I(in);
e1=fabs(i1+in->Vapplied*(1.0/in->Rshunt-1.0/Rdrain));

deriv=(e1-e0)/step;
step=-e1/deriv;

step=step/(1.0+fabs(step/clamp));
in->Vapplied+=step;

int count=0;
int max=200;
do
{
	e0=e1;
	solve_all(sim,in);
	i1=get_I(in);
	e1=fabs(i1+in->Vapplied*(1.0/in->Rshunt-1.0/Rdrain));
	deriv=(e1-e0)/step;
	step=-e1/deriv;

	step=step/(1.0+fabs(step/clamp));
	in->Vapplied+=step;
	if (get_dump_status(sim,dump_print_text)==TRUE)
	{
		printf_log(sim,"%d voc find Voc Vapplied=%Lf step=%Le error=%Le\n",count,in->Vapplied,step,e1);
	}
	if (count>max) break;
	count++;

}while(e1>1e-12);

gdouble ret=in->Vapplied-C*(i1-in->Ilast)/in->dt;
return ret;
}

void set_light_for_voc(struct simulation *sim,struct device *in,gdouble Voc)
{
}
