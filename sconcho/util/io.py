# -*- coding: utf-8 -*-
########################################################################
#
# (c) 2010 Markus Dittrich
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License Version 3 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License Version 3 for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
#######################################################################

from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from __future__ import absolute_import

from PyQt4.QtCore import (QFile, QTextStream, QIODevice, QString,
                          Qt, QRectF, QDataStream)
from PyQt4.QtGui import (QColor, QMessageBox, QImage, QPainter,
                         QPrinter, QPrintDialog, QDialog)
from PyQt4.QtXml import (QDomDocument, QDomNode, QDomElement)
from gui.patternCanvas import (PatternGridItem, PatternLegendItem,
                               legendItem_symbol, legendItem_text)
from util.misc import wait_cursor
from util.exceptions import PatternReadError
import util.messages as msg

# magick number to specify binary API
MAGIC_NUMBER = 0xA3D1
API_VERSION  = 1


#############################################################################
#
# routines for writing a project.
#
#############################################################################
@wait_cursor
def save_project_bin(canvas, colors, activeSymbol, saveFileName):
    """ Toplevel writer routine. """

    # prepare data structures
    patternGridItems = get_patternGridItems(canvas)
    legendItems      = canvas.legend.values()

    status = None
    handle = None
    try:
        handle = QFile(saveFileName)
        if not handle.open(QIODevice.WriteOnly | QIODevice.Truncate):
            raise IOError, unicode(handle.errorString())

        stream = QDataStream(handle)

        # write header
        stream.writeInt32(MAGIC_NUMBER)
        stream.writeInt32(API_VERSION)
        stream.setVersion(QDataStream.Qt_4_5)

        stream.writeInt32(len(patternGridItems))
        stream.writeInt32(len(legendItems))
        stream.writeInt32(len(colors))

        # the next are 5 dummy entries so we can add more
        # output within the same API
        for count in range(5):
            stream.writeInt32(0)

        # write content
        write_patternGridItems_bin(stream, patternGridItems)
        write_legendItems_bin(stream, legendItems)
        write_colors_bin(stream, colors)
        write_active_symbol_bin(stream, activeSymbol)

    except (IOError, OSError) as e:
        status = "Failed to save: %s " % e

    finally:
        if handle is not None:
            handle.close()
        if status is not None:
            return (False, status)

    return (True, None)



def get_patternGridItems(canvas):
    """ Extract all the patternGridItems """

    patternGridItems = []

    for item in canvas.items():
        if isinstance(item, PatternGridItem):
            patternGridItems.append(item)

    return patternGridItems



def write_patternGridItems_bin(stream, items):
    """ Write all patternGridItems to our output stream """

    for item in items:
        stream << QString(item.symbol["category"])  \
               << QString(item.symbol["name"])
        stream.writeInt32(item.column)
        stream.writeInt32(item.row)
        stream.writeInt32(item.width)
        stream.writeInt32(item.height)
        stream << item.color



def write_legendItems_bin(stream, items):
    """ Write all legendItems to our output stream. """

    for item in items:

        symbolItem = legendItem_symbol(item)
        textItem   = legendItem_text(item)

        stream << QString(symbolItem.symbol["category"]) \
               << QString(symbolItem.symbol["name"])
        stream.writeDouble(symbolItem.pos().x())
        stream.writeDouble(symbolItem.pos().y())
        stream.writeDouble(textItem.pos().x())
        stream.writeDouble(textItem.pos().y())
        stream << symbolItem.color                        
        stream << QString(textItem.toPlainText())



def write_colors_bin(stream, items):
    """ Write all colors to our output stream. """
    
    for (color, state) in items:
        stream << color
        stream.writeInt16(state)



def write_active_symbol_bin(stream, activeSymbol):
    """ Write the info regarding the active symbol """

    if activeSymbol:
        stream << QString(activeSymbol["category"])
        stream << QString(activeSymbol["name"])
    else:
        stream << QString("None") << QString("None")





#############################################################################
#
# routines for writing a project.
#
#############################################################################
@wait_cursor
def read_project(openFileName):
    """ Toplevel reader routine. """

    status = None
    handle = None
    try:
        handle = QFile(openFileName)
        if not handle.open(QIODevice.ReadOnly):
            raise IOError, unicode(handle.errorString())

        stream = QDataStream(handle)

        # check header
        magic = stream.readInt32()
        if magic != MAGIC_NUMBER:
            
            # before we give up lets try an the old style
            # sconcho format
            # FIXME: This needs to be punted soon
            try:
                handle.close()
                handle = None
                (patternGridItems, legendItems, colors, activeSymbol) = \
                        read_project_legacy(openFileName)
                return (True, None, patternGridItems, legendItems, colors, \
                        activeSymbol)
            except IOError:
                # give up
                raise IOError, "unrecogized file Type"

        version = stream.readInt32()
        if version != API_VERSION:
            raise IOError, "unsupported API version"

        stream.setVersion(QDataStream.Qt_4_5)

        # start parsing
        numGridItems   = stream.readInt32()
        numLegendItems = stream.readInt32()
        numColors      = stream.readInt32()

        # the next are 5 dummy entries we just skip
        for count in range(5):
            stream.readInt32()

        # write elements
        patternGridItems = read_patternGridItems(stream, numGridItems)
        legendItems      = read_legendItems(stream, numLegendItems)
        colors           = read_colors(stream, numColors)
        activeSymbol     = read_active_symbol(stream)


    except (IOError, OSError) as e:
        status = "Failed to open %s: %s " % (openFileName, e)

    finally:
        if handle is not None:
            handle.close()
        if status is not None:
            return (False, status, None, None, None, None)

    return (True, None, patternGridItems, legendItems, colors, activeSymbol)



def read_patternGridItems(stream, numItems):
    """ Read all patternGridItems from our output stream """

    patternGridItems = []
    for count in range(numItems):
        category = QString()
        stream >> category

        name = QString()
        stream >> name

        column = stream.readInt32()
        row    = stream.readInt32()
        width  = stream.readInt32()
        height = stream.readInt32()
       
        color  = QColor()
        stream >> color

        newItem = { "category" : category,
                    "name"     : name,
                    "column"   : column,
                    "row"      : row,
                    "width"    : width,
                    "height"   : height,
                    "color"    : color}

        patternGridItems.append(newItem)

    return patternGridItems



def read_legendItems(stream, numItems):
    """ Read all legendItems from our output stream """

    legendItems = []
    for count in range(numItems):
        category = QString()
        stream >> category

        name = QString()
        stream >> name

        itemXPos  = stream.readDouble()
        itemYPos  = stream.readDouble()
        labelXPos = stream.readDouble()
        labelYPos = stream.readDouble()
       
        color  = QColor()
        stream >> color

        description = QString()
        stream >> description

        newItem = { "category"    : category,
                    "name"        : name,
                    "itemXPos"    : itemXPos,
                    "itemYPos"    : itemYPos,
                    "labelXPos"   : labelXPos,
                    "labelYPos"   : labelYPos, 
                    "color"       : color,
                    "description" : description}
 
        legendItems.append(newItem)

    return legendItems



def read_colors(stream, numItems):
    """ Write all colors to our output stream. """
   
    colors = []
    for count in range(numItems):
        color = QColor()
        stream >> color
        state = stream.readInt16()

        colors.append((color, state))

    return colors



def read_active_symbol(stream):
    """ Read the previously active symbol if any """

    category = QString()
    stream >> category

    name = QString()
    stream >> name

    activeSymbol = {}
    if category != "None":
        activeSymbol ["category"] = category
        activeSymbol ["name"]     = name

    return activeSymbol



#############################################################################
#
# lecacy routines for parsing a project saved in the old style XML
# format.
#
#############################################################################
@wait_cursor
def read_project_legacy(readFileName):
    """
    Toplevel reader routine.
    """

    readFile = QFile(readFileName)
    if not readFile.open(QIODevice.ReadOnly):
        return

    readDoc = QDomDocument()
    (status, errMsg, errLine, errCol) = readDoc.setContent(readFile, True)
    if not status:
        QMessageBox.critical(None, msg.domParserErrorTitle,
                             msg.domParserErrorText
                             % (unicode(readFileName), errLine, errCol,
                                unicode(errMsg)))
        raise PatternReadError()

    root = readDoc.documentElement()
    if root.tagName() != "sconcho":
        raise IOError

    QMessageBox.critical(None, "Warning: Deprecated File Format Detected",
            "<p> %s is still in the old sconcho XML format! "
            "Support for the old XML format will be removed <b>very soon</b>. "
            "<p> If you still have old sconcho spf "
            "files please convert them into the new format by simply loading "
            "them into sconcho and then saving them again!" % readFileName)

    # list of parsed patternGridItems
    patternGridItems = []
    legendItems      = []
    projectColors    = None
    activeSymbol     = None
    
    node = root.firstChild()
    while not node.isNull():
        if node.toElement().tagName() == "canvasItem":
            item = node.firstChild()

            if item.toElement().tagName() == "patternGridItem":
                newEntry = parse_patternGridItem(item)
                if newEntry:
                    patternGridItems.append(newEntry)

            if item.toElement().tagName() == "legendEntry":
                newEntry = parse_legendItem(item)
                if newEntry:
                    legendItems.append(newEntry)

        elif node.toElement().tagName() == "projectColors":
            item = node.firstChild()
            projectColors = parse_colors(item)

        elif node.toElement().tagName() == "activeSymbol":
            item = node.firstChild()
            activeSymbol = parse_activeSymbol(item)

        node = node.nextSibling()

    return (patternGridItems, legendItems, projectColors, activeSymbol)



