// Qt4Gui.cpp: KDE4/Qt4 Gui implementation for Gnash window
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <boost/assign/list_inserter.hpp>

#include <QMainWindow>
#include <QX11Info>
#include <QMenu>
#include <QMenuBar>
#include <QWidget>
#include <QCursor>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStack>
#include <QTabWidget>
#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
#include <QLCDNumber>
#include <QSpinBox>
#include <QClipboard>
#include <QString>
#include <QDesktopWidget>

#include "Range2d.h"

#include "smart_ptr.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "Qt4Gui.h"
#include "Qt4Gui.moc"
#include "Renderer.h"
#include "RunResources.h" 

// Macro for using gettext strings where Qt expects QStrings
#define _q(Str) QString::fromUtf8(_(Str))

extern "C" {
#include <X11/Xlib.h>
}

namespace gnash 
{

Qt4Gui::Qt4Gui(unsigned long xid, float scale, bool loop, RunResources& r)
 : Gui(xid, scale, loop, r)
{
}


Qt4Gui::~Qt4Gui()
{
}

void
Qt4Gui::setClipboard(const std::string& copy)
{
    QClipboard* cb = QApplication::clipboard();
    assert(cb);
    cb->setText(QString::fromStdString(copy));
}

bool
Qt4Gui::init(int /*argc*/, char ** /*argv*/[])
{

    char** r = NULL;
    int* i = new int(0);

    _application.reset(new QApplication(*i, r));
    _window.reset(new QMainWindow());
    _embedWidget = new EmbedWidget(*this);
    _drawingWidget = _embedWidget->drawingWidget();

    std::string renderer = _runResources.getRenderBackend();

    if (renderer == "cairo") {
#ifdef RENDERER_CAIRO
        log_debug("Using Cairo renderer");
        _glue.reset(new Qt4CairoGlue());
#else
        log_error(_("Cairo renderer not supported!"));
        return false;
#endif
    } else if (renderer == "opengl") {
#ifdef RENDERER_OPENGL
        log_debug("Using OpenGL renderer");
        _glue.reset(new Qt4OglGlue());
#else
        log_error(_("OpenGL renderer not supported!"));
        return false;
#endif
    } else if (renderer == "agg") {
#ifdef RENDERER_AGG
        log_debug("Using AGG renderer");
        _glue.reset(new Qt4AggGlue());
#else
        log_error(_("AGG renderer not supported!"));
        return false;
#endif
    }
    else {
        boost::format fmt = boost::format("Non-existent renderer %1% "
            "specified") % renderer;
        throw gnash::GnashException(fmt.str());
    }

    setupActions();
    setupMenus();

    if (!_xid) {
        createMainMenu();
    }

    // Make sure key events are ready to be passed
    // before the widget can receive them.
    setupKeyMap();
    
    return true;
}


bool
Qt4Gui::run()
{
    return _application->exec();
}

bool
Qt4Gui::createWindow(const char* windowtitle, int width, int height,
                     int xPosition, int yPosition)
{
    _width = width;
    _height = height;

    _drawingWidget->setMinimumSize(_width, _height);

    // Enable receiving of mouse events.
    _drawingWidget->setMouseTracking(true);
    _drawingWidget->setFocusPolicy(Qt::StrongFocus);
    _window->setWindowTitle(windowtitle);
    _window->setWindowIcon(QIcon(PKGDATADIR"/GnashG.png"));
    
    if(_xid) {
        _embedWidget->embedInto(_xid);
        _embedWidget->show();
        // Adjust width and height to the window we're being embedded into...
        XWindowAttributes winAttributes;
        XGetWindowAttributes(QX11Info::display(), _xid, &winAttributes);
        _width=winAttributes.width;
        _height=winAttributes.height;
    } else {
        // The QMainWindow takes ownership of the widgets.
        _window->setCentralWidget(_embedWidget);
        if (xPosition > -1 || yPosition > -1)
            _window->move(xPosition, yPosition);
        _window->show();
    }

    _glue->prepDrawingArea(_drawingWidget);

    _renderer.reset(_glue->createRenderHandler());

    if (!_renderer.get()) {
        return false;
    }

    _validbounds.setTo(0, 0, _width, _height);
    _glue->initBuffer(_width, _height);
    
    log_debug(_("Setting renderer"));

    _runResources.setRenderer(_renderer);
    
    log_debug(_("Set renderer"));
   
    return true;
}


void
Qt4Gui::resizeWindow(int width, int height)
{
    _width = width;
    _height = height;

    _drawingWidget->setMinimumSize(_width, _height);
}

void
Qt4Gui::popupMenu(const QPoint& point)
{
    QMenu popupMenu(_drawingWidget);
    popupMenu.addMenu(fileMenu);
    popupMenu.addMenu(editMenu);
    popupMenu.addMenu(movieControlMenu);
    popupMenu.addMenu(viewMenu);
    popupMenu.exec(point);
}


void
Qt4Gui::renderBuffer()
{
    
    for (DrawBounds::const_iterator i = _drawbounds.begin(),
                        e = _drawbounds.end(); i != e; ++i) {
        
        // it may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (i->isNull()) continue;
        
        assert(i->isFinite()); 

        _drawingWidget->update(i->getMinX(), i->getMinY(),
                               i->width(), i->height());

    }
}


void
Qt4Gui::renderWidget(const QRect& updateRect)
{
    // This call renders onto the widget using a QPainter,
    // which *must only happen inside a paint event*.
    _glue->render(updateRect);
}


void
Qt4Gui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _renderer->set_invalidated_regions(ranges);

