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

/** Qt headers */
#include <QDebug>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QVBoxLayout>


/** local headers */
#include "basicDefs.h"
#include "patternKeyCanvas.h"
#include "patternKeyDialog.h"


QT_BEGIN_NAMESPACE

/**************************************************************
 *
 * PUBLIC FUNCTIONS 
 *
 **************************************************************/

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
PatternKeyDialog::PatternKeyDialog(int cellSize,
  const QSettings& aSetting, QWidget* myParent)
    :
      QDialog(myParent),
      cellSize_(cellSize),
      settings_(aSetting),
      mainSplitter_(new QSplitter)
{
  status_ = SUCCESSFULLY_CONSTRUCTED;
}


//--------------------------------------------------------------
// main initialization routine
//--------------------------------------------------------------
bool PatternKeyDialog::Init()
{
  if ( status_ != SUCCESSFULLY_CONSTRUCTED )
  {
    return false;
  }

  /* call individual initialization routines */
  setModal(false);
  setWindowTitle(tr("Edit pattern key"));

  /* create interface */
  create_canvas_();
  create_buttons_();

  /* generate main layout */
  mainSplitter_->addWidget(patternKeyView_);
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(mainSplitter_);
  mainLayout->addLayout(buttonLayout_);
  setLayout(mainLayout);


  /* some plumbing */
  connect(this,
          SIGNAL(settings_changed()),
          patternKeyCanvas_,
          SLOT(update_after_settings_change())
         );


  return true;
}


/**************************************************************
 *
 * PUBLIC SLOTS
 *
 *************************************************************/


//-------------------------------------------------------------
// every time a symbol is added on the canvas this slot is
// invoked to reference count the current number of symbols
// present
//-------------------------------------------------------------
void PatternKeyDialog::add_knitting_symbol(
  KnittingSymbolPtr newSymbol)
{
  /* update reference count */
  QString symbolName = newSymbol->fullName();
  int currentValue = usedKnittingSymbols_[symbolName] + 1;
  assert(currentValue > 0);
  
  usedKnittingSymbols_[symbolName] = currentValue;

  /* if the currentValue is one the symbol has come "into
   * existence" and we need to show it on the legend canvas;
   * if this is the very first time the user selected it
   * we also add the default descriptor text to the 
   * symbolDescriptor_ map */
  if (currentValue == 1)
  {
    if (!symbolDescriptors_.contains(symbolName))
    {
      QString description = newSymbol->baseName();
      symbolDescriptors_[symbolName] = description;
      qDebug() << symbolDescriptors_[symbolName];
    }

    /* show it on the canvas */
    patternKeyCanvas_->add_symbol(newSymbol);
  }

}


//-------------------------------------------------------------
// every time a symbol is removed from the canvas this slot is
// invoked to reference count the current number of symbols
// present
//-------------------------------------------------------------
void PatternKeyDialog::remove_knitting_symbol(
  KnittingSymbolPtr deadSymbol)
{
  /* update reference count */
  QString symbolName = deadSymbol->fullName();
  int currentValue = usedKnittingSymbols_[symbolName] - 1;
  usedKnittingSymbols_[symbolName] = currentValue;

  assert(currentValue >= 0);
}



/**************************************************************
 *
 * PUBLIC MEMBER FUNCTIONS
 *
 *************************************************************/

/**************************************************************
 *
 * PROTECTED MEMBER FUNCTIONS 
 *
 *************************************************************/

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

//------------------------------------------------------------
// create the pattern key canvas and the associated 
// QGraphicsView
//------------------------------------------------------------
void PatternKeyDialog::create_canvas_()
{
  QPoint origin(0,0);
  patternKeyCanvas_ = 
    new PatternKeyCanvas(origin, cellSize_, settings_, this);  
  patternKeyCanvas_->Init();
  patternKeyView_ = new QGraphicsView(patternKeyCanvas_);
  patternKeyView_->setRenderHints(QPainter::Antialiasing);
  patternKeyView_->setViewportUpdateMode(
      QGraphicsView::FullViewportUpdate);
}


//-------------------------------------------------------------
// create and wire up the dialog buttons
//-------------------------------------------------------------
void PatternKeyDialog::create_buttons_()
{
  buttonLayout_ = new QHBoxLayout;

  QPushButton* closeButton = new QPushButton(tr("close"));
  QPushButton* exportToCanvasButton = 
    new QPushButton(tr("export to canvas"));

  buttonLayout_->addWidget(exportToCanvasButton);
  buttonLayout_->addStretch(1);
  buttonLayout_->addWidget(closeButton);

  connect(closeButton,
          SIGNAL(clicked()),
          this,
          SLOT(hide())
         );
}



QT_END_NAMESPACE