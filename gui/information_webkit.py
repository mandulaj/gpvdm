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
from tab_base import tab_base
from cal_path import get_image_file_path

#qt
from PyQt5.QtCore import QSize, Qt, QUrl
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QWidget, QVBoxLayout,QProgressBar,QLabel,QDesktopWidget,QToolBar,QHBoxLayout,QAction,QSizePolicy,QLineEdit
from PyQt5.QtGui import QPixmap
from PyQt5.QtWebKitWidgets import QWebView

class information(QWidget,tab_base):

	def update_buttons(self, widget, data=None):
		self.url_bar.set_text( widget.get_main_frame().get_uri() )
		self.back_button.set_sensitive(self.web.can_go_back())
		self.forward_button.set_sensitive(self.web.can_go_forward())

	def browse(self):

		url = self.tb_url.text()
		if url.count("://")==0:
		    url = "http://"+url
		self.tb_url.setText(url)

		self.html.load(QUrl(url))

	def home(self):
		self.tb_url.setText(self.default_url)
		self.browse()

	def __init__(self):
		QWidget.__init__(self)

		self.setMinimumSize(1000,500)
		self.html = QWebView()

		vbox=QVBoxLayout()

		toolbar=QToolBar()
		toolbar.setIconSize(QSize(48, 48))

		back = QAction(QIcon(os.path.join(get_image_file_path(),"back.png")),  _("back"), self)
		back.triggered.connect(self.html.back)
		toolbar.addAction(back)

		home = QAction(QIcon(os.path.join(get_image_file_path(),"home.png")),  _("home"), self)
		home.triggered.connect(self.home)
		toolbar.addAction(home)

		self.tb_url=QLineEdit()
		self.tb_url.returnPressed.connect(self.browse)
		toolbar.addWidget(self.tb_url)
		
		vbox.addWidget(toolbar)

		self.default_url = "http://www.gpvdm.com/welcome.html"
		self.tb_url.setText(self.default_url)
		self.browse()
		vbox.addWidget(self.html)

		self.setLayout(vbox)
		return