    _drawbounds.clear();

    for (size_t i = 0, e = ranges.size(); i != e; ++i) {

        geometry::Range2d<int> bounds = Intersection(
        _renderer->world_to_pixel(ranges.getRange(i)),
        _validbounds);

        // It may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (bounds.isNull()) continue;

        assert(bounds.isFinite());

        _drawbounds.push_back(bounds);

    }
}


void
Qt4Gui::setTimeout(unsigned int timeout)
{
    // This must go through Gui::quit() to make sure screenshots are
    // handled if necessary.
    QTimer::singleShot(timeout, _drawingWidget, SLOT(quit()));
}


void
Qt4Gui::setInterval(unsigned int interval)
{
    _drawingWidget->startTimer(interval);
}


void
Qt4Gui::setCursor(gnash_cursor_type newcursor)
{
    if (! _mouseShown) return;

    switch (newcursor) {
        case CURSOR_HAND:
            _drawingWidget->setCursor(Qt::PointingHandCursor);
            break;
        case CURSOR_INPUT:
            _drawingWidget->setCursor(Qt::IBeamCursor); 
            break;
        default:
            _drawingWidget->unsetCursor(); 
    }
}

bool
Qt4Gui::showMouse(bool show)
{
    bool prevState = _mouseShown;
    _mouseShown = show;

    if (show) {
        _drawingWidget->unsetCursor();
    }
    else {
        _drawingWidget->setCursor(Qt::BlankCursor);
    }

    return prevState;
}

void
Qt4Gui::setFullscreen()
{
    _fullscreen = true;
    fullscreenAction->setChecked(_fullscreen);

    _embedWidget->setWindowFlags(Qt::Window);
    _embedWidget->showFullScreen();
}

void
Qt4Gui::unsetFullscreen()
{
    _fullscreen = false;
    fullscreenAction->setChecked(_fullscreen);

    if (_embedWidget->isFullScreen()) {
        _embedWidget->setWindowFlags(Qt::Widget);
        _embedWidget->showNormal();
        if (_xid) {
            _embedWidget->embedInto(_xid);
        }
    }
}
    
double
Qt4Gui::getScreenDPI() const
{
    assert(_drawingWidget);
    // Should this be logical or physical DPI?
    return _drawingWidget->logicalDpiX();
}

std::pair<int, int>
Qt4Gui::screenResolution() const
{
    QDesktopWidget* d = QApplication::desktop();
    assert(d);

    const QRect c = d->screenGeometry();
    return std::make_pair(c.width(), c.height());
}

