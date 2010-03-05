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
#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

/** local headers */
#include "aboutSconcho.h"
#include "basicDefs.h"
#include "colorSelectorWidget.h"
#include "graphicsScene.h"
#include "gridDimensionDialog.h"
#include "helperFunctions.h"
#include "io.h"
#include "mainWindow.h"
#include "patternView.h"
#include "preferencesDialog.h"
#include "settings.h"
#include "symbolSelectorWidget.h"


QT_BEGIN_NAMESPACE


/**************************************************************
 *
 * PUBLIC FUNCTIONS 
 *
 **************************************************************/

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
MainWindow::MainWindow() 
  :
    mainSplitter_(new QSplitter),
    saveFilePath_(""),
    settings_("sconcho","settings")
{
  status_ = SUCCESSFULLY_CONSTRUCTED;
}


//--------------------------------------------------------------
// main initialization routine
//--------------------------------------------------------------
bool MainWindow::Init()
{
  if ( status_ != SUCCESSFULLY_CONSTRUCTED )
  {
    return false;
  }

  setWindowTitle(tr("sconcho"));
  setMinimumSize(initialSize);

  initialize_settings(settings_);

  /* populate the main interface 
   * NOTE: We NEED to first create the patterKeyDialog and
   * symbolsWidget before we add the graphicsScence */
  QList<ParsedSymbol> rawSymbols = load_all_symbols();
  initialize_symbols_(rawSymbols);
  create_symbols_widget_(rawSymbols);
  create_graphics_scene_();
  create_toolbar_();
  create_color_widget_();
  create_menu_bar_();
  create_status_bar_();
  create_property_symbol_layout_();
  create_timers_();

  connect(symbolSelector_,
          SIGNAL(selected_symbol_changed(const KnittingSymbolPtr)),
          canvas_,
          SLOT(update_selected_symbol(const KnittingSymbolPtr)),
          Qt::DirectConnection
         );
  
  connect(this,
          SIGNAL(settings_changed()),
          canvas_,
          SLOT(update_after_settings_change())
         );

  connect(canvas_,
          SIGNAL(show_whole_scene()),
          canvasView_,
          SLOT(accessible_in_view())
         ); 

  /* set up main interface and set initial splitter sizes */
  mainSplitter_->addWidget(canvasView_);
  mainSplitter_->addWidget(propertyBox_);
  QList<int> sizes;
  sizes << 450 << 250;
  mainSplitter_->setSizes(sizes);
  setCentralWidget(mainSplitter_);
  return true;
}


/**************************************************************
 *
 * PUBLIC SLOTS
 *
 *************************************************************/

//-------------------------------------------------------------
// update the display widget with the current mouse position
//-------------------------------------------------------------
void MainWindow::update_mouse_position_display(QPointF newPosition)
{
  QString xPos;
  xPos.setNum(newPosition.x());
  QString yPos;
  yPos.setNum(newPosition.y());
  
  QString newLabel = "X : " + xPos + "   " + "Y : " + yPos;
  currentMousePosWidget_->setText(newLabel);
}


//-------------------------------------------------------------
// show messages in status bar 
//-------------------------------------------------------------
void MainWindow::show_statusBar_message(QString aMessage)
{
  QPalette aPalette = statusBarMessages_->palette();
  aPalette.setBrush(QPalette::WindowText, Qt::black);
  statusBarMessages_->setPalette(aPalette);
  statusBarMessages_->setText(aMessage);
}


//-------------------------------------------------------------
// show errors in status bar 
//-------------------------------------------------------------
void MainWindow::show_statusBar_error(QString aMessage)
{
  QPalette aPalette = statusBarMessages_->palette();
  aPalette.setBrush(QPalette::WindowText, Qt::red);
  statusBarMessages_->setPalette(aPalette);
  statusBarMessages_->setText(aMessage);
}


