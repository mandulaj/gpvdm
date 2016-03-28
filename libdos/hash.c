//
//  General-purpose Photovoltaic Device Model gpvdm.com- a drift diffusion
//  base/Shockley-Read-Hall model for 1st, 2nd and 3rd generation solarcells.
// 
//  Copyright (C) 2012 Roderick C. I. MacKenzie
//
//      roderick.mackenzie@nottingham.ac.uk
//      www.roderickmackenzie.eu
//      Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include <math.h>

int hashget(gdouble * x, int N, gdouble find)
{
	static gdouble *x_ = NULL;
	static gdouble find_ = 0.0;
	static int steps_ = 0.0;
	if (N == 1)
		return 0;
	if ((x_ == x) && (find_ == find))
		return steps_;
	gdouble x0 = x[0];
	gdouble x1 = x[1];
	gdouble delta = find - x0;
	gdouble step = x1 - x0;
	int steps = delta / step;

	if (steps > (N - 2))
		steps = N - 2;
	if (steps < 0)
		steps = 0;
//printf("%ld\n",steps);
	x_ = x;
	find_ = find;
	steps_ = steps;
	return steps;
}