gnash::key::code
Qt4Gui::qtToGnashKey(QKeyEvent *event)
{

    // This should be initialized by now.
    assert (!_keyMap.empty());

    // Gnash uses its own keycodes to map key events
    // to the three sometimes weird and confusing values that flash movies
    // can refer to. See GnashKey.h for the keycodes and map.
    //
    // Gnash's keycodes are gnash::key::code. They are mainly in ascii order.
    // Standard ascii characters (32-127) have the same value. Extended ascii
    // characters (160-254) are in ascii order but correspond to
    // gnash::key::code
    // 169-263. Non-character values must normally be mapped separately.

    const int key = event->key();

    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
          return static_cast<gnash::key::code>(
                key - Qt::Key_0 + gnash::key::_0);
    }

    // All other characters between ascii 32 and 126 are simple.
    // From space (32) to slash (47):
    else if (key >= Qt::Key_Space && key <= Qt::Key_AsciiTilde) {
        return static_cast<gnash::key::code>(
                key - Qt::Key_Space + gnash::key::SPACE);
    }

    // Function keys:
    else if (key >= Qt::Key_F1 && key <= Qt::Key_F15) {
        return static_cast<gnash::key::code>(
                key - Qt::Key_F1 + gnash::key::F1);
    }

    // Extended ascii from non-breaking (160) space to ÿ (264) is in the same
    // order.
    else if (key >= Qt::Key_nobreakspace && key <= Qt::Key_ydiaeresis) {
        return static_cast<gnash::key::code>(
                key - Qt::Key_nobreakspace + gnash::key::NOBREAKSPACE);
    }

    const KeyMap::const_iterator it = _keyMap.find(key);
    
    if (it == _keyMap.end()) return gnash::key::INVALID;
    
    return it->second;

}


int
Qt4Gui::qtToGnashModifier(const Qt::KeyboardModifiers modifiers)
{
    int gnashModifier = gnash::key::GNASH_MOD_NONE;

    if (modifiers & Qt::ShiftModifier)
        gnashModifier = gnashModifier | gnash::key::GNASH_MOD_SHIFT;
    if (modifiers & Qt::ControlModifier)
        gnashModifier = gnashModifier | gnash::key::GNASH_MOD_CONTROL;
    if (modifiers & Qt::AltModifier)
        gnashModifier = gnashModifier | gnash::key::GNASH_MOD_ALT;

    return gnashModifier;
}

void
Qt4Gui::handleKeyEvent(QKeyEvent *event, bool down)
{
    gnash::key::code c = qtToGnashKey(event);
    int mod = qtToGnashModifier(event->modifiers());
    notify_key_event(c, mod, down);
}


void
Qt4Gui::resize(int width, int height)
{
    _glue->resize(width, height);
    resize_view(width, height);
}

void
Qt4Gui::showProperties()
{
    QDialog* propsDialog = new QDialog(_drawingWidget);
    propsDialog->setWindowTitle(_q("Movie properties"));
    propsDialog->setAttribute(Qt::WA_DeleteOnClose);
    propsDialog->resize(500, 300);

    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
                 QDialogButtonBox::Close, Qt::Horizontal, propsDialog);
    dialogButtons->button(QDialogButtonBox::Close)->setDefault(true);

    QVBoxLayout* layout = new QVBoxLayout(propsDialog);
    propsDialog->connect(dialogButtons->button(QDialogButtonBox::Close),
            SIGNAL(clicked()), SLOT(close()));