//--------------------------------------------------------------
// clear the status bar
//--------------------------------------------------------------
void MainWindow::clear_statusBar()
{
  QPalette aPalette = statusBarMessages_->palette();
  aPalette.setBrush(QPalette::WindowText, Qt::black);
  statusBarMessages_->setPalette(aPalette);
  statusBarMessages_->setText(tr("Nice chart!"));
}


//-------------------------------------------------------------
// set the path  the current project file
//-------------------------------------------------------------
void MainWindow::set_project_file_path(const QString& newName)
{ 
  saveFilePath_ = newName;
  QFileInfo info(newName);
  setWindowTitle(info.fileName());
}



/**************************************************************
 *
 * PRIVATE SLOTS
 *
 *************************************************************/

//-------------------------------------------------------------
// SLOT: show file open menu
//-------------------------------------------------------------
void MainWindow::show_file_open_dialog_()
{
  QString currentDirectory = QDir::currentPath();
  QString openFileName = QFileDialog::getOpenFileName(this,
    tr("open data file"), currentDirectory,
    tr("sconcho pattern files (*.spf)"));

  if ( openFileName.isEmpty() )
  {
    return;
  }

  /* extract file extension and make sure it corresponds to
   * a supported format */
  QFileInfo openFileInfo(openFileName);
  QString extension = openFileInfo.completeSuffix();

  if (extension != "spf")
  {
    QMessageBox::warning(this, tr("Warning"),
      tr("Can not open file with format ") + extension,
      QMessageBox::Ok);
    return;
  }

  /* update path to project file */
  set_project_file_path(openFileName);

  load_project_(openFileName);
}

  
//-------------------------------------------------------------
// SLOT: show file save menu
//-------------------------------------------------------------
void MainWindow::show_file_save_dialog_()
{
  QFileInfo currentFileInfo(saveFilePath_);
  QString saveFileName = QFileDialog::getSaveFileName(this,
    tr("Save Pattern"), currentFileInfo.baseName(),
    tr("sconcho pattern files (*.spf)"));

  if ( saveFileName.isEmpty() )
  {
    return;
  }

  /* extract file extension and make sure it corresponds to
   * a supported format */
  QFileInfo saveFileInfo(saveFileName);
  QString extension = saveFileInfo.completeSuffix();

  if (extension != "spf")
  {
    QMessageBox::warning(this, tr("Warning"),
      tr("Unknown file format ") + extension,
      QMessageBox::Ok);
    return;
  }

  /* update project filename */
  set_project_file_path(saveFileName);

  save_project_(saveFileName);
}


//-------------------------------------------------------------
// SLOT: show file export menu and then export the legend
// only.
//
// NOTE: Here we have to do some tricks since the Legend
// items can in principle be all over the canvas. First of
// all, we only go ahead if the legend is currently
// visible. Then we hide all but the legend items, export
// and turn everything back on.
//-------------------------------------------------------------
void MainWindow::export_legend_dialog_()
{
  if (!canvas_->legend_is_visible())
  {
    QMessageBox warn(QMessageBox::Critical, tr("Error"),
      "Legend is currently not visible. Please turn legend "
      "on before trying to export it.", QMessageBox::Ok);
    warn.exec();
    return;
  }
  
  QString exportFilename = show_file_export_dialog(saveFilePath_);
  if (exportFilename.isEmpty())
  {
    return;
  }

  canvas_->hide_all_but_legend();
  export_scene(exportFilename, canvas_);
  canvas_->show_all_items();
}


//-------------------------------------------------------------
// SLOT: show file export menu and then export the actual
// scene
//-------------------------------------------------------------
void MainWindow::export_canvas_dialog_()
{
  QString exportFilename = show_file_export_dialog(saveFilePath_);
  if (exportFilename.isEmpty())
  {
    return;
  }
  
  export_scene(exportFilename, canvas_);
}


//------------------------------------------------------------
// SLOT: show the print menu
//------------------------------------------------------------
void MainWindow::show_print_dialog_()
{
  print_scene(canvas_);
}


