/***************************************************************
*
* (c) 2009-2010 Markus Dittrich 
*
* This program is free software; you can redistribute it 
* and/or modify it under the terms of the GNU General Public 
* License Version 3 as published by the Free Software Foundation. 
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License Version 3 for more details.
*
* You should have received a copy of the GNU General Public 
* License along with this program; if not, write to the Free 
* Software Foundation, Inc., 59 Temple Place - Suite 330, 
* Boston, MA 02111-1307, USA.
*
****************************************************************/

/** Qt headers */
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

/** local headers */
#include "basicDefs.h"
#include "colorSelectorItem.h"
#include "colorSelectorWidget.h"


QT_BEGIN_NAMESPACE

/* convenience definitions */
const QColor DEFAULT_COLOR = Qt::white;
const int NUM_TOP_COLORSELECTORS = 5;
const int NUM_BOTTOM_COLORSELECTORS = 5;


/**************************************************************
 *
 * PUBLIC FUNCTIONS 
 *
 **************************************************************/

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
ColorSelectorWidget::ColorSelectorWidget(const QList<QColor>& colors,
                                         QWidget* myParent)
  :
    QWidget(myParent),
    selectorColors_(colors)
{
  status_ = SUCCESSFULLY_CONSTRUCTED;
}


//--------------------------------------------------------------
// main initialization routine
//--------------------------------------------------------------
bool ColorSelectorWidget::Init()
{
  if ( status_ != SUCCESSFULLY_CONSTRUCTED )
  {
    return false;
  }

  /* call individual initialization routines */
  pad_colors_();
  create_layout_();

  return true;
}


/**************************************************************
 *
 * PUBLIC FUNCTIONS
 *
 *************************************************************/

//-------------------------------------------------------------
// return a list of all currently selected colors
//-------------------------------------------------------------
QList<QColor> ColorSelectorWidget::get_colors() const
{
  QList<QColor> allColors;
  foreach(ColorSelectorItem* item, colorSelectors_)
  {
    allColors.push_back(item->get_color());
  }

  return allColors;
}


//-------------------------------------------------------------
// set the list of selector colors to the given ones. If the
// number is less than the available number of selectors the
// latter are padded with the default color. If there are more,
// the superfluous colors are ignored.
//-------------------------------------------------------------
void ColorSelectorWidget::set_colors(
  const QList<QColor>& newColors)
{
  selectorColors_ = newColors;
  pad_colors_();

  assert(selectorColors_.length() == colorSelectors_.length());

  int count = 0;
  foreach(ColorSelectorItem* item, colorSelectors_)
  {
    item->set_color(selectorColors_[count]);
    ++count;
  }
}
  


/**************************************************************
 *
 * PUBLIC SLOTS
 *
 *************************************************************/

//-------------------------------------------------------------
// switch the currently selected color selected to the newly
// selected one
//-------------------------------------------------------------
void ColorSelectorWidget::highlight_color_button(
  ColorSelectorItem* newActiveItem)
{
  activeSelector_->unselect();
  activeSelector_ = newActiveItem;
  activeSelector_->select();
}


/*************************************************************
 *
 * PRIVATE MEMBER FUNCTIONS
 *
 *************************************************************/

//----------------------------------------------------------------
// main interface creation function
// We place all colors that we got passed in selectorColors_.
// If we have less than the 10 slots available we pad with
// DEFAULT_COLOR. Slot number 1 is always the default selected
// color
//----------------------------------------------------------------
void ColorSelectorWidget::create_layout_()
{
  /* set up the top selector layout */
  QHBoxLayout* topSelectorLayout = new QHBoxLayout;
  for (int topCount=0; topCount < NUM_TOP_COLORSELECTORS; ++topCount)
  {
    ColorSelectorItem* newItem =
      new ColorSelectorItem(selectorColors_[topCount],this);
    newItem ->Init();
    topSelectorLayout->addWidget(newItem);
    colorSelectors_.push_back(newItem);

    /* activate first color as default */
    if ( topCount == 0 )
    {
      newItem->select();
      activeSelector_ = newItem;
    }
  }
    
  
  /* set up the bottom selector layout */
  QHBoxLayout* bottomSelectorLayout = new QHBoxLayout;
  for (int bottomCount=NUM_TOP_COLORSELECTORS;
       bottomCount < (NUM_TOP_COLORSELECTORS + NUM_BOTTOM_COLORSELECTORS);
       ++bottomCount)
  {
    ColorSelectorItem* newItem =
      new ColorSelectorItem(selectorColors_[bottomCount],this);
    newItem ->Init();
    bottomSelectorLayout->addWidget(newItem);
    colorSelectors_.push_back(newItem);
  }
  
  QVBoxLayout* selectorLayout = new QVBoxLayout;
  selectorLayout->addLayout(topSelectorLayout);
  selectorLayout->addLayout(bottomSelectorLayout);

  setLayout(selectorLayout);
}


//-----------------------------------------------------------------------
// pad list of selector colors so we have
// NUM_TOP_COLORSELECTORS + NUM_BOTTOM_COLORSELECTORS
//-----------------------------------------------------------------------
void ColorSelectorWidget::pad_colors_()
{
 /* pad list to 10 if necessary */
  int neededColors = NUM_TOP_COLORSELECTORS + NUM_BOTTOM_COLORSELECTORS
    - selectorColors_.length();
  for (int count = 0; count < neededColors; ++count)
  {
    selectorColors_.push_back(DEFAULT_COLOR);
  }
}

QT_END_NAMESPACE