#ifdef USE_SWFTREE
    std::auto_ptr<movie_root::InfoTree> infoptr = getMovieInfo();
    const movie_root::InfoTree& info = *infoptr;

    QTreeWidget *tree = new QTreeWidget();
    tree->setColumnCount(2);
    QStringList treeHeader;
    treeHeader.append(_q("Variable"));
    treeHeader.append(_q("Value"));
    tree->setHeaderLabels(treeHeader);

    QList<QTreeWidgetItem *> items;

    int prevDepth = 0;
    QStack<QTreeWidgetItem*> stack;
    for (movie_root::InfoTree::iterator i = info.begin(), e = info.end();
            i != e; ++i) {

        const movie_root::InfoTree::value_type& p = *i;

        QStringList cols;
        cols.append(p.first.c_str());
        cols.append(p.second.c_str());
        QTreeWidgetItem* item = new QTreeWidgetItem(cols);

        int newDepth = info.depth(i);

        if (newDepth == 0) {
            // Insert top level entries directly into the tree widget.
            items.append(item);
            stack.empty();
        } else {
            // The position to insert the new row.
            QTreeWidgetItem* parent = NULL;

            if (newDepth == prevDepth ) {
                // Pop an extra time if there is a sibling on the stack.
                int size = stack.size();
                if (size + 1 > newDepth)
                    stack.pop();

                parent = stack.pop();
            } else if (newDepth > prevDepth) {
                parent = stack.pop();
            } else if (newDepth < prevDepth) {
                // Pop until the stack has the right depth.
                int size = stack.size();
                for (int j = 0; j < (size + 1) - newDepth; ++j) {
                    parent = stack.pop();
                }
            }

            parent->addChild(item);
            stack.push(parent);
        }

        stack.push(item);
        prevDepth = newDepth;
    }
    tree->insertTopLevelItems(0, items);
    layout->addWidget(tree);

#endif // USE_SWFTREE
    layout->addWidget(dialogButtons);

    propsDialog->show();
    propsDialog->activateWindow();
}



void
Qt4Gui::showPreferences()
{
    Qt4GuiPrefs::PreferencesDialog* prefsDialog = new Qt4GuiPrefs::PreferencesDialog(_drawingWidget);

    prefsDialog->setAttribute(Qt::WA_DeleteOnClose);
    prefsDialog->show();
    prefsDialog->raise();
    prefsDialog->activateWindow();
}


void
Qt4Gui::quitUI()
{
    _application->quit();
}


void
Qt4Gui::setupActions()
{

    // File Menu actions
    propertiesAction = new QAction(_q("Properties"), _window.get());
    _drawingWidget->connect(propertiesAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(properties()));

    quitAction = new QAction(_q("Quit Gnash"), _window.get());
    // This must go through Gui::quit() to make sure we don't exit
    // before doing whatever the Gui wants to do on exit.
    _drawingWidget->connect(quitAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(quit()));

    // Edit Menu actions
    preferencesAction = new QAction(_q("Preferences"), _window.get());
    _drawingWidget->connect(preferencesAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(preferences()));

    // Movie Control Menu actions
    playAction = new QAction(_q("Play"), _window.get());
    _drawingWidget->connect(playAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(play()));

    pauseAction = new QAction(_q("Pause"), _window.get());
    _drawingWidget->connect(pauseAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(pause()));

    stopAction = new QAction(_q("Stop"), _window.get());
    _drawingWidget->connect(stopAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(stop()));

    restartAction = new QAction(_q("Restart"), _window.get());
    _drawingWidget->connect(restartAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(restart()));

    // View Menu actions
    refreshAction = new QAction(_q("Refresh"), _window.get());
    _drawingWidget->connect(refreshAction, SIGNAL(triggered()),
                     _drawingWidget, SLOT(refresh()));

    fullscreenAction = new QAction(_q("Fullscreen"), _window.get());
    fullscreenAction->setCheckable(true);
    _drawingWidget->connect(fullscreenAction, SIGNAL(toggled(bool)),
                           _drawingWidget, SLOT(fullscreen(bool)));
}


