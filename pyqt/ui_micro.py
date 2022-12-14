from PyQt5 import QtWidgets, QtCore, QtGui
from pyqtgraph import PlotWidget, plot
from PyQt5.Qt import Qt
import pyqtgraph as pg
import sys 
import os
from random import randint
import serial.tools.list_ports
from PyQt5.QtWidgets import *
from PyQt5 import QtGui,QtCore
import re

ports = serial.tools.list_ports.comports()
serialInst = serial.Serial()
portsList = []

for onePort in ports:
    portsList.append(str(onePort))
    print(str(onePort))

val = input("Select Port: COM")

for x in range(0,len(portsList)):
    if portsList[x].startswith("COM" + str(val)):
        portVar = "COM" + str(val)
        print(portVar)

serialInst.baudrate = 115200
serialInst.port = portVar
serialInst.open()

class MainWindow(QtWidgets.QMainWindow):

    rotation_sensor=0
    light_sensor=0

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        
        self.widget = QWidget()
        layout = QGridLayout()
        self.widget.setLayout(layout)

        
        self.graphWidget_light = pg.PlotWidget()
        self.graphWidget_rotation = pg.PlotWidget()
        
        layout.addWidget(self.graphWidget_light, 0, 0)
        layout.addWidget(self.graphWidget_rotation, 0, 1)

        self.setCentralWidget(self.widget)
        self.setWindowTitle("Setting threshold to light sensor")

        self.xr = list(range(100))  # 100 time points
        self.yr = [30 for _ in range(100)]  # 100 data points
        
        self.xl = list(range(100))  # 100 time points
        self.yl = [30 for _ in range(100)]  # 100 data points

        self.graphWidget_rotation.setBackground('k')
        self.graphWidget_light.setBackground('k')

        color1 = pg.mkPen(color=(0, 255, 255))
        self.data_line =  self.graphWidget_rotation.plot(self.xr, self.yr, pen=color1)

        color2 = pg.mkPen(color=(255,0,0))
        self.data_line2 =  self.graphWidget_light.plot(self.xl, self.yl, pen=color2)
    
    def update_plot_data_rotation(self):
        self.xr = self.xr[1:]  # Remove the first y element.
        self.xr.append(self.xr[-1] + 1)  # Add a new value 1 higher than the last.
        if self.rotation_sensor<6000:
            self.yr = self.yr[1:]  # Remove the first 
            self.yr.append(self.rotation_sensor)  
            self.data_line.setData(self.xr, self.yr)  # Update the data.
        
    def update_plot_data_light(self):
        self.xl = self.xl[1:]  # Remove the first y element.
        self.xl.append(self.xl[-1] + 1)  # Add a new value 1 higher than the last.

        self.yl = self.yl[1:]  # Remove the first 
        self.yl.append(self.light_sensor)  
        if self.light_sensor>0 and self.light_sensor<2000:
            # daca e valoare mica => VERDE (0,255,0)
            color1 = pg.mkPen(color=(0,255,0))    
            self.data_line2.setData(self.xl, self.yl, pen = color1)  # Update the data.
        elif self.light_sensor>2000 and self.light_sensor<4000:
            # daca e valoare mica => GALBEN (255,255,0)
            color2 = pg.mkPen(color=(255,255,0))    
            self.data_line2.setData(self.xl, self.yl, pen=color2)
        elif self.light_sensor>4000 and self.light_sensor<6000:
            # daca e valoare mica => RED (255,0,0)
            color3 = pg.mkPen(color=(255,0,0))    
            self.data_line2.setData(self.xl, self.yl, pen=color3)
            # Update the data.

def get_data_from_serial():
    global window1
    global data
    if serialInst.in_waiting:
        packet=serialInst.readline()
       
        buffer=packet.decode('utf',errors='replace').strip("\n")
        aux=buffer.split(" ")
        if len(aux) == 3:
            if aux[1].isnumeric():
                if "Light" in aux[0]:
                    data=aux[1]
                    print(data)
                    window1.light_sensor=int(data)
                    
                else :
                    aux=buffer.split(" ")
                    data=aux[1]
                    print(data)
                    window1.rotation_sensor=int(data)


app = QtWidgets.QApplication(sys.argv)
window1 = MainWindow()
window1.show()


timer = QtCore.QTimer()
timer2 = QtCore.QTimer()

timer.setInterval(50)
timer2.setInterval(1)
timer2.timeout.connect(get_data_from_serial)

timer.timeout.connect(window1.update_plot_data_rotation)
timer.timeout.connect(window1.update_plot_data_light)
timer2.start()
timer.start()

sys.exit(app.exec_())
            