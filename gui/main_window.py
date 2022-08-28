# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'copter_arm.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(386, 527)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.groupBox = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox.setGeometry(QtCore.QRect(100, 180, 281, 291))
        self.groupBox.setObjectName("groupBox")
        self.gridLayoutWidget = QtWidgets.QWidget(self.groupBox)
        self.gridLayoutWidget.setGeometry(QtCore.QRect(10, 30, 261, 51))
        self.gridLayoutWidget.setObjectName("gridLayoutWidget")
        self.gridLayout = QtWidgets.QGridLayout(self.gridLayoutWidget)
        self.gridLayout.setContentsMargins(0, 0, 0, 0)
        self.gridLayout.setObjectName("gridLayout")
        self.label = QtWidgets.QLabel(self.gridLayoutWidget)
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        self.label_position = QtWidgets.QLabel(self.gridLayoutWidget)
        self.label_position.setText("")
        self.label_position.setObjectName("label_position")
        self.gridLayout.addWidget(self.label_position, 0, 1, 1, 1)
        self.label_3 = QtWidgets.QLabel(self.gridLayoutWidget)
        self.label_3.setObjectName("label_3")
        self.gridLayout.addWidget(self.label_3, 1, 0, 1, 1)
        self.label_velocity = QtWidgets.QLabel(self.gridLayoutWidget)
        self.label_velocity.setText("")
        self.label_velocity.setObjectName("label_velocity")
        self.gridLayout.addWidget(self.label_velocity, 1, 1, 1, 1)
        self.groupBox_2 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_2.setGeometry(QtCore.QRect(10, 10, 371, 91))
        self.groupBox_2.setObjectName("groupBox_2")
        self.list_serial_path = QtWidgets.QComboBox(self.groupBox_2)
        self.list_serial_path.setGeometry(QtCore.QRect(10, 30, 351, 25))
        self.list_serial_path.setObjectName("list_serial_path")
        self.btn_connect_serial = QtWidgets.QPushButton(self.groupBox_2)
        self.btn_connect_serial.setGeometry(QtCore.QRect(10, 60, 89, 25))
        self.btn_connect_serial.setObjectName("btn_connect_serial")
        self.label_serial_status = QtWidgets.QLabel(self.groupBox_2)
        self.label_serial_status.setGeometry(QtCore.QRect(110, 60, 281, 20))
        self.label_serial_status.setObjectName("label_serial_status")
        self.groupBox_3 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_3.setGeometry(QtCore.QRect(10, 110, 371, 61))
        self.groupBox_3.setObjectName("groupBox_3")
        self.btn_arm = QtWidgets.QPushButton(self.groupBox_3)
        self.btn_arm.setGeometry(QtCore.QRect(10, 30, 89, 25))
        self.btn_arm.setObjectName("btn_arm")
        self.btn_disarm = QtWidgets.QPushButton(self.groupBox_3)
        self.btn_disarm.setGeometry(QtCore.QRect(110, 30, 89, 25))
        self.btn_disarm.setObjectName("btn_disarm")
        self.label_armed_status = QtWidgets.QLabel(self.groupBox_3)
        self.label_armed_status.setGeometry(QtCore.QRect(210, 30, 151, 20))
        self.label_armed_status.setObjectName("label_armed_status")
        self.groupBox_4 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_4.setGeometry(QtCore.QRect(10, 180, 81, 291))
        self.groupBox_4.setObjectName("groupBox_4")
        self.slider_setpoint = QwtSlider(self.groupBox_4)
        self.slider_setpoint.setGeometry(QtCore.QRect(10, 30, 63, 250))
        self.slider_setpoint.setLowerBound(-60.0)
        self.slider_setpoint.setUpperBound(60.0)
        self.slider_setpoint.setScaleMaxMajor(6)
        self.slider_setpoint.setTotalSteps(120)
        self.slider_setpoint.setObjectName("slider_setpoint")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 386, 22))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.groupBox.setTitle(_translate("MainWindow", "State"))
        self.label.setText(_translate("MainWindow", "position (deg)"))
        self.label_3.setText(_translate("MainWindow", "velocity (deg/s)"))
        self.groupBox_2.setTitle(_translate("MainWindow", "Serial Port Configuration"))
        self.btn_connect_serial.setText(_translate("MainWindow", "Connect"))
        self.label_serial_status.setText(_translate("MainWindow", "status: disconnected"))
        self.groupBox_3.setTitle(_translate("MainWindow", "Arming State"))
        self.btn_arm.setText(_translate("MainWindow", "Arm"))
        self.btn_disarm.setText(_translate("MainWindow", "Disarm"))
        self.label_armed_status.setText(_translate("MainWindow", "status: disarmed"))
        self.groupBox_4.setTitle(_translate("MainWindow", "Setpoint"))
from PyQt5.Qwt import *