void
Qt4Gui::setupMenus()
{
    /// The menus are children of the QMainWindow so that
    /// they are destroyed on exit. The QMainWindow already has
    /// ownership of the main QMenuBar.

    // Set up the File menu.
    fileMenu = new QMenu(_q("File"), _window.get());
    fileMenu->addAction(propertiesAction);
    fileMenu->addAction(quitAction);

    // Set up the Edit menu.
    editMenu = new QMenu(_q("Edit"), _window.get());
    editMenu->addAction(preferencesAction);

    // Set up the Movie Control menu
    movieControlMenu = new QMenu(_q("Movie Control"), _window.get());
    movieControlMenu->addAction(playAction);
    movieControlMenu->addAction(pauseAction);
    movieControlMenu->addAction(stopAction);
    movieControlMenu->addAction(restartAction);

    // Set up the View menu
    viewMenu = new QMenu(_q("View"), _window.get());
    viewMenu->addAction(refreshAction);
    viewMenu->addAction(fullscreenAction);
}


void
Qt4Gui::createMainMenu()
{
    std::auto_ptr<QMenuBar> mainMenu(new QMenuBar);

    // Set up the menu bar.
    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(editMenu);
    mainMenu->addMenu(movieControlMenu);
    mainMenu->addMenu(viewMenu);

    // The QMainWindow::setMenuBar transfers ownership
    // of the QMenuBar.
    _window->setMenuBar(mainMenu.release());

}

void
Qt4Gui::setupKeyMap()
{
    // We only want to do this once, although it would not
    // be harmful to do it more.
    assert (_keyMap.empty());
    
    boost::assign::insert(_keyMap)
    (Qt::Key_Backspace, gnash::key::BACKSPACE)
    (Qt::Key_Tab, gnash::key::TAB)
    (Qt::Key_Clear, gnash::key::CLEAR)
    (Qt::Key_Return, gnash::key::ENTER)
    (Qt::Key_Enter, gnash::key::ENTER)
    (Qt::Key_Shift, gnash::key::SHIFT)
    (Qt::Key_Control, gnash::key::CONTROL)
    (Qt::Key_Alt, gnash::key::ALT)
    (Qt::Key_CapsLock, gnash::key::CAPSLOCK)
    (Qt::Key_Escape, gnash::key::ESCAPE)
    (Qt::Key_Space, gnash::key::SPACE)
    (Qt::Key_PageDown, gnash::key::PGDN)
    (Qt::Key_PageUp, gnash::key::PGUP)
    (Qt::Key_Home, gnash::key::HOME)
    (Qt::Key_End, gnash::key::END)
    (Qt::Key_Left, gnash::key::LEFT)
    (Qt::Key_Up, gnash::key::UP)
    (Qt::Key_Right, gnash::key::RIGHT)
    (Qt::Key_Down, gnash::key::DOWN)
    (Qt::Key_Insert, gnash::key::INSERT)
    (Qt::Key_Delete, gnash::key::DELETEKEY)
    (Qt::Key_Help, gnash::key::HELP)
    (Qt::Key_NumLock, gnash::key::NUM_LOCK)
    (Qt::Key_Semicolon, gnash::key::SEMICOLON)
    (Qt::Key_Equal, gnash::key::EQUALS)
    (Qt::Key_Minus, gnash::key::MINUS)
    (Qt::Key_Slash, gnash::key::SLASH)
    (Qt::Key_BracketLeft, gnash::key::LEFT_BRACKET)
    (Qt::Key_Backslash, gnash::key::BACKSLASH)
    (Qt::Key_BracketRight, gnash::key::RIGHT_BRACKET)
    (Qt::Key_QuoteDbl, gnash::key::DOUBLE_QUOTE);
}

void
Qt4Gui::playHook()
{
    _embedWidget->hidePlayButton();
}

void
Qt4Gui::stopHook()
{
    _embedWidget->showPlayButton();
}

/// EmbedWidget implementation

