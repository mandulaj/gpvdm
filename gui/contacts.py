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
from numpy import *
from inp import inp_load_file
import webbrowser
from inp_util import inp_search_token_value
from cal_path import get_image_file_path
from window_list import windows

from epitaxy import epitaxy_get_layers
from epitaxy import epitaxy_get_dos_file
from epitaxy import epitaxy_get_width
from inp import inp_update
from scan_item import scan_item_add

import i18n
_ = i18n.language.gettext


#contacts io
from contacts_io import segment
from contacts_io import contacts_save
from contacts_io import contacts_get_array
from contacts_io import contacts_clear
from contacts_io import contacts_print
from contacts_io import contacts_load
from contacts_io import contacts_print
from contacts_io import contacts_append

#qt
from PyQt5.QtWidgets import QMainWindow, QTextEdit, QAction, QApplication
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import QSize, Qt 
from PyQt5.QtWidgets import QWidget,QSizePolicy,QHBoxLayout,QPushButton,QDialog,QFileDialog,QToolBar, QMessageBox, QVBoxLayout, QGroupBox, QTableWidget,QAbstractItemView, QTableWidgetItem


class contacts_window(QWidget):

	visible=1

	def update_contact_db(self):
		contacts_clear()
		for i in range(0,self.tab.rowCount()):
			contacts_append(float(self.tab.item(i, 0).text()),float(self.tab.item(i, 1).text()),float(self.tab.item(i, 2).text()),float(self.tab.item(i, 3).text()))	


	def on_add_clicked(self, button):
		self.tab.blockSignals(True)
		index = self.tab.selectionModel().selectedRows()

		print index
		if len(index)>0:
			pos=index[0].row()+1
		else:
			pos = self.tab.rowCount()

		self.tab.insertRow(pos)

		self.tab.setItem(pos,0,QTableWidgetItem(_("start")))
		self.tab.setItem(pos,1,QTableWidgetItem(_("width")))
		self.tab.setItem(pos,2,QTableWidgetItem(_("depth")))
		self.tab.setItem(pos,3,QTableWidgetItem(_("voltage")))

		self.save()
		self.tab.blockSignals(False)

	def on_remove_clicked(self, button):
		self.tab.blockSignals(True)
		index = self.tab.selectionModel().selectedRows()

		print index
		if len(index)>0:
			pos=index[0].row()
			self.tab.removeRow(pos)

		self.save()
		self.tab.blockSignals(False)

	def save(self):
		self.update_contact_db()
		contacts_save()

	def callback_close(self, widget, data=None):
		self.win_list.update(self,"contact")
		self.hide()
		return True

	def callback_help(self):
		webbrowser.open('http://www.gpvdm.com/man/index.html')

	def tab_changed(self, x,y):
		print x,y
		self.save()

	def load(self):
		self.tab.clear()
		self.tab.setHorizontalHeaderLabels([_("Start"), _("Width"),_("Depth"),_("Voltage")])
		contacts_load()
		contacts_print()
		contacts=contacts_get_array()
		self.tab.setRowCount(len(contacts))
		i=0
		for c in contacts_get_array():

			value = QTableWidgetItem(str(c.start))
			self.tab.setItem(i,0,value)

			value = QTableWidgetItem(str(c.width))
			self.tab.setItem(i,1,value)

			value = QTableWidgetItem(str(c.depth))
			self.tab.setItem(i,2,value)

			value = QTableWidgetItem(str(c.voltage))
			self.tab.setItem(i,3,value)
			i=i+1


		return

	def __init__(self):
		QWidget.__init__(self)
		self.setFixedSize(500, 400)

		self.win_list=windows()
		self.win_list.set_window(self,"contacts")

		self.setWindowIcon(QIcon(os.path.join(get_image_file_path(),"contact.png")))

		self.setWindowTitle(_("Edit contacts (www.gpvdm.com)")) 
		
		self.main_vbox = QVBoxLayout()

		toolbar=QToolBar()
		toolbar.setIconSize(QSize(48, 48))

		add = QAction(QIcon(os.path.join(get_image_file_path(),"add.png")),  _("Add contact"), self)
		add.triggered.connect(self.on_add_clicked)
		toolbar.addAction(add)

		remove = QAction(QIcon(os.path.join(get_image_file_path(),"minus.png")),  _("Remove contacts"), self)
		remove.triggered.connect(self.on_remove_clicked)
		toolbar.addAction(remove)

		spacer = QWidget()
		spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
		toolbar.addWidget(spacer)


		self.help = QAction(QIcon(os.path.join(get_image_file_path(),"help.png")), 'Hide', self)
		self.help.setStatusTip(_("Close"))
		self.help.triggered.connect(self.callback_help)
		toolbar.addAction(self.help)

		self.main_vbox.addWidget(toolbar)

		self.tab = QTableWidget()
		self.tab.resizeColumnsToContents()

		self.tab.verticalHeader().setVisible(False)

		self.tab.clear()
		self.tab.setColumnCount(4)
		self.tab.setSelectionBehavior(QAbstractItemView.SelectRows)

		self.load()

		self.tab.cellChanged.connect(self.tab_changed)

		self.main_vbox.addWidget(self.tab)


		self.setLayout(self.main_vbox)

		return
