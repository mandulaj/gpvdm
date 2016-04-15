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

#include "device.h"
#include "mesh.h"
#include "inp.h"
#include "util.h"
#include "const.h"
#include "hard_limit.h"
#include <cal_path.h>

void mesh_remesh(struct simulation *sim, struct device *in)
{
	in->ymeshlayers = 1;
	in->meshdata[0].len = in->ylen;
	in->meshdata[0].number = 40;
	mesh_save(sim, in);
	mesh_free(sim, in);
	mesh_load(sim, in);
}

void mesh_save(struct simulation *sim, struct device *in)
{
	int i = 0;
	char buffer[2000];
	char temp[2000];
	char full_file_name[200];

	strcpy(buffer, "");
	strcat(buffer, "#mesh_layers\n");

	sprintf(temp, "%d\n", in->ymeshlayers);
	strcat(buffer, temp);

	for (i = 0; i < in->ymeshlayers; i++) {
		strcat(buffer, "#mesh_layer_length0\n");

		sprintf(temp, "%Le\n", in->meshdata[i].len);
		strcat(buffer, temp);

		strcat(buffer, "#mesh_layer_points0\n");

		sprintf(temp, "%d\n", (int)(in->meshdata[i].number));
		strcat(buffer, temp);
	}

	strcat(buffer, "#ver\n");
	strcat(buffer, "1.0\n");
	strcat(buffer, "#end\n");

	strcpy(full_file_name, "mesh.inp");
	zip_write_buffer(sim, full_file_name, buffer, strlen(buffer));

}

void mesh_free(struct simulation *sim, struct device *in)
{
	device_free(sim, in);
	in->ymeshpoints = 0;
	free(in->meshdata);
}

void mesh_load(struct simulation *sim, struct device *in)
{
	int i;
	struct inp_file inp;
	char token0[200];
	char token1[200];

	inp_init(sim, &inp);
	inp_load_from_path(sim, &inp, get_input_path(sim), "mesh.inp");

	inp_check(sim, &inp, 1.0);

	inp_reset_read(sim, &inp);

	inp_get_string(sim, &inp);	//"#mesh_layers"

	sscanf(inp_get_string(sim, &inp), "%d", &(in->ymeshlayers));

	//config_read_line_to_int(&(in->ymeshlayers),config,"#mesh_layers");

	in->meshdata = malloc(in->ymeshlayers * sizeof(struct mesh));

	in->ymeshpoints = 0;

	for (i = 0; i < in->ymeshlayers; i++) {
		sscanf(inp_get_string(sim, &inp), "%s", token0);
		sscanf(inp_get_string(sim, &inp), "%Lf",
		       &(in->meshdata[i].len));

		sscanf(inp_get_string(sim, &inp), "%s", token1);
		sscanf(inp_get_string(sim, &inp), "%Lf",
		       &(in->meshdata[i].number));

		in->meshdata[i].len = fabs(in->meshdata[i].len);
		hard_limit(sim, token0, &(in->meshdata[i].len));
		in->meshdata[i].den =
		    in->meshdata[i].len / in->meshdata[i].number;
		in->ymeshpoints += in->meshdata[i].number;
	}

	inp_free(sim, &inp);

	device_get_memory(sim, in);

	int pos = 0;
	int ii;
	gdouble dpos = 0.0;
	for (i = 0; i < in->ymeshlayers; i++) {

		for (ii = 0; ii < in->meshdata[i].number; ii++) {
			dpos += in->meshdata[i].den / 2.0;
			in->ymesh[pos] = dpos;
			in->imat[pos] =
			    epitaxy_get_electrical_material_layer(&
								  (in->
								   my_epitaxy),
								  dpos);
			dpos += in->meshdata[i].den / 2.0;
			pos++;
		}
	}

}

void mesh_cal_layer_widths(struct device *in)
{
	int i;
	int cur_i = in->imat[0];

	in->layer_start[cur_i] = 0.0;

	for (i = 0; i < in->ymeshpoints; i++) {
		if ((in->imat[i] != cur_i) || (i == (in->ymeshpoints - 1))) {
			in->layer_stop[cur_i] = in->ymesh[i - 1];	//+(in->ymesh[i]-in->ymesh[i-1])/2;
			in->layer_width[cur_i] =
			    in->layer_stop[cur_i] - in->layer_start[cur_i];
			if (i == (in->ymeshpoints - 1)) {
				break;
			}
			cur_i = in->imat[i];
			in->layer_start[cur_i] = in->ymesh[i];	//-(in->ymesh[i]-in->ymesh[i-1])/2;
		}
//printf("%d\n",in->imat[i]);
	}
}