EmbedWidget::EmbedWidget(Qt4Gui& gui)
  : QX11EmbedWidget()
{
    _drawingWidget = new DrawingWidget(gui);
    _playButton = new QPushButton(_q("Click to Play"), this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(_playButton);
    layout->addWidget(_drawingWidget);
    _playButton->hide();

    connect(_playButton, SIGNAL(clicked()), this, SLOT(hidePlayButton()));
    connect(_playButton, SIGNAL(clicked()), _drawingWidget, SLOT(play()));
}

void
EmbedWidget::hidePlayButton()
{
    _playButton->hide();
}

void
EmbedWidget::showPlayButton()
{
    _playButton->show();
}

namespace Qt4GuiPrefs {

PreferencesDialog::PreferencesDialog(QWidget* parent)
    :
    QDialog(parent),
    _rcfile(RcInitFile::getDefaultInstance())
{
    setWindowTitle(_q("Gnash preferences"));
    setAttribute(Qt::WA_DeleteOnClose);

    QDialogButtonBox *buttons = new QDialogButtonBox(
                          QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout *vLayout = new QVBoxLayout(this);

    QLabel* tmpLabel;

    // Make notebook pages.
    QTabWidget *tabs = new QTabWidget;

    // Logging tab
    QFrame* loggingTab = new QFrame();
    tabs->addTab(loggingTab, _q("Logging"));
    QVBoxLayout* layout = new QVBoxLayout (loggingTab);

    tmpLabel = new QLabel(_q("<b>Logging options</b>"), loggingTab);
    layout->addWidget(tmpLabel);

    tmpLabel = new QLabel(_q("Verbosity level"), loggingTab);
    layout->addWidget(tmpLabel);

    QLCDNumber* lcd = new QLCDNumber(loggingTab);
    lcd->display(_rcfile.verbosityLevel());
    lcd->setNumDigits(2);
    layout->addWidget(lcd);

    _verbositySlider = new QSlider(loggingTab);
    _verbositySlider->setOrientation(Qt::Horizontal);
    _verbositySlider->setMaximum(10);
    _verbositySlider->setSingleStep(1);
    _verbositySlider->setPageStep(1);
    _verbositySlider->setValue(_rcfile.verbosityLevel());
    connect(_verbositySlider, SIGNAL(valueChanged(int)),
            lcd, SLOT(display(int)));
    layout->addWidget(_verbositySlider);

    _logToFileToggle = new QCheckBox(_q("Log to file"), loggingTab);
    _logToFileToggle->setChecked(_rcfile.useWriteLog());
    layout->addWidget(_logToFileToggle);

    _logFileName = new QLineEdit(_rcfile.getDebugLog().c_str(), loggingTab);
    layout->addWidget(_logFileName);

    _parserDumpToggle = new QCheckBox(_q("Log parser output"), loggingTab);
    _parserDumpToggle->setChecked(_rcfile.useParserDump());
    layout->addWidget(_parserDumpToggle);

    _actionDumpToggle = new QCheckBox(_q("Log SWF actions"), loggingTab);
    _actionDumpToggle->setChecked(_rcfile.useActionDump());
    layout->addWidget(_actionDumpToggle);

    _malformedSWFToggle = new QCheckBox(_q("Log malformed SWF errors"),
                                        loggingTab);
    _malformedSWFToggle->setChecked(_rcfile.showMalformedSWFErrors());
    layout->addWidget(_malformedSWFToggle);

    _ASCodingErrorToggle = new QCheckBox(_q("Log ActionScript coding errors"),
                                        loggingTab);
    _ASCodingErrorToggle->setChecked(_rcfile.showASCodingErrors());
    layout->addWidget(_ASCodingErrorToggle);

    // Security tab
    QFrame* securityTab = new QFrame(tabs);
    tabs->addTab(securityTab, _q("Security"));
    layout = new QVBoxLayout (securityTab);

    tmpLabel = new QLabel(_q("<b>Network connections</b>"), securityTab);
    layout->addWidget(tmpLabel);

    _localHostToggle = new QCheckBox(_q("Connect only to local host"),
                                     securityTab);
    _localHostToggle->setChecked(_rcfile.useLocalHost());
    layout->addWidget(_localHostToggle);

    _localDomainToggle = new QCheckBox(_q("Connect only to local domain"),
                                       securityTab);
    _localDomainToggle->setChecked(_rcfile.useLocalDomain());
    layout->addWidget(_localDomainToggle);

    _insecureSSLToggle = new QCheckBox(_q("Disable SSL verification"),
                                       securityTab);
    _insecureSSLToggle->setChecked(_rcfile.insecureSSL());
    layout->addWidget(_insecureSSLToggle);

    tmpLabel = new QLabel(_q("<b>Privacy</b>"), securityTab);
    layout->addWidget(tmpLabel);

    tmpLabel = new QLabel(_q("Shared objects directory:"), securityTab);
    layout->addWidget(tmpLabel);
    _solSandboxDir = new QLineEdit(_rcfile.getSOLSafeDir().c_str(),
                                   securityTab);
    layout->addWidget(_solSandboxDir);

    _solReadOnlyToggle = new QCheckBox(_q("Do not write Shared Object files"),
                                       securityTab);
    _solReadOnlyToggle->setChecked(_rcfile.getSOLReadOnly());
    layout->addWidget(_solReadOnlyToggle);

    _solLocalDomainToggle = new QCheckBox(
            _q("Only access local Shared Object files"), securityTab);
    _solLocalDomainToggle->setChecked(_rcfile.getSOLLocalDomain());
    layout->addWidget(_solLocalDomainToggle);

    _localConnectionToggle = new QCheckBox(
            _q("Disable Local Connection object"), securityTab);
    _localConnectionToggle->setChecked(_rcfile.getLocalConnection());
    layout->addWidget(_localConnectionToggle);
    layout->addStretch();

    // Network tab
    QFrame* networkTab = new QFrame(tabs);
    tabs->addTab(networkTab, _q("Network"));
    layout = new QVBoxLayout (networkTab);

    tmpLabel = new QLabel(_q("<b>Network preferences</b>"), networkTab);
    layout->addWidget(tmpLabel);

    tmpLabel = new QLabel(_q("Network timeout in seconds"), networkTab);
    layout->addWidget(tmpLabel);

    _streamsTimeoutScale = new QSpinBox(networkTab);
    _streamsTimeoutScale->setMinimum(0);
    _streamsTimeoutScale->setMaximum(300);
    _streamsTimeoutScale->setValue(_rcfile.getStreamsTimeout());
    layout->addWidget(_streamsTimeoutScale);
    layout->addStretch();

    // Network tab
    QFrame* mediaTab = new QFrame(tabs);
    tabs->addTab(mediaTab, _q("Media"));
    layout = new QVBoxLayout (mediaTab);

    tmpLabel = new QLabel(_q("<b>Sound</b>"), mediaTab);
    layout->addWidget(tmpLabel);

    _soundToggle = new QCheckBox(_q("Use sound handler"), mediaTab);
    _soundToggle->setChecked(_rcfile.useSound());
    layout->addWidget(_soundToggle);

    _saveStreamingMediaToggle = new QCheckBox(_q("Save media streams to disk"),
                                              mediaTab);
    _saveStreamingMediaToggle->setChecked(_rcfile.saveStreamingMedia());
    layout->addWidget(_saveStreamingMediaToggle);

    _saveLoadedMediaToggle = new QCheckBox(
                        _q("Save dynamically loaded media to disk"), mediaTab);
    _saveLoadedMediaToggle->setChecked(_rcfile.saveLoadedMedia());
    layout->addWidget(_saveLoadedMediaToggle);

    tmpLabel = new QLabel(_q("Saved media directory:"), mediaTab);
    layout->addWidget(tmpLabel);

    _mediaDir = new QLineEdit(_rcfile.getMediaDir().c_str(), mediaTab);
    layout->addWidget(_mediaDir);
    layout->addStretch();

    // Player tab
    QFrame* playerTab = new QFrame(tabs);
    tabs->addTab(playerTab, _q("Player"));
    layout = new QVBoxLayout (playerTab);

    tmpLabel = new QLabel(_q("<b>Player description</b>"), playerTab);
    layout->addWidget(tmpLabel);

    tmpLabel = new QLabel(_q("Player version:"), playerTab);
    layout->addWidget(tmpLabel);
    _versionText = new QLineEdit(_rcfile.getFlashVersionString().c_str(),
                                 playerTab);
    layout->addWidget(_versionText);

    tmpLabel = new QLabel(_q("Operating system:"), playerTab);
    layout->addWidget(tmpLabel);

    _osText = new QLineEdit(playerTab);
    if (_rcfile.getFlashSystemOS().empty()) {
        _osText->setText(_q("<Autodetect>"));
    } else {
        _osText->setText(_rcfile.getFlashSystemOS().c_str());
    }
    layout->addWidget(_osText);

    tmpLabel = new QLabel(_q("URL opener:"), playerTab);
    layout->addWidget(tmpLabel);

    _urlOpenerText = new QLineEdit(_rcfile.getURLOpenerFormat().c_str(),
                                 playerTab);
    layout->addWidget(_urlOpenerText);

    tmpLabel = new QLabel(_q("<b>Performance</b>"), playerTab);
    layout->addWidget(tmpLabel);

    tmpLabel = new QLabel(_q("Max size of movie library:"), playerTab);
    layout->addWidget(tmpLabel);

    _librarySize = new QSpinBox(playerTab);
    _librarySize->setMinimum(0);
    _librarySize->setMaximum(100);
    _librarySize->setValue(_rcfile.getMovieLibraryLimit());
    layout->addWidget(_librarySize);

    _startStoppedToggle = new QCheckBox(_q("Start Gnash in pause mode"),
                                        playerTab);
    _startStoppedToggle->setChecked(_rcfile.startStopped());
    layout->addWidget(_startStoppedToggle);
    layout->addStretch();
    // End of notebook tabs

    vLayout->addWidget(tabs);
    vLayout->addStretch();
    vLayout->addWidget(buttons);

    // Connect the dialog buttons.
    connect(buttons, SIGNAL(accepted()), this, SLOT(savePreferences()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void
PreferencesDialog::savePreferences()
{
    // Logging tab widgets
    _rcfile.verbosityLevel(_verbositySlider->value());
    _rcfile.useWriteLog(_logToFileToggle->isChecked());
    _rcfile.setDebugLog(_logFileName->text().toStdString());
    _rcfile.useParserDump(_parserDumpToggle->isChecked());
    _rcfile.useActionDump(_actionDumpToggle->isChecked());
    _rcfile.showMalformedSWFErrors(_malformedSWFToggle->isChecked());
    _rcfile.showASCodingErrors(_ASCodingErrorToggle->isChecked());

    // Security tab widgets
    _rcfile.useLocalHost(_localHostToggle->isChecked());
    _rcfile.useLocalDomain(_localDomainToggle->isChecked());
    _rcfile.insecureSSL(_insecureSSLToggle->isChecked());
    _rcfile.setSOLSafeDir(_solSandboxDir->text().toStdString());
    _rcfile.setSOLReadOnly(_solReadOnlyToggle->isChecked());
    _rcfile.setSOLLocalDomain(_solLocalDomainToggle->isChecked());
    _rcfile.setLocalConnection(_localConnectionToggle->isChecked());

    // Network tab widgets
    _rcfile.setStreamsTimeout(_streamsTimeoutScale->value());

    // Media tab widgets
    _rcfile.useSound(_soundToggle->isChecked());
    _rcfile.saveStreamingMedia(_saveStreamingMediaToggle->isChecked());
    _rcfile.saveLoadedMedia(_saveLoadedMediaToggle->isChecked());
    _rcfile.setMediaDir(_mediaDir->text().toStdString());

    // Player tab widgets
    _rcfile.setFlashVersionString(_versionText->text().toStdString());
    if (_osText->text() != _q("<Autodetect>")) {
        _rcfile.setFlashSystemOS(_osText->text().toStdString());
    }
    _rcfile.setURLOpenerFormat(_urlOpenerText->text().toStdString());
    _rcfile.setMovieLibraryLimit(_librarySize->value());
    _rcfile.startStopped(_startStoppedToggle->isChecked());

    // Save the file.
    _rcfile.updateFile();

    // Allow the dialog to close normally.
    emit accept();
}

} // End of Qt4GuiPrefs namespace

}

