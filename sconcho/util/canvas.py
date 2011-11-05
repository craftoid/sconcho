# -*- coding: utf-8 -*-
########################################################################
#
# (c) 2009-2011 Markus Dittrich
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

import math

from PyQt4.QtCore import (QPointF)
from PyQt4.QtGui import (QMessageBox)

import sconcho.util.messages as msg 


def convert_pos_to_col_row(mousePos, cellWidth, cellHeight):
    """ Converts a mouse position on the canvas into a tuple
    of (column, row).
    Note: This may be outside the actual pattern grid.

    """

    column = int( math.floor( mousePos.x()/cellWidth ) )
    row    = int( math.floor( mousePos.y()/cellHeight ) )

    return (column, row)



def convert_col_row_to_pos(column, row, cellWidth, cellHeight):
    """ Converts a (row, column) tuple to a position on
    the canvas in the center of the corresponding grid element.

    """

    return QPointF((column + 0.5) * cellWidth, 
                   (row + 0.5) * cellHeight)



def is_click_in_grid(col, row, numCols, numRows):
    """ Returns true if col and row is within the limits
    set by numCol and numRow.

    """

    if ( col >= 0 and col < numCols ) and ( row >= 0 and row < numRows ):
        return True
    else:
        return False

    

def is_click_on_labels(col, row, numCols, numRows):
    """ Returns true if col and row is within the grid labels. 

    NOTE: We accept both clicks on the right/left or top/bottom
    of the grid.
    """

    if (((col == numCols or col == -1) and (row >= 0 and row < numRows)) or 
        ((row == -1 or row == numRows) and (col >= 0 and col < numCols))):
        return True
    else:
        return False



def shift_item_row_wise(item, num, unitCellHeight):
    """ Shifts the given item by num rows given unitCellHeight. """
    
    yShift = num * unitCellHeight
    item.prepareGeometryChange()
    item.row += num
    item.setPos(item.pos() + QPointF(0.0, yShift))



def shift_item_column_wise(item, num, unitCellWidth):
    """ Shifts the given item by num columns given unitCellWidth. """
    
    xShift = num * unitCellWidth
    item.prepareGeometryChange()
    item.column += num
    item.setPos(item.pos() + QPointF(xShift, 0.0))



def shift_legend_vertically(legendList, rowShift, unitCellHeight, 
                            numColumns, unitWidth):
    """ Shift all legend items below the grid down by rowShift. """

    yShift = rowShift * unitCellHeight

    for item in legendList.values():
        symbol = legendItem_symbol(item)
        text   = legendItem_text(item)

        # we ignore all items above or right of the pattern grid
        if (symbol.scenePos().y() >= 0) and \
           (symbol.scenePos().x() <= numColumns * unitWidth):

            symbol.prepareGeometryChange()
            symbol.setPos(symbol.pos() + QPointF(0.0, yShift))
            
        
        if (text.scenePos().y() >= 0) and \
           (text.scenePos().x() <= numColumns * unitWidth):

            text.prepareGeometryChange()
            text.setPos(text.pos() + QPointF(0.0, yShift)) 



def shift_selection_vertically(selection, pivot, rowShift):
        """ Shifts all items in the current selection that are below the 
        pivot down. 
        
        """

        newSelection = {}
        for (key, entry) in selection.items():

            if entry.row >= pivot:
                entry.row += rowShift

            newID = get_item_id(entry.column, entry.row)
            newSelection[newID] = entry

        return newSelection



def shift_legend_horizontally(legendList, columnShift, unitCellWidth,
                              numRows, unitHeight):
    """ Shift all legend items to the right of the grid right by
    columnShift.
    
    """

    xShift = columnShift * unitCellWidth

    for item in legendList.values():
        symbol = legendItem_symbol(item)
        text = legendItem_text(item)

        # we ignore all items above or right of the
        # pattern grid
        if (symbol.scenePos().x() >= 0) and \
           (symbol.scenePos().y() >= 0) and \
           (symbol.scenePos().y() <= numRows * unitHeight):

            symbol.prepareGeometryChange()
            symbol.setPos(symbol.pos() + QPointF(xShift, 0.0))
            

        if (text.scenePos().x() >= 0) and \
           (text.scenePos().y() >= 0) and \
           (text.scenePos().y() <= numRows * unitHeight):

            text.prepareGeometryChange()
            text.setPos(text.pos() + QPointF(xShift, 0.0))



