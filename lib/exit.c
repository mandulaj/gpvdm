//    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
//    model for 1st, 2nd and 3rd generation solar cells.
//    Copyright (C) 2012 Roderick C. I. MacKenzie
//
//      roderick.mackenzie@nottingham.ac.uk
//      www.roderickmackenzie.eu
//      Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/time.h>
#include "util.h"
#include "log.h"
#include "const.h"
//#include "dump_ctrl.h"
//#include "dos_types.h"
#include "gui_hooks.h"
#include "lang.h"

static char lock_name[100];
static char lock_data[100];

void set_ewe_lock_file(char *lockname, char *data)
{
	strcpy(lock_name, lockname);
	strcpy(lock_data, data);
}

int ewe(const char *format, ...)
{
	FILE *out;
	char temp[1000];
	char temp2[1000];
	va_list args;
	va_start(args, format);
	vsprintf(temp, format, args);

	sprintf(temp2, "error:%s", temp);

	printf_log("%s\n", temp2);

	//gui_send_data(temp2);

	va_end(args);

	//server_send_finished_to_gui(&globalserver);

	out = fopen("server_stop.dat", "w");
	fprintf(out, "solver\n");
	fclose(out);

	if (strcmp(lock_name, "") != 0) {
		out = fopen(lock_name, "w");
		fprintf(out, "%s", lock_data);
		fclose(out);
	}

	exit(1);
}
