/***************************************************************
*
* (c) 2009 Markus Dittrich 
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

/* C++ headers */
#include <float.h>


/** Qt headers */
#include <QColor>
#include <QDebug>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QPainter>



/** local headers */
#include "basicDefs.h"
#include "graphicsScene.h"
#include "patternGridItem.h"


/**************************************************************
 *
 * PUBLIC FUNCTIONS 
 *
 **************************************************************/

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
PatternGridItem::PatternGridItem(const QPoint& aLoc, 
  const QSize& aDim, const int aScale, GraphicsScene* myParent)
    :
      selected_(false),
      parent_(myParent),
      svgItem_(0),
      loc_(aLoc),
      dim_(aDim),
      scaling_(aScale)
{
  status_ = SUCCESSFULLY_CONSTRUCTED;
}


//--------------------------------------------------------------
// main initialization routine
//--------------------------------------------------------------
bool PatternGridItem::Init()
{
  if ( status_ != SUCCESSFULLY_CONSTRUCTED )
  {
    return false;
  }

  /* call individual initialization routines */
  set_up_pens_brushes_();

  /* some signals and slots */
  connect(this, 
          SIGNAL(item_selected(PatternGridItem*, bool)), 
          parent_, 
          SLOT(grid_item_selected(PatternGridItem*, bool)));

  connect(this, 
          SIGNAL(item_reset(PatternGridItem*)), 
          parent_, 
          SLOT(grid_item_reset(PatternGridItem*)));


  return true;
}



/**************************************************************
 *
 * PUBLIC SLOTS
 *
 *************************************************************/



/**************************************************************
 *
 * PUBLIC MEMBER FUNCTIONS
 *
 *************************************************************/

//------------------------------------------------------------
// overload pure virtual base class function returning our
// dimensions
//------------------------------------------------------------
QRectF PatternGridItem::boundingRect() const
{
  return QRectF(loc_, scaling_*dim_);
}
  
  
//------------------------------------------------------------
// overload pure virtual base class function painting 
// ourselves
//------------------------------------------------------------
void PatternGridItem::paint(QPainter *painter, 
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setPen(pen_);
  painter->setBrush(*currentBrush_);
  painter->drawRect(QRectF(loc_, scaling_*dim_));
}


//-------------------------------------------------------------
// insert a new knitting symbol
//-------------------------------------------------------------
void PatternGridItem::insert_knitting_symbol(KnittingSymbolPtr aSymbol)
{
  /* update pointers */
  knittingSymbol_ = aSymbol;
  QString symbolPath(aSymbol->path());

  /* deselect us */
  unselect_();

  /* delete the previous svgItem if there was one */
  if ( svgItem_ != 0 ) 
   {
    delete svgItem_;
    svgItem_ = 0;
  }

  if (symbolPath != "")
  {
    svgItem_ = new QGraphicsSvgItem(symbolPath,this);
    fit_svg_();
    svgItem_->setVisible(true);
  }
}
  


/**************************************************************
 *
 * PROTECTED MEMBER FUNCTIONS 
 *
 *************************************************************/

//-------------------------------------------------------------
// handle mouse press events 
//-------------------------------------------------------------
void PatternGridItem::mousePressEvent(
  QGraphicsSceneMouseEvent* anEvent)
{
  /* if the user has shift pressed this is a reset event
   * otherwise a select/deselect event */
  if (parent_->shift_pressed())
  {
    emit item_reset(this);
  }
  else
  {
    if (selected_)
    {
      unselect_();
      emit item_selected(this, false);
    }
    else
    {
      select_();
      emit item_selected(this, true);
    }
  }

  /* repaint */
  update();
}


/**************************************************************
 *
 * PRIVATE SLOTS
 *
 *************************************************************/

/*************************************************************
 *
 * PRIVATE MEMBER FUNCTIONS
 *
 *************************************************************/

//-------------------------------------------------------------
// set up all the pens we use for drawing
//-------------------------------------------------------------
void PatternGridItem::set_up_pens_brushes_()
{
  /* pen used */
  pen_.setWidth(0.5);
  pen_.setJoinStyle(Qt::MiterJoin);
  pen_.setColor(Qt::black);

  /* brushes for active/inactive item */
  activeBrush_ = QBrush(Qt::gray);
  inactiveBrush_ = QBrush(Qt::white);
  currentBrush_ = &inactiveBrush_;
}


//---------------------------------------------------------------
// scale and shift svg item so it fits into our bounding 
// box
//---------------------------------------------------------------
void PatternGridItem::fit_svg_()
{
  if (svgItem_ == 0)
  {
    return;
  }

  /* get bounding boxes */
  QRectF svgRect = svgItem_->sceneBoundingRect();
  QRectF boxRect = sceneBoundingRect();

  /* scale */
  double scaleX = 1.0;
  if ( svgRect.width() > DBL_EPSILON )
  {
    scaleX = (boxRect.width()-pen_.width())/svgRect.width();
  }

  double scaleY = 1.0;
  if ( svgRect.height() > DBL_EPSILON )
  {
    scaleY = (boxRect.height()-pen_.width())/svgRect.height();
  }

  svgItem_->scale(scaleX, scaleY);

  /* translate */
  svgItem_->moveBy(boxRect.x(), boxRect.y());
}


//-----------------------------------------------------------------
// helper function for doing the internal housekeeping during
// selecting/unselecting
// NOTE: these functions should not emit an item_selected signal
//-----------------------------------------------------------------
void PatternGridItem::select_()
{
  selected_ = true;
  currentBrush_ = &activeBrush_;
}


void PatternGridItem::unselect_()
{
  selected_ = false;
  currentBrush_ = &inactiveBrush_;
}