//-------------------------------------------------------------
// SLOT: close the application
//-------------------------------------------------------------
void MainWindow::quit_sconcho_()
{
  QMessageBox::StandardButton answer = QMessageBox::warning(this,
      tr("Warning"), tr("Are you sure you want to quit?"),
      QMessageBox::Yes | QMessageBox::No);

  if ( answer == QMessageBox::Yes )
  {
    exit(0);
  }
}


//------------------------------------------------------------
// SLOT: this slot handles user requests to reset the pattern grid
// and create a new one of a certain dimension
//------------------------------------------------------------
void MainWindow::reset_grid_()
{
  /* first off, let's warn the user that she is about to
   * loose all her work */
  QMessageBox warn(QMessageBox::Critical, tr("Warning"),
      "Resetting the grid will cause the loss of "
      "everything that is in the current pattern "
      "grid. Is this ok?",
      QMessageBox::Ok | QMessageBox::Cancel);

  int status = warn.exec();
  
  if (status != QMessageBox::Ok)
  {
    return;
  }

  /* ask for new grid dimensions and reset canvas */
  QSize newDimensions = show_grid_dimension_dialog_();
  canvas_->reset_grid(newDimensions);
  canvasView_->visible_in_view();
}


//-------------------------------------------------------------
// SLOT responsible for displaying a the default Qt info
// widget
//-------------------------------------------------------------
void MainWindow::show_about_qt_dialog_()
{
  QMessageBox::aboutQt(this,tr("About Qt"));
}


//------------------------------------------------------------- 
// SLOT responsible for displaying our very own personalized
// sconcho info and copyright notice
//------------------------------------------------------------- 
void MainWindow::show_sconcho_dialog_() 
{ 
  AboutSconchoWidget about; 
  about.exec(); 
} 


//------------------------------------------------------------
// SLOT: fire up the preferences dialog
//------------------------------------------------------------
void MainWindow::show_preferences_dialog_()
{
  PreferencesDialog prefDialog(settings_);
  prefDialog.Init();

  /* update all child widgets with new preferences */
  emit settings_changed();
}


//------------------------------------------------------------
// SLOT: If the user has already defined a fileName just
// save it, other wise open a save_file dialog
//------------------------------------------------------------
void MainWindow::save_file_()
{
  if (saveFilePath_.isEmpty())
  {
    show_file_save_dialog_();
  }
  else
  {
    save_project_(saveFilePath_);
  }
}


/*************************************************************
 *
 * PRIVATE MEMBER FUNCTIONS
 *
 *************************************************************/

//------------------------------------------------------------
// create the menu bar
//------------------------------------------------------------
void MainWindow::create_menu_bar_()
{
  menuBar_ = new QMenuBar(this);
  setMenuBar(menuBar_);

  /* create the individual menu options */
  create_file_menu_();
  create_edit_menu_();
  create_view_menu_();
  create_help_menu_();
} 


