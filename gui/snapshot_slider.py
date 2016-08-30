#    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
#    model for 1st, 2nd and 3rd generation solar cells.
#    Copyright (C) 2012 Roderick C. I. MacKenzie <r.c.i.mackenzie@googlemail.com>
#
#	www.gpvdm.com
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
from tab import tab_class
from window_list import windows
from cal_path import get_image_file_path

#qt
from PyQt5.QtCore import QSize, Qt 
from PyQt5.QtWidgets import QWidget,QVBoxLayout,QToolBar,QSizePolicy,QAction,QTabWidget,QSlider,QHBoxLayout,QLabel,QComboBox
from PyQt5.QtGui import QPainter,QIcon
from PyQt5.QtCore import pyqtSignal

from help import help_window
from util import read_data_2d


def max_min_2d(array):
	y_len=len(array)
	x_len=len(array[0])
	my_max=array[0][0]
	my_min=array[0][0]

	for x in range(0,x_len):
		for y in range(0,y_len):
			if array[x][y]>my_max:
				my_max=array[x][y]

			if array[x][y]<my_min:
				my_min=array[x][y]

	return [my_max,my_min]

class snapshot_slider(QWidget):

	changed = pyqtSignal()

	def cal_min_max(self):

		self.z_max=-1e40
		self.z_min=1e40

		for i in range(0,len(self.dirs)):
			fname=os.path.join(self.dirs[i],self.files_combo.currentText())
			x=[]
			y=[]
			z=[]
			print("one=",fname)
			if read_data_2d(x,y,z,fname) == True:

				temp_max,temp_min=max_min_2d(z)

				if temp_max>self.z_max:
					self.z_max=temp_max

				if temp_min<self.z_min:
					self.z_min=temp_min
		
	def update(self):
		self.dirs=[]
		if os.path.isdir(self.path)==True:
			for name in os.listdir(self.path):
				if name!="." and name!= "..":
					full_path=os.path.join(self.path, name)
					if os.path.isdir(full_path):
						self.dirs.append(full_path)

		self.slider0.setMaximum(len(self.dirs)-1)		

		self.update_file_combo()

	def slider0_change(self):
		value = self.slider0.value()
		self.label0.setText(str(value))
		self.changed.emit()

	def get_file_name(self):
		file_path=os.path.join(self.path,str(self.slider0.value()),self.files_combo.currentText())
		if os.path.isfile(file_path)==False:
			file_path=None
		return file_path
		
	def __init__(self,path):
		QWidget.__init__(self)
		self.path=path
		
		self.setWindowTitle(_("Snapshot slider")) 
		
		self.main_vbox = QVBoxLayout()

		self.slider_hbox0= QHBoxLayout()

		self.slider0 = QSlider(Qt.Horizontal)
		self.slider0.setMinimum(10)
		self.slider0.setMaximum(30)
		self.slider0.setTickPosition(QSlider.TicksBelow)
		self.slider0.setTickInterval(5)
		self.slider0.valueChanged.connect(self.slider0_change)
		self.slider0.setMinimumSize(300, 80)

		self.slider_hbox0.addWidget(self.slider0)

		self.label0 = QLabel()
		self.label0.setText("")

		self.slider0.setValue(20)

		self.slider_hbox0.addWidget(self.label0)

		self.widget0=QWidget()
		self.widget0.setLayout(self.slider_hbox0)

		self.main_vbox.addWidget(self.widget0)


################
		self.slider_hbox1= QHBoxLayout()
		self.label1 = QLabel()
		self.label1.setText("File")
		self.slider_hbox1.addWidget(self.label1)

		self.files_combo=QComboBox()
		self.slider_hbox1.addWidget(self.files_combo)

		self.update()
		self.files_combo.currentIndexChanged.connect(self.files_combo_changed)

		self.widget1=QWidget()
		self.widget1.setLayout(self.slider_hbox1)

		self.main_vbox.addWidget(self.widget1)

###############

		self.cal_min_max()
		self.setLayout(self.main_vbox)

	def update_file_combo(self):
		self.files_combo.clear()
		path=os.path.join(self.path,str(self.slider0.value()))
		if os.path.isdir(path)==True:
			for name in os.listdir(path):
				full_path=os.path.join(path, name)
				if os.path.isfile(full_path):
					self.files_combo.addItem(name)

		all_items  = [self.files_combo.itemText(i) for i in range(self.files_combo.count())]

		for i in range(0,len(all_items)):
			if all_items[i] == "Jn.dat":
				self.files_combo.setCurrentIndex(i)

	def files_combo_changed(self):
		self.cal_min_max()
		self.changed.emit()


