# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/update_dialog.ui'
#
# Created: Mon Oct 24 22:13:55 2011
#      by: PyQt4 UI code generator 4.8.5
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_UpdateDialog(object):
    def setupUi(self, UpdateDialog):
        UpdateDialog.setObjectName(_fromUtf8("UpdateDialog"))
        UpdateDialog.resize(357, 154)
        UpdateDialog.setWindowTitle(QtGui.QApplication.translate("UpdateDialog", "Check For Updates", None, QtGui.QApplication.UnicodeUTF8))
        UpdateDialog.setModal(True)
        self.verticalLayout = QtGui.QVBoxLayout(UpdateDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.updateTextEdit = QtGui.QTextEdit(UpdateDialog)
        self.updateTextEdit.setObjectName(_fromUtf8("updateTextEdit"))
        self.verticalLayout.addWidget(self.updateTextEdit)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.closeButton = QtGui.QPushButton(UpdateDialog)
        self.closeButton.setText(QtGui.QApplication.translate("UpdateDialog", "Close", None, QtGui.QApplication.UnicodeUTF8))
        self.closeButton.setObjectName(_fromUtf8("closeButton"))
        self.horizontalLayout.addWidget(self.closeButton)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.retranslateUi(UpdateDialog)
        QtCore.QMetaObject.connectSlotsByName(UpdateDialog)

    def retranslateUi(self, UpdateDialog):
        pass