//------------------------------------------------------------
// create the file menu 
//------------------------------------------------------------
void MainWindow::create_file_menu_()
{
  QMenu* fileMenu = menuBar_->addMenu(tr("&File"));

  /* new */
  QAction* newAction =
    new QAction(QIcon(":/icons/fileopen.png"),tr("&New"), this);
  fileMenu->addAction(newAction);
  newAction->setShortcut(tr("Ctrl+N"));
  connect(newAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(reset_grid_()));

  fileMenu->addSeparator();


  /* open */
  QAction* openAction =
    new QAction(QIcon(":/icons/fileopen.png"),tr("&Open"), this);
  fileMenu->addAction(openAction);
  openAction->setShortcut(tr("Ctrl+O"));
  connect(openAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(show_file_open_dialog_()));

  fileMenu->addSeparator();

  /* save */
  QAction* saveAction =
    new QAction(QIcon(":/icons/filesave.png"),tr("&Save"), this);
  fileMenu->addAction(saveAction);
  saveAction->setShortcut(tr("Ctrl+S"));
  connect(saveAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(save_file_()));


  /* save as */
  QAction* saveAsAction =
    new QAction(QIcon(":/icons/filesave.png"),tr("Save as"), this);
  fileMenu->addAction(saveAsAction);
  connect(saveAsAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(show_file_save_dialog_()));

  /* export */
  QAction* exportAction =
    new QAction(QIcon(":/icons/fileexport.png"),tr("&Export"), this);
  fileMenu->addAction(exportAction);
  exportAction->setShortcut(tr("Ctrl+E"));
  connect(exportAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(export_canvas_dialog_()));


  /* export legend only */
  QAction* legendExportAction =
    new QAction(QIcon(":/icons/fileexport.png"),
                tr("&Export Legend Only"), this);
  fileMenu->addAction(legendExportAction);
  legendExportAction->setShortcut(tr("Ctrl+L"));
  connect(legendExportAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(export_legend_dialog_()));

  /* print */
  QAction* printAction =
    new QAction(QIcon(":/icons/fileprint.png"),tr("&Print"), this);
  fileMenu->addAction(printAction);
  printAction->setShortcut(tr("Ctrl+P"));
  connect(printAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(show_print_dialog_()));

  fileMenu->addSeparator();

  /* exit */
  QAction* exitAction =
    new QAction(QIcon(":/icons/exit.png"),tr("E&xit"), this);
  fileMenu->addAction(exitAction);
  exitAction->setShortcut(tr("Ctrl+X"));
  connect(exitAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(quit_sconcho_()));
} 



//------------------------------------------------------------
// create the edit menu for general pattern grid controls 
//------------------------------------------------------------
void MainWindow::create_edit_menu_()
{
  QMenu* editMenu = menuBar_->addMenu(tr("&Edit"));

  /* preferences */
  QAction* preferencesAction = 
    new QAction(tr("&Preferences"), this);
  editMenu->addAction(preferencesAction);
  connect(preferencesAction, 
          SIGNAL(triggered()), 
          this,
          SLOT(show_preferences_dialog_()));
} 


//------------------------------------------------------------
// create the view menu for general view control of canvas
//------------------------------------------------------------
void MainWindow::create_view_menu_()
{
  QMenu* viewMenu = menuBar_->addMenu(tr("&View"));

  /* zoom in */
  QAction* zoomInAction =
    new QAction(QIcon(":/icons/viewmag+.png"),tr("&Zoom in"), this);
  viewMenu->addAction(zoomInAction);
  zoomInAction->setShortcut(tr("Ctrl++"));
  connect(zoomInAction, 
          SIGNAL(triggered()), 
          canvasView_,
          SLOT(zoom_in()));

  /* zoom out */
  QAction* zoomOutAction =
    new QAction(QIcon(":/icons/viewmag-.png"),tr("&Zoom out"), this);
  viewMenu->addAction(zoomOutAction);
  zoomOutAction->setShortcut(tr("Ctrl+-"));
  connect(zoomOutAction, 
          SIGNAL(triggered()), 
          canvasView_,
          SLOT(zoom_out()));

  /* zoom in */
  QAction* fitAction =
    new QAction(QIcon(":/icons/viewmagfit.png"),tr("&Fit view"), this);
  viewMenu->addAction(fitAction);
  fitAction->setShortcut(tr("Ctrl+0"));
  connect(fitAction,
          SIGNAL(triggered()),
          canvasView_,
          SLOT(visible_in_view()));
}


//------------------------------------------------------------
// create the grid menu for general pattern grid controls 
//------------------------------------------------------------
void MainWindow::create_help_menu_()
{
  QMenu* helpMenu = menuBar_->addMenu(tr("&Help"));

  /* add entries */
  QAction* aboutAction = new QAction(tr("&About"),this);
  helpMenu->addAction(aboutAction);
  connect(aboutAction, SIGNAL(triggered()), this,
      SLOT(show_sconcho_dialog_()));

  QAction* aboutQtAction = new QAction(tr("About Qt"),this);
  helpMenu->addAction(aboutQtAction);
  connect(aboutQtAction, SIGNAL(triggered()), this,
      SLOT(show_about_qt_dialog_()));
} 