def shift_selection_horizontally(selection, pivot, columnShift):
        """ Shifts all items in the current selection that are right
        of the pivot to the right. 
        
        """

        newSelection = {}
        for (key, entry) in selection.items():

            if entry.column >= pivot:
                entry.column += columnShift

            newID = get_item_id(entry.column, entry.row)
            newSelection[newID] = entry

        return newSelection



def compute_max_legend_y_coordinate(gridLegend):
    """ Given the current list of existing legend items
    figure out the largest y coordinate among them all.

    """

    yList = [0]
    for item in gridLegend.values():
        yList.append(legendItem_symbol(item).scenePos().y())
        yList.append(legendItem_text(item).scenePos().y())

    return max(yList)



def change_count(item, count):
    """ Convenience wrapper changing the count for a particular
    legend entry.

    """

    item[0] += count
    return item



def legendItem_count(item):
    """ Convenience wrapper returning the reference count for the
    particular legend item.

    """

    return item[0]



def legendItem_symbol(item):
    """ Convenience wrapper returning the current symbol for a
    particular legend item.

    """

    return item[1]



def legendItem_text(item):
    """ Convenience wrapper returning the current description text
    for a particular legend item.

    """

    return item[2]



def generate_legend_id(symbol, color):
    """ Based on a symbol/legend info, generate an id tag. Currently
    this is just based on name and category.

    """

    name = symbol["name"]
    category = symbol["category"]

    return (name, color.name())



def arrange_label_item(legendItems, legendID, itemXPos, itemYPos, labelXPos, 
                       labelYPos, description):
    """ Position all label items (pairs of PatternGridItem
    and PatternLegendItem) as requested in dict legendItems
    which comes from a parsed spf file.

    """

    if legendID in legendItems:
        
        legendItem = legendItems[legendID]
        legendPatternItem = legendItem_symbol(legendItem)
        legendTextItem = legendItem_text(legendItem)
        legendPatternItem.setPos(itemXPos, itemYPos)
        legendTextItem.setPos(labelXPos, labelYPos)
        legendTextItem.setPlainText(description)

    else:
        QMessageBox.critical(None, msg.errorMatchingLegendItemTitle,   
                             msg.errorMatchingLegendItemText,
                             QMessageBox.Close)



def is_active_selection_rectangular(selectedCells):
    """ This function checks if the currently active selection 
    is rectangular (i.e., not jagged or disconnected).
    The function returns (True, (col, row)) if yes and (False, (0,0)) 
    otherwise. Here, col and row and the number of columns and rows
    of the selected rectangle.

    """

    if not selectedCells:
        return (False, (0,0))

    cellsByRow = order_selection_by_rows(selectedCells)
   
    # make sure the rows are consecutive
    rowIDs = cellsByRow.keys()
    rowIDs.sort()
    for item in range(1, len(rowIDs)):
        if (rowIDs[item] - rowIDs[item-1]) != 1:
            return(False, (0,0))

    # check that each row has the same number of unit cells
    values = set(num_unitcells(row) for row in cellsByRow.values())
    if len(values) != 1:
        return (False, (0,0))

    # look for "holes"
    for row in cellsByRow.values():
        row.sort(lambda x, y: cmp(x.column, y.column))
        if not are_consecutive([row]):
            return (False, (0,0))
    
    numCols = values.pop()
    numRows = len(cellsByRow)
    return (True, (numCols, numRows))



