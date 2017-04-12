#    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
#    model for 1st, 2nd and 3rd generation solar cells.
#    Copyright (C) 2012 Roderick C. I. MacKenzie r.c.i.mackenzie at googlemail.com
#
#	https://www.gpvdm.com
#	Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License v2.0, as published by
#    the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


import os
import shutil

class dat_file():
	valid_data=False
	grid=False
	show_pointer=False
	logy=False
	logx=False
	logz=False
	logdata=False
	label_data=False
	invert_y=False
	normalize=False
	norm_to_peak_of_all_data=False
	subtract_first_point=False
	add_min=False
	legend_pos="lower right"
	ymax=-1
	ymin=-1
	x_label=""
	y_label=""
	z_label=""
	data_label=""
	x_units=""
	y_units=""
	z_units=""
	data_units=""
	x_mul=1.0
	y_mul=1.0
	z_mul=1.0
	data_mul=1.0
	key_units=""
	file0=""
	tag0=""
	file1=""
	tag1=""
	file2=""
	tag2=""
	example_file0=""
	example_file1=""
	example_file2=""
	time=0.0
	Vexternal=0.0
	file_name=""
	other_file=""
	title=""
	type="xy"
	section_one=""
	section_two=""

	x_start=0
	x_stop=1
	x_points=25
	y_start=0
	y_stop=1
	y_points=25
	x_len=0
	y_len=0
	z_len=0
	
	x_scale=[]
	y_scale=[]
	z_scale=[]
	data=[]
	labels=[]