//------------------------------------------------------------
// create the status bar
//------------------------------------------------------------
void MainWindow::create_status_bar_()
{
  statusBar_ = new QStatusBar(this);
  statusBar_->setSizeGripEnabled(false);

  /* add welcome message widget */
  QString message = "Welcome to " + IDENTIFIER;
  statusBarMessages_ = new QLabel(message);
  statusBarMessages_->setMinimumWidth(200);
  statusBar_->addWidget(statusBarMessages_,1);

  /* add widget to display the mouse position */
  currentMousePosWidget_ = new QLabel;
  currentMousePosWidget_->setMinimumWidth(200);
  currentMousePosWidget_->setAlignment(Qt::AlignCenter);
  statusBar_->addPermanentWidget(currentMousePosWidget_,0);
  

  /* add to main window */
  setStatusBar(statusBar_);
}


//-------------------------------------------------------------
// create the main GraphicsScene widget
//-------------------------------------------------------------
void MainWindow::create_graphics_scene_()
{
  /* ask user for the grid size */
  QSize gridSize = show_grid_dimension_dialog_();

  /* get default symbol from symbol selector widget */
  KnittingSymbolPtr defaultSymbol = symbolSelector_->selected_symbol();

  /* create canvas */
  QPoint origin(0,0);
  canvas_ = new GraphicsScene(origin, gridSize, GRID_CELL_SIZE, 
      settings_, defaultSymbol, this);
  if ( !canvas_->Init() )
  {
    qDebug() << "Failed to initialize canvas";
  }

  canvasView_ = new PatternView(canvas_);
  canvasView_->Init();
}


//------------------------------------------------------------
// take the rawSymbols from the symbol parser and use them
// to initialize the list of stored KnittingPointer symbols
//------------------------------------------------------------
void MainWindow::initialize_symbols_(
  const QList<ParsedSymbol>& rawSymbols)
{
  foreach(ParsedSymbol sym, rawSymbols)
  {
    allSymbols_.push_back(sym.first);
  }
}


//-------------------------------------------------------------
// create the widget showing the available symbols 
//-------------------------------------------------------------
void MainWindow::create_symbols_widget_(
  const QList<QPair<KnittingSymbolPtr,int> >& allSymbols)
{
  symbolSelector_ = new SymbolSelectorWidget(allSymbols,
    GRID_CELL_SIZE, this);
  symbolSelector_->Init();
}