def can_outline_selection(selection):
    """ This function determines if the currently action selection
    can be outlined. This requires the selection to be connected
    without any holes.

    """

    if len(selection) == 0:
        return False

    # check that rows are consecutive
    cellsByRow = order_selection_by_rows(selection)
    keys = cellsByRow.keys()
    keys.sort()
    differences = set([(j-i) for (i,j) in zip(keys, keys[1:])])
    if len(differences) > 1: 
        return False
    elif len(differences) == 1 and (1 not in differences):
        return False

    # check that each row has no holes
    for row in cellsByRow.values():
        row.sort(lambda x, y: cmp(x.column, y.column))
        if not are_consecutive([row]):
            return False

    return True



def order_selection_by_rows(selection):
    """ Given a list of selected grid cells order them by row. """

    cellsByRow = {}
    if selection:
        for cell in selection:
            if not cell.row in cellsByRow:
                cellsByRow[cell.row] = [cell]
            else:
                cellsByRow[cell.row].append(cell)

    return cellsByRow



def order_selection_by_columns(selection):
    """ Given a list of selected grid cells order them by column. """

    cellsByColumn = {}
    if selection:
        for cell in selection:
            if not cell.column in cellsByColumn:
                cellsByColumn[cell.column] = [cell]
            else:
                cellsByColumn[cell.column].append(cell)

    return cellsByColumn



def chunkify_cell_arrangement(width, allCellsDict):
    """ Given a collection of selected cells verifies that we
    can place a symbol of given width. If so, return a
    list of consecutive chunks of cells all of a multiple of width
    that can be filled with the new symbol.
    
    """

    allCells = allCellsDict.values()

    # check 1: number of active cells has to be a multiple
    # of width
    if num_unitcells(allCells) % width != 0:
        return []

    cellsByRow = order_selection_by_rows(allCells)

    # check 2: each row has to be a multiple of width
    for row in cellsByRow.values():
        if num_unitcells(row) % width != 0:
            return []

    chunkList = chunk_all_rows(width, cellsByRow)

    return chunkList



def chunk_all_rows(width, cellsByRow):
    """ Separate each row into chunks at least as long as
    the items we want to place. Then we check if each
    chunk is consecutive.
    
    """
    
    chunkList = []
    for row in cellsByRow.values():
        row.sort(lambda x, y: cmp(x.column, y.column))

        chunks = []
        chunk = []
        length = 0
        for entry in row:
            chunk.append(entry)
            length += entry.width
            if length % width == 0:
               chunks.append(chunk)
               chunk = []
               length = 0

        if not are_consecutive(chunks):
            return []

        chunkList.extend(chunks)

    return chunkList



def are_consecutive(chunks):
    """ Checks if each chunk in a list of chunks consists
    of consecutive items.
    
    """

    if not chunks:
        return True

    for chunk in chunks:
        if not chunk:
            return False

        consecutiveCol = chunk[0].column + chunk[0].width
        for cell in chunk[1:]:
            if cell.column != consecutiveCol:
                return False
            
            consecutiveCol = cell.column + cell.width
            
    return True
        


def num_unitcells(cells):
    """ Compute the total number of unit cells in the
    selection.
    
    """

    totalWidth = 0
    for item in cells:
        totalWidth += item.width

    return totalWidth



def get_item_id(column, row):
    """ Returns an items id based on its row and column location. """

    return str(column) + ":" + str(row)



def get_edge_id(gridPoint1, gridPoint2):
    """ Given the column and row values of two grid points
    return an string ID.

    NOTE: Each cell has 4 grid points. The upper left hand
    corner corresponds to the row/column id of the cell.
    The other corners each add +1 for row and/or column id.
    E.g. lower right hand corner has an id of (row + 1, column + 1).

    """

    return ":".join(map(str, gridPoint1 + gridPoint2))



############################################################################
#
# Helper Classes
#
############################################################################
class PatternCanvasEntry(object):
    """ This is a small helper class for storing all relevant information
    to track a PatternGridItem. 

    """

    def __init__(self,  column, row, width, color, symbol):

        self.column = column
        self.row    = row
        self.width  = width
        self.color  = color
        self.symbol = symbol



