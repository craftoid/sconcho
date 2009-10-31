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

#ifndef PATTERN_KEY_CANVAS_H
#define PATTERN_KEY_CANVAS_H

/* boost includes */
#include <boost/utility.hpp>

/* QT includes */
#include <QGraphicsScene>
#include <QList>
#include <QPoint>

/* local includes */
#include "basicDefs.h"
#include "knittingSymbol.h"


QT_BEGIN_NAMESPACE


/* a few forward declarations */
class KnittingPatternItem;
class QGraphicsTextItem;
class QSettings;


namespace KeyCanvas
{
  struct LabelItem
  {
    KnittingPatternItem* pattern;
    QGraphicsTextItem* description;
  };
};


/***************************************************************
 * 
 * The PatternKeyCanvas allows manipulation of the 
 * QGraphicsItems belonging to the pattern key
 *
 ***************************************************************/
class PatternKeyCanvas
  :
    public QGraphicsScene,
    public boost::noncopyable
{
  
  Q_OBJECT

  
public:


  explicit PatternKeyCanvas(QPoint origin, int aSize, 
    const QSettings& settings, QObject* myParent = 0);
  bool Init();

  /* add or remove symbol from legend canvas */
  void add_symbol(KnittingSymbolPtr newSymbol, const QString& desc);


  /* setters for properties */
  //void new_settings(const QSettings& newSettings); 

//protected:

//  void mousePressEvent(QGraphicsSceneMouseEvent* event);

public slots:

  void update_after_settings_change();

    
private:

  /* some tracking variables */
  int status_;

  /* origin of display */
  QPoint origin_;

  /* list of currently displayed KnittingPatternItems in
   * the order they are displayed on the screen so we can
   * easily figure out where to insert a new item */
  QList<KeyCanvas::LabelItem> displayedItems_;

  /* size of a cell for displaying knitting patterns present
   * on canvas and number of rows currently shown */
  int cellSize_;
  int cellMargin_;

  /* our graphic items */
  QGraphicsTextItem* mainText_;

  /* properties objects */
  const QSettings& settings_;

  /* interface creation functions */
  void create_main_label_();

  /* helper functions */
  int get_text_x_position_(const KnittingPatternItem* anItem) const;
};


QT_END_NAMESPACE

#endif
