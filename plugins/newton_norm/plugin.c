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

#include "newton.h"
#include <dll_interface.h>
#include <solver_interface.h>
#include <dll_export.h>

struct dll_interface *fun;

EXPORT void set_interface(struct dll_interface *in)
{
	fun = in;
}

EXPORT void dll_newton_set_min_ittr(int ittr)
{
	dllinternal_newton_set_min_ittr(ittr);
}

EXPORT int dll_solve_cur(struct simulation *sim, struct device *in)
{
	return dllinternal_solve_cur(sim, in);
}

EXPORT void dll_solver_realloc(struct simulation *sim, struct device *in)
{
	dllinternal_solver_realloc(sim, in);
}

EXPORT void dll_solver_free_memory(struct device *in)
{
	dllinternal_solver_free_memory(in);
}
