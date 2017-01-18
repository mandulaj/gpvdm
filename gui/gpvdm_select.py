#    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
#    model for 1st, 2nd and 3rd generation solar cells.
#    Copyright (C) 2012-2017 Roderick C. I. MacKenzie r.c.i.mackenzie at googlemail.com
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
#from global_objects import global_object_get
from plot_io import get_plot_file_info
from plot_state import plot_state

#qt
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import QSize, Qt, QTimer
from PyQt5.uic import loadUi
from PyQt5.QtWidgets import QApplication,QGraphicsScene,QListWidgetItem,QListView,QLineEdit,QWidget,QHBoxLayout,QPushButton,QLineEdit
from PyQt5.QtGui import QPixmap

#cal_path
from cal_path import get_image_file_path
from cal_path import get_ui_path
from open_save_dlg import open_as_filter

from help import help_window

import i18n
_ = i18n.language.gettext


class gpvdm_select(QWidget):

	def __init__(self,file_box=False):
		QWidget.__init__(self)
		self.hbox=QHBoxLayout()
		self.edit=QLineEdit()
		self.button=QPushButton()
		self.button.setFixedSize(25, 25)
		self.button.setText("...")
		self.hbox.addWidget(self.edit)
		self.hbox.addWidget(self.button)

		self.hbox.setContentsMargins(0, 0, 0, 0)
		self.edit.setStyleSheet("QLineEdit { border: none }");

		if file_box==True:
			self.button.clicked.connect(self.callback_button_click)

		self.setLayout(self.hbox)

	def callback_button_click(self):
		file_name=open_as_filter(self,"dat (*.dat);;csv (*.csv);;txt (*.txt)")
		if file_name!=None:
			self.setText(file_name)

	def setText(self,text):
		self.edit.setText(text)
	
	def text(self):
		return self.edit.text()
		
