#    General-purpose Photovoltaic Device Model - a drift diffusion base/Shockley-Read-Hall
#    model for 1st, 2nd and 3rd generation solar cells.
#    Copyright (C) 2012 Roderick C. I. MacKenzie <r.c.i.mackenzie@googlemail.com>
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
from tab_base import tab_base
from help import my_help_class
from cal_path import get_image_file_path

import i18n
_ = i18n.language.gettext

#qt
from PyQt5.QtWidgets import QMainWindow, QTextEdit, QAction, QApplication
from PyQt5.QtGui import QIcon,QPixmap,QPalette
from PyQt5.QtCore import QSize, Qt
from PyQt5.QtWidgets import QWidget,QScrollArea,QSizePolicy,QHBoxLayout,QPushButton,QDialog,QFileDialog,QToolBar, QMessageBox, QVBoxLayout, QGroupBox, QTableWidget,QAbstractItemView, QTableWidgetItem, QLabel

class information(QScrollArea,tab_base):

	lines=[]
	edit_list=[]
	file_name=""
	line_number=[]
	save_file_name=""
	name="Welcome"

	def __init__(self):
		QScrollArea.__init__(self)
		self.main_widget=QWidget()

		hbox=QHBoxLayout()
		self.setStyleSheet("background-color:white;");
		self.label = QLabel()
		#self.label.setStyleSheet("background-color:white;");

		self.label.setAlignment(Qt.AlignTop)
		self.label.setOpenExternalLinks(True);
		#self.label.setSizePolicy( QSizePolicy.Ignored, QSizePolicy.Ignored );
		#self.label.setMaximumSize( QSize(16777215, 16777215) )
		self.label.setWordWrap(True)
		text="<b><font size=30>"
		text=text+"General-purpose photovoltaic device model"
		text=text+"</b><br>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~<br></font>"
		text=text+"<br><font size=12>(<a href=\"http://www.gpvdm.com\" title=\""
		text=text+_("Click to find out more")
		text=text+"\">www.gpvdm.com</a>)<br><br> "
		text=text+_("To make a new simulation click <i><u>N</u>ew Simulation</i> in the <i><u>F</u>ile</i> menu<br> or to open an existing simulation select <i><u>O</u>pen simulation</i>. ")
		text=text+"<br> <br> "
		text=text+_("There is more help on the <a href=\"http://www.gpvdm.com/man/index.html\">man pages</a>.")
		text=text+"<br><br>"
		text=text+_("Please report bugs to <br><a href=\"mailto:roderick.mackenzie@nottingham.ac.uk?Subject=gpvdm%20bug\">")
		text=text+"roderick.mackenzie@nottingham.ac.uk</a>.<br><br><br> Rod<br>18/10/13<br>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~<br><br><br><br></font>"
		self.label.setText(text)
		#self.label.setMinimumSize( QSize(0,0) )

		hbox.addWidget(self.label)

		image = QLabel()
		pixmap = QPixmap(os.path.join(get_image_file_path(),"image.jpg"))
		image.setPixmap(pixmap)

		hbox.addWidget(image)

		self.main_widget.setLayout(hbox)
		self.main_widget.show()
		self.setWidget(self.main_widget)

	def update(self):
		print("")
		
	def help(self):
		my_help_class.help_set_help(["icon.png",_("<big><b>Welcome to gpvdm</b></big>\n The window will provide you with information about new versions and bugs in gpvdm.")])


