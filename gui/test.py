#!/usr/bin/env python

from PyQt5.QtWidgets import QMainWindow, QTextEdit, QAction, QApplication, QPushButton
from OpenGL.GL import *
from OpenGL.GLU import *
from PyQt5.QtGui import QPainter,QColor,QPainterPath,QPolygonF
from PyQt5 import QtOpenGL
from PyQt5.QtWidgets import QWidget, QHBoxLayout
from PyQt5.QtCore import QRectF,QPoint
from PyQt5.QtCore import QTimer

class MainWindow(QWidget):
	def update(self):
		print("a")
		self.setValue()
		self.repaint()
		
	def __init__(self):
		QWidget.__init__(self)
		self.setMaximumHeight(40)
		self.pulse_direction=True
		# self.widget = glWidget(self)

		#self.button = QPushButton('Test', self)

		#mainLayout = QHBoxLayout()
		#mainLayout.addWidget(self.widget)
		#mainLayout.addWidget(self.button)

		#self.setLayout(mainLayout)
		self.value=0.0
		self.enable_pulse=False

		self.timer=QTimer()
		self.timer.timeout.connect(self.update)
		self.timer.start(100);
	def setValue(self,v):
		self.value=v
		
	def setValue(self):
		self.value=self.value+0.02
		if self.value>1.0:
			self.value=0

	def pulse(self):
		self.enable_pulse=True
		
		if self.pulse_direction==True:
			self.value=self.value+0.02
		else:
			self.value=self.value-0.02

		if self.value>=1.0 or self.value<=0.0:
			self.pulse_direction= not self.pulse_direction

		print(self.value)
		
	def paintEvent(self, e):
		w=self.width()
		h=self.height()
		
		if self.enable_pulse==True:
			l=(w-3)*0.2
			bar_start=((w-3)-l)*self.value
		else:
			l=(w-3)*self.value
			bar_start=0
		
		qp = QPainter()
		qp.begin(self)

		color = QColor(0, 0, 0)
		color.setNamedColor('#d4d4d4')
		qp.setBrush(color)
		
		path=QPainterPath()
		path.addRoundedRect(QRectF(0, 0, w, h), 0, 0)
		qp.fillPath(path,QColor(206 , 206, 206));


		path=QPainterPath()
		path.addRoundedRect(QRectF(bar_start, 3, l, h-6), 5, 5)
		qp.fillPath(path,QColor(71 , 142, 216));

		
		#path.addPolygon(QPolygonF([QPoint(0,0), QPoint(10,0), QPoint(10,10), QPoint(10,0)]));
		#qp.setBrush(QColor(71 , 142, 216))

		#path.addPolygon(QPolygonF([QPoint(0,0), QPoint(50,0), QPoint(40,10), QPoint(10,0)]))
		#qp.setBrush(QColor(71 , 142, 216))
		#qp.setBrush(QColor(206 , 207, 206))




		qp.end()


		
if __name__ == '__main__':
    app = QApplication(['Yo'])
    window = MainWindow()
    window.show()
    app.exec_()