def parse_patternGridItem(item):
    """
    Parse a patternGridItem.
    """

    node = item.firstChild()
    while not node.isNull():

        if node.toElement().tagName() == "colIndex":
            (colIndex, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "rowIndex":
            (rowIndex, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "width":
            (width, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "height":
            (height, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "backgroundColor":
            color = node.firstChild().toText().data()

        if node.toElement().tagName() == "patternCategory":
            category = node.firstChild().toText().data()

        if node.toElement().tagName() == "patternName":
            name = node.firstChild().toText().data()

        node = node.nextSibling()

    try:
        newItem = { "column"   : colIndex,
                    "row"      : rowIndex,
                    "width"    : width,
                    "height"   : height,
                    "color"    : QColor(color),
                    "category" : category,
                    "name"     : name }
    except:
        QMessageBox.critical(None, msg.patternGridItemParseErrorTitle,
                             msg.patternGridItemParseErrorText,
                             QMessageBox.Close)
        raise PatternReadError()

    return newItem



def parse_legendItem(item):
    """
    Parse a patternGridItem.
    """

    node = item.firstChild()
    while not node.isNull():

        if node.toElement().tagName() == "patternCategory":
            category = node.firstChild().toText().data()

        if node.toElement().tagName() == "patternName":
            name = node.firstChild().toText().data()
            
        if node.toElement().tagName() == "itemXPos":
            (itemXPos, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "itemYPos":
            (itemYPos, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "labelXPos":
            (labelXPos, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "labelYPos":
            (labelYPos, status) = node.firstChild().toText().data().toInt()

        if node.toElement().tagName() == "backgroundColor":
            color = node.firstChild().toText().data()
            
        if node.toElement().tagName() == "description":
            description = node.firstChild().toText().data()

        node = node.nextSibling()

    try:
        newItem = { "category"  : category,
                    "name"      : name,
                    "itemXPos"  : itemXPos,
                    "itemYPos"  : itemYPos,
                    "labelXPos" : labelXPos,
                    "labelYPos" : labelYPos, 
                    "color"     : QColor(color),
                    "description" : description }
    except:
        QMessageBox.critical(None, msg.patternLegendItemParseErrorTitle,
                             msg.patternLegendItemParseErrorText,
                             QMessageBox.Close)
        raise PatternReadError()

    return newItem



def parse_colors(node):
    """
    Parse a projectColor entry
    """

    colors = []
    while not node.isNull():
        if node.toElement().tagName() == "color":

            colorNode = node.firstChild()
            while not colorNode.isNull():
                
                if colorNode.toElement().tagName() == "name":
                    name = colorNode.firstChild().toText().data()

                if colorNode.toElement().tagName() == "active":
                    (state, status) = colorNode.firstChild().toText().data().toInt()

                colorNode = colorNode.nextSibling()

            try:
                colors.append((QColor(name), state))
            except:
                QMessageBox.critical(None, msg.patternColorParseErrorTitle,
                                     msg.patternColorParseErrorText,
                                     QMessageBox.Close)
                raise PatternReadError()
            
        node = node.nextSibling()

    return colors



def parse_activeSymbol(node):
    """ Parse the active symbol selected in the saved project.

    None means no active symbol was selected.
    """

    activeSymbol = {}
    
    while not node.isNull():
        
        if node.toElement().tagName() == "patternName":
            activeSymbol["name"] = node.firstChild().toText().data()

        if node.toElement().tagName() == "patternCategory":
            activeSymbol["category"] = node.firstChild().toText().data()
            
        node = node.nextSibling()

    return activeSymbol




#############################################################################
#
# routines for exporting a project to an image file
#
#############################################################################
@wait_cursor
def export_scene(canvas, width, height, exportFileName):
    """
    This function exports the scene to a file.
    """
    canvas.clear_all_selected_cells()

    # NOTE: We seem to need the 1px buffer region to avoid
    # the image being cut off
    margin = 10
    theScene = canvas.itemsBoundingRect()
    theScene.adjust(-margin, -margin, margin, margin)

    finalImage = QImage(width+2*margin, height+2*margin, 
                        QImage.Format_ARGB32_Premultiplied )
    finalImage.fill(1)

    painter = QPainter(finalImage)
    painter.setRenderHints(QPainter.SmoothPixmapTransform )
    painter.setRenderHints(QPainter.HighQualityAntialiasing )
    painter.setRenderHints(QPainter.TextAntialiasing )
    painter.setBackgroundMode(Qt.TransparentMode )

    canvas.render(painter, QRectF(), theScene )
    painter.end()
    finalImage.save(exportFileName)




#############################################################################
#
# routines for printing a project 
#
#############################################################################
def print_scene(canvas):
    """
    This function prints the content of the canvas.
    """

    aPrinter = QPrinter(QPrinter.HighResolution)
    printDialog = QPrintDialog(aPrinter)
    
    if printDialog.exec_() == QDialog.Accepted:
        
        theScene = canvas.itemsBoundingRect()
        theScene.adjust(-10, -10, 10, 10)

        painter = QPainter(aPrinter)
        painter.setRenderHints(QPainter.SmoothPixmapTransform)
        painter.setRenderHints(QPainter.HighQualityAntialiasing)
        painter.setRenderHints(QPainter.TextAntialiasing)
        canvas.render(painter, QRectF(), theScene)
        painter.end()