//-------------------------------------------------------------
// create toolbar
//-------------------------------------------------------------
void MainWindow::create_toolbar_()
{
  QToolBar* toolBar = new QToolBar(this);

  QToolButton* openButton = new QToolButton(this);
  openButton->setIcon(QIcon(":/icons/fileopen.png"));
  openButton->setToolTip(tr("open data file"));
  toolBar->addWidget(openButton);
  connect(openButton,SIGNAL(clicked()),this,
      SLOT(show_file_open_dialog_()));


  QToolButton* saveButton = new QToolButton(this);
  saveButton->setIcon(QIcon(":/icons/filesave.png"));
  saveButton->setToolTip(tr("save"));
  toolBar->addWidget(saveButton);
  connect(saveButton,
          SIGNAL(clicked()),
          this,
          SLOT(save_file_()));
 
  QToolButton* exportButton = new QToolButton(this);
  exportButton->setIcon(QIcon(":/icons/fileexport.png"));
  exportButton->setToolTip(tr("export canvas"));
  toolBar->addWidget(exportButton);
  connect(exportButton,
          SIGNAL(clicked()),
          this,
          SLOT(export_canvas_dialog_()));
 
  QToolButton* printButton = new QToolButton(this);
  printButton->setIcon(QIcon(":/icons/fileprint.png"));
  printButton->setToolTip(tr("print canvas"));
  toolBar->addWidget(printButton);
  connect(printButton,
          SIGNAL(clicked()),
          this,
          SLOT(show_print_dialog_()));

  toolBar->addSeparator();

  QToolButton* zoomInButton = new QToolButton(this);
  zoomInButton->setIcon(QIcon(":/icons/viewmag+.png"));
  zoomInButton->setToolTip(tr("zoom in"));
  toolBar->addWidget(zoomInButton);
  connect(zoomInButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(zoom_in()));
  
  QToolButton* zoomOutButton = new QToolButton(this);
  zoomOutButton->setIcon(QIcon(":/icons/viewmag-.png"));
  zoomOutButton->setToolTip(tr("zoom out"));
  toolBar->addWidget(zoomOutButton);
  connect(zoomOutButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(zoom_out()));

  QToolButton* resetButton = new QToolButton(this);
  resetButton->setIcon(QIcon(":/icons/viewmagfit.png"));
  resetButton->setToolTip(tr("fit view"));
  toolBar->addWidget(resetButton);
  connect(resetButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(visible_in_view()));

  toolBar->addSeparator();
 
  QToolButton* leftMoveButton = new QToolButton(this);
  leftMoveButton->setIcon(QIcon(":/icons/left.png"));
  leftMoveButton->setToolTip(tr("move canvas left"));
  toolBar->addWidget(leftMoveButton);
  connect(leftMoveButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(pan_left()));
  
  QToolButton* rightMoveButton = new QToolButton(this);
  rightMoveButton->setIcon(QIcon(":/icons/right.png"));
  rightMoveButton->setToolTip(tr("move canvas right"));
  toolBar->addWidget(rightMoveButton);
  connect(rightMoveButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(pan_right()));

  QToolButton* upMoveButton = new QToolButton(this);
  upMoveButton->setIcon(QIcon(":/icons/up.png"));
  upMoveButton->setToolTip(tr("move canvas up"));
  toolBar->addWidget(upMoveButton);
  connect(upMoveButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(pan_up()));
  
  QToolButton* downMoveButton = new QToolButton(this);
  downMoveButton->setIcon(QIcon(":/icons/down.png"));
  downMoveButton->setToolTip(tr("move canvas down"));
  toolBar->addWidget(downMoveButton);
  connect(downMoveButton,
          SIGNAL(clicked()),
          canvasView_,
          SLOT(pan_down()));
  
  toolBar->addSeparator();

  QToolButton* resetSelectionButton = new QToolButton(this);
  resetSelectionButton->setIcon(QIcon(":/icons/stop.png"));
  resetSelectionButton->setToolTip(tr("reset current selection"));
  toolBar->addWidget(resetSelectionButton);
  connect(resetSelectionButton,
          SIGNAL(clicked()),
          canvas_,
          SLOT(deselect_all_active_items()));

  QToolButton* markRectangleButton = new QToolButton(this);
  markRectangleButton->setIcon(QIcon(":/icons/rectangle.png"));
  markRectangleButton->setToolTip(tr("mark selection with rectangle"));
  toolBar->addWidget(markRectangleButton);
  connect(markRectangleButton,
          SIGNAL(clicked()),
          canvas_,
          SLOT(mark_active_cells_with_rectangle()));

  QToolButton* toggleKeyButton = new QToolButton(this);
  toggleKeyButton->setIcon(QIcon(":/icons/key.png"));
  toggleKeyButton->setToolTip(tr("toggle pattern key"));
  toolBar->addWidget(toggleKeyButton);
  connect(toggleKeyButton,
          SIGNAL(clicked()),
          canvas_,
          SLOT(toggle_legend_visibility()));

  addToolBar(toolBar);
}


