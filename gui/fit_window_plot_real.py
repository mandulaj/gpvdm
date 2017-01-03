#    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
#    model for 1st, 2nd and 3rd generation solar cells.
#    Copyright (C) 2012-2017 Roderick C. I. MacKenzie <r.c.i.mackenzie@googlemail.com>
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
from numpy import *
from scan_item import scan_item_add
from inp import inp_load_file
from inp import inp_read_next_item
from gui_util import dlg_get_text
from inp import inp_get_token_value
from inp import inp_write_lines_to_file
import webbrowser
from util import time_with_units
from inp_util import inp_search_token_value
from cal_path import get_image_file_path
from scan_item import scan_remove_file
from code_ctrl import enable_betafeatures
from util import read_xyz_data

import i18n
_ = i18n.language.gettext

#qt
from PyQt5.QtCore import QSize, Qt 
from PyQt5.QtWidgets import QWidget,QVBoxLayout,QToolBar,QSizePolicy,QAction,QTabWidget,QMenuBar,QStatusBar, QMenu, QTableWidget, QAbstractItemView
from PyQt5.QtGui import QPainter,QIcon,QCursor

#matplotlib
import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

from gui_util import open_as_filter

from import_data import import_data
mesh_articles = []


class fit_window_plot_real(QWidget):
	lines=[]
	edit_list=[]

	line_number=[]
	save_file_name=""

	file_name=""
	name=""
	visible=1

	def update(self):
		self.draw_graph()
		self.fig.canvas.draw()

	def draw_graph(self):

		x=[]
		y=[]
		z=[]
		if read_xyz_data(x,y,z,"fit_data"+str(self.index)+".inp")==True:

			self.fig.clf()
			self.fig.subplots_adjust(bottom=0.2)
			self.fig.subplots_adjust(left=0.1)
			self.ax1 = self.fig.add_subplot(111)
			self.ax1.ticklabel_format(useOffset=False)

			self.ax1.set_xlabel(_("x"))
			self.ax1.set_ylabel(_("y"))

			voltage, = self.ax1.plot(x,y, 'ro-', linewidth=3 ,alpha=1.0)


	def save_image(self,file_name):
		self.fig.savefig(file_name)

	def callback_save(self):
		file_name=save_as_filter(parent,"png (*.png);;jpg (*.jpg)")

		if file_name!=None:
			self.save_image(file_name)

	def callback_import(self):
		self.im=import_data(os.path.join(os.getcwd(),"fit_data"+str(self.index)+".inp"))
		self.im.run()
		print("return value",self.im)
		if self.im.ret==True:
			self.update()

	def __init__(self,index):
		QWidget.__init__(self)

		self.index=index
		self.fig = Figure(figsize=(5,4), dpi=100)
		self.ax1=None
		self.show_key=True
		
		self.hbox=QVBoxLayout()
		self.edit_list=[]
		self.line_number=[]

		self.list=[]
		print("index=",index)


		canvas = FigureCanvas(self.fig)  # a gtk.DrawingArea
		#canvas.set_background('white')
		#canvas.set_facecolor('white')
		canvas.figure.patch.set_facecolor('white')
		#canvas.set_size_request(500, 150)

		#canvas.set_size_request(700,400)

		self.draw_graph()

		toolbar=QToolBar()
		toolbar.setIconSize(QSize(48, 48))

		self.import_data= QAction(QIcon(os.path.join(get_image_file_path(),"import.png")), _("Import data"), self)
		self.import_data.triggered.connect(self.callback_import)
		toolbar.addAction(self.import_data)

		self.tb_save = QAction(QIcon(os.path.join(get_image_file_path(),"save.png")), _("Save graph"), self)
		self.tb_save.triggered.connect(self.callback_save)
		toolbar.addAction(self.tb_save)


		nav_bar=NavigationToolbar(canvas,self)
		toolbar.addWidget(nav_bar)

		self.hbox.addWidget(toolbar)
		
		self.hbox.addWidget(canvas)

		self.setLayout(self.hbox)