//-------------------------------------------------------------
// create graph property widget (color selector, line width
// selector)
//-------------------------------------------------------------
void MainWindow::create_color_widget_()
{
  colorSelectorGrouper_ = new QGroupBox("cell color");
  QList<QColor> colors;
  colors << Qt::white << Qt::red << Qt::green << Qt::blue
         << Qt::cyan << Qt::yellow << Qt::gray << Qt::magenta
         << Qt::darkBlue << Qt::darkMagenta;

  colorSelectorWidget_ = new ColorSelectorWidget(colors,this);
  colorSelectorWidget_->Init();
  QHBoxLayout* colorLayout = new QHBoxLayout;
  colorLayout->addWidget(colorSelectorWidget_);
  colorSelectorGrouper_->setLayout(colorLayout);

  connect(colorSelectorWidget_,
          SIGNAL(color_changed(const QColor&)),
          canvas_,
          SLOT(update_selected_background_color(const QColor&)),
          Qt::DirectConnection
         );
}



//-------------------------------------------------------------
// create the main splitter
// 
// NOTE: all Widgets that belong to the splitter have to exist
//       at this point
//-------------------------------------------------------------
void MainWindow::create_property_symbol_layout_()
{
  /* properties layout */
  QVBoxLayout* propertiesLayout = new QVBoxLayout;
  propertiesLayout->addWidget(symbolSelector_);
  propertiesLayout->addWidget(colorSelectorGrouper_);
  propertyBox_ = new QGroupBox(this);
  propertyBox_->setLayout(propertiesLayout);
}


//-------------------------------------------------------------
// create all timers used during exectution
//-------------------------------------------------------------
void MainWindow::create_timers_()
{
  /* this timers clears the status bar every so ofter */
  QTimer* statusBarTimer = new QTimer(this);
  connect(statusBarTimer,
          SIGNAL(timeout()),
          this,
          SLOT(clear_statusBar()));
  statusBarTimer->start(5000);
}


//-------------------------------------------------------------
// open dialog to ask user for the dimensions of the pattern
// grid
//-------------------------------------------------------------
QSize MainWindow::show_grid_dimension_dialog_()
{
  GridDimensionDialog gridDialog; 
  gridDialog.Init();
  return gridDialog.dim();
}


//--------------------------------------------------------------
// save canvas to file
//-------------------------------------------------------------
void MainWindow::save_project_(const QString& fileName)
{
  QList<QColor> activeColors(colorSelectorWidget_->get_colors());
  CanvasIOWriter writer(canvas_, activeColors, fileName);

  /* we need to check if we can open the file for writing */
  if (!writer.Init())
  {
    QMessageBox::critical(0,"Save File",
      QString("Failed to open file\n%1\nfor saving.") 
      .arg(fileName));
    return;
  }
  else
  {
    writer.save();
  }
}


//-------------------------------------------------------------
// load a previously saved canvas from file
//-------------------------------------------------------------
void MainWindow::load_project_(const QString& fileName)
{
  CanvasIOReader reader(fileName, allSymbols_);

  /* we need to make sure that we are able to open the file 
   * for reading */
  if (!reader.Init())
  {
    QMessageBox::critical(0,"Read File",
      QString("Failed to open file\n%1\nfor reading.") 
      .arg(fileName));
    return;
  }
  else
  {
    if (!reader.read())
    {
      QMessageBox::critical(0,"Read File",
        QString("Failed to open file\n%1\nfor reading.") 
        .arg(fileName));
      return;
    }

    /* establish canvas */
    canvas_->load_new_canvas(reader.get_pattern_items());
    canvas_->place_legend_items(reader.get_legend_items());

    /* read custom colors and apply them */
    QList<QColor> foo(reader.get_project_colors());
    colorSelectorWidget_->set_colors(reader.get_project_colors());
  }

  canvasView_->visible_in_view();
}


QT_BEGIN_NAMESPACE
