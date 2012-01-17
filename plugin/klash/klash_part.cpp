// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

// Written by Koos Vriezen <koos ! vriezen ? xs4all ! nl>

#ifdef KDE_USE_FINAL
#undef Always
#endif

#include "gnashconfig.h"
#include <cassert>
#include <qcstring.h>
#include <qpopupmenu.h>
#include <qtimer.h>

class KXMLGUIClient; // workaround for kde3.3 on sarge with gcc4, kactioncollection.h does not forward declare KXMLGUIClient
#include <klibloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kinstance.h>
#include <kparts/factory.h>
#include <kstaticdeleter.h>

#include "klash_part.h"

//-----------------------------------------------------------------------------

class KlashFactory : public KParts::Factory {
public:
    KDE_NO_CDTOR_EXPORT KlashFactory ();
    KDE_NO_CDTOR_EXPORT virtual ~KlashFactory ();
    KDE_NO_CDTOR_EXPORT virtual KParts::Part *createPartObject
        (QWidget *wparent, const char *wname,
         QObject *parent, const char *name,
         const char *className, const QStringList &args);
    static KInstance * instance () { return s_instance; }
private:
    static KInstance * s_instance;
};

K_EXPORT_COMPONENT_FACTORY (libklashpart, KlashFactory)

KInstance *KlashFactory::s_instance = 0;

KlashFactory::KlashFactory () {
    s_instance = new KInstance ("klash");
}

KlashFactory::~KlashFactory () {
    delete s_instance;
}

KParts::Part *KlashFactory::createPartObject
  (QWidget *wparent, const char *wname,
   QObject *parent, const char * name,
   const char * /*cls*/, const QStringList & args) {
      //kdDebug() << "KlashFactory::createPartObject " << cls << endl;
      return new KlashPart (wparent, wname, parent, name, args);
}

//-----------------------------------------------------------------------------

static bool getBoolValue (const QString & value) {
    return (value.lower() != QString::fromLatin1("false") &&
            value.lower() != QString::fromLatin1("off") &&
            value.lower() != QString::fromLatin1("0"));
}

KDE_NO_CDTOR_EXPORT KlashPart::KlashPart (QWidget * wparent, const char *wname,
                    QObject * parent, const char *name, const QStringList &args)
 : KParts::ReadOnlyPart (parent, name),
   //new KSimpleConfig ("klashrc")),
   m_browserextension (new KlashBrowserExtension (this)),
   m_liveconnectextension (new KlashLiveConnectExtension (this)),
   m_process (0L),
   m_width (0),
   m_height (0),
   m_autostart (false),
   m_fullscreen (false),
   m_started_emited (false) {
    //kdDebug () << "KlashPart(" << this << ")::KlashPart ()" << endl;
    bool show_fullscreen = false;
    setInstance (KlashFactory::instance (), true);
    /*KAction *playact =*/ new KAction(i18n("P&lay"), QString ("player_play"), KShortcut (), this, SLOT(play ()), actionCollection (), "play");
    /*KAction *pauseact =*/ new KAction(i18n("&Pause"), QString ("player_pause"), KShortcut (), this, SLOT(pause ()), actionCollection (), "pause");
    /*KAction *stopact =*/ new KAction(i18n("&Stop"), QString ("player_stop"), KShortcut (), this, SLOT(stop ()), actionCollection (), "stop");
    //new KAction (i18n ("Increase Volume"), QString ("player_volume"), KShortcut (), this, SLOT (increaseVolume ()), actionCollection (), "edit_volume_up");
    //new KAction (i18n ("Decrease Volume"), QString ("player_volume"), KShortcut (), this, SLOT (decreaseVolume ()), actionCollection (), "edit_volume_down");
    QStringList::const_iterator end = args.end ();
    for (QStringList::const_iterator it = args.begin () ; it != end; ++it) {
        int equalPos = (*it).find("=");
        if (equalPos > 0) {
            QString name = (*it).left (equalPos).lower ();
            QString value = (*it).right ((*it).length () - equalPos - 1);
            if (value.at(0)=='\"')
                value = value.right (value.length () - 1);
            if (value.at (value.length () - 1) == '\"')
                value.truncate (value.length () - 1);
            //kdDebug () << "name=" << name << " value=" << value << endl;
            if (name == QString::fromLatin1("width")) {
                m_width = value.toInt ();
            } else if (name == QString::fromLatin1("height")) {
                m_height = value.toInt ();
            //} else if (name == QString::fromLatin1("type")) {
            } else if (name == QString::fromLatin1("__khtml__pluginbaseurl")) {
                m_docbase = KURL (value);
            } else if (name == QString::fromLatin1("src")) {
                m_src_url = value;
            } else if (name == QString::fromLatin1 ("fullscreenmode")) {
                show_fullscreen = getBoolValue (value);
            } else if (name == QString::fromLatin1 ("autostart")) {
                bool ok;
                m_autostart = value.toInt (&ok);
                if (!ok)
                    m_autostart = (value.lower () == "false");
            }
            m_args.push_back(name + QChar('=') + value);
        }
    }
    KParts::Part::setWidget (new KlashView (wparent));
    setXMLFile("klashpartui.rc");
    setProgressInfoEnabled (false);

    if (m_fullscreen)
        fullScreen ();
}

KDE_NO_CDTOR_EXPORT KlashPart::~KlashPart () {
    kdDebug() << "KlashPart::~KlashPart" << endl;
    stop ();
    //delete m_config;
    //m_config = 0L;
}

KDE_NO_EXPORT bool KlashPart::allowRedir (const KURL & url) const {
    return kapp->authorizeURLAction ("redirect", m_docbase, url);
}

KDE_NO_EXPORT void KlashPart::play ()
{

    QString procname;
    char *gnash_env = getenv("KLASH_PLAYER");
    if (!gnash_env) {
      procname = GNASHBINDIR "/kde-gnash";
    } else {
      procname = gnash_env;
    }

    stop ();
    if (m_src_url.isEmpty ())
        return;
    m_process = new KProcess;
    m_process->setUseShell (true);
    m_process->setEnvironment (QString::fromLatin1 ("SESSION_MANAGER"), QString::fromLatin1 (""));

    QString cmd = procname + QString(" -x ") +
                             QString::number(static_cast<KlashView*>(widget())->embedId());

    if (m_width > 0 && m_height > 0)
        cmd += QString(" -j ") + QString::number(m_width) +
               QString(" -k ") + QString::number(m_height);

    QString url = m_url.url();
    if (!url.isEmpty())
        cmd += QString(" -u ") + KProcess::quote(url);
    url = m_docbase.url();
    if (!url.isEmpty())
        cmd += QString(" -U ") + KProcess::quote(url);

    for (QStringList::const_iterator it=m_args.begin(), end=m_args.end();it != end; ++it)
        cmd += QString(" -P ") + KProcess::quote(*it);

    cmd += QString (" ") + KProcess::quote(m_src_url);

    kdDebug () << cmd << endl;
    *m_process << cmd;

    connect (m_process, SIGNAL (processExited (KProcess *)),
            this, SLOT (processStopped (KProcess *)));
    m_process->start (KProcess::NotifyOnExit, KProcess::NoCommunication);
}

KDE_NO_EXPORT void KlashPart::pause () {
}

KDE_NO_EXPORT void KlashPart::stop () {
    if (m_process) {
        if (m_process->isRunning ()) {
            ; // IPC close
            //m_process->wait(2);
	    
	    // Ignore SIGTERM, so we won't kill ourselves.
            void (*oldhandler)(int) = signal(SIGTERM, SIG_IGN);

	    int pid = -1 * ::getpid();
	    assert(pid < -1);

	    // Terminate every process in our process group.
            ::kill (pid, SIGTERM);

	    // Restore the old handler.
            signal(SIGTERM, oldhandler);
            m_process->wait(2);
        }
        delete m_process;
        m_process = 0L;
    }
}

bool KlashPart::openFile() {
    if (!m_file.isEmpty ())
        m_src_url = m_file;
    play ();
    return true;
}

KDE_NO_EXPORT bool KlashPart::openURL (const KURL & url) {
    kdDebug () << "KlashPart::openURL " << url.url() << endl;
    emit started (0);
    return KParts::ReadOnlyPart::openURL (url);
}

KDE_NO_EXPORT bool KlashPart::closeURL () {
    return KParts::ReadOnlyPart::closeURL ();
}

KDE_NO_EXPORT void KlashPart::fullScreen () {
}

KDE_NO_EXPORT void KlashPart::setLoaded (int percentage) {
    if (percentage < 100) {
        m_browserextension->setLoadingProgress (percentage);
        m_browserextension->infoMessage
            (QString::number (percentage) + i18n ("% Cache fill"));
    }
}

KDE_NO_EXPORT void KlashPart::playingStarted () {
    //kdDebug () << "KlashPart::processStartedPlaying " << endl;
    //if (m_settings->sizeratio && !m_noresize && m_source->width() > 0 && m_source->height() > 0)
    //    m_liveconnectextension->setSize (m_source->width(), m_source->height());
    m_browserextension->setLoadingProgress (100);
    if (m_started_emited) {
        emit completed ();
        m_started_emited = false;
    }
    m_liveconnectextension->started ();
    m_browserextension->infoMessage (i18n("Klash: Playing"));
}

KDE_NO_EXPORT void KlashPart::playingStopped () {
    if (m_started_emited) {
        m_started_emited = false;
        m_browserextension->setLoadingProgress (100);
        emit completed ();
    }
    m_liveconnectextension->finished ();
    m_browserextension->infoMessage (i18n ("Klash: Stop Playing"));
}

KDE_NO_EXPORT void KlashPart::processStopped (KProcess *) {
    QTimer::singleShot (0, this, SLOT (playingStopped ()));
}

//---------------------------------------------------------------------

KDE_NO_CDTOR_EXPORT KlashBrowserExtension::KlashBrowserExtension (KlashPart * parent)
  : KParts::BrowserExtension (parent, "Klash Browser Extension") {
}

KDE_NO_EXPORT void KlashBrowserExtension::urlChanged (const QString & url) {
    emit setLocationBarURL (url);
}

KDE_NO_EXPORT void KlashBrowserExtension::setLoadingProgress (int percentage) {
    emit loadingProgress (percentage);
}

KDE_NO_EXPORT void KlashBrowserExtension::saveState (QDataStream & stream) {
    stream << static_cast <KlashPart *> (parent ())->source ();
}

KDE_NO_EXPORT void KlashBrowserExtension::restoreState (QDataStream & stream) {
    QString url;
    stream >> url;
    static_cast <KlashPart *> (parent ())->openURL (KURL(url));
}

KDE_NO_EXPORT void KlashBrowserExtension::requestOpenURL (const KURL & url, const QString & target, const QString & service) {
    KParts::URLArgs args;
    args.frameName = target;
    args.serviceType = service;
    emit openURLRequest (url, args);
}

//---------------------------------------------------------------------
/*
 * add
 * .error.errorCount
 * .error.item(count)
 *   .errorDescription
 *   .errorCode
 * .controls.stop()
 * .controls.play()
 */

enum JSCommand {
    notsupported,
    isfullscreen, isloop,
    length, width, height, position, prop_source, source, setsource,
    play, jsc_pause, start, stop,
    prop_volume, volume, setvolume
};

struct KLASH_NO_EXPORT JSCommandEntry {
    const char * name;
    JSCommand command;
    const char * defaultvalue;
    const KParts::LiveConnectExtension::Type rettype;
};

// keep this list in alphabetic order
// http://service.real.com/help/library/guides/realonescripting/browse/htmfiles/embedmet.htm
static const JSCommandEntry JSCommandList [] = {
    { "GetSource", source, 0L, KParts::LiveConnectExtension::TypeString },
    { "GetTitle", notsupported, "title", KParts::LiveConnectExtension::TypeString },
    { "GetVolume", volume, "100", KParts::LiveConnectExtension::TypeNumber },
    { "Pause", jsc_pause, 0L, KParts::LiveConnectExtension::TypeBool },
    { "Play", play, 0L, KParts::LiveConnectExtension::TypeBool },
    { "SetSource", setsource, 0L, KParts::LiveConnectExtension::TypeBool },
    { "SetVolume", setvolume, "true", KParts::LiveConnectExtension::TypeBool },
    { "Start", start, 0L, KParts::LiveConnectExtension::TypeBool },
    { "Stop", stop, 0L, KParts::LiveConnectExtension::TypeBool },
    { "Volume", prop_volume, "100", KParts::LiveConnectExtension::TypeNumber },
    { "pause", jsc_pause, 0L, KParts::LiveConnectExtension::TypeBool },
    { "play", play, 0L, KParts::LiveConnectExtension::TypeBool },
    { "stop", stop, 0L, KParts::LiveConnectExtension::TypeBool },
    { "volume", volume, 0L, KParts::LiveConnectExtension::TypeBool },
};

static const JSCommandEntry * getJSCommandEntry (const char * name, int start = 0, int end = sizeof (JSCommandList)/sizeof (JSCommandEntry)) {
    if (end - start < 2) {
        if (start != end && !strcasecmp (JSCommandList[start].name, name))
            return &JSCommandList[start];
        return 0L;
    }
    int mid = (start + end) / 2;
    int cmp = strcasecmp (JSCommandList[mid].name, name);
    if (cmp < 0)
        return getJSCommandEntry (name, mid + 1, end);
    if (cmp > 0)
        return getJSCommandEntry (name, start, mid);
    return &JSCommandList[mid];
}

KDE_NO_CDTOR_EXPORT KlashLiveConnectExtension::KlashLiveConnectExtension (KlashPart * parent)
  : KParts::LiveConnectExtension (parent), player (parent),
    lastJSCommandEntry (0L),
    m_started (false),
    m_enablefinish (false) {
      connect (parent, SIGNAL (started (KIO::Job *)), this, SLOT (started ()));
}

KDE_NO_CDTOR_EXPORT KlashLiveConnectExtension::~KlashLiveConnectExtension() {
    //kdDebug () << "KlashLiveConnectExtension::~KlashLiveConnectExtension()" << endl;
}

KDE_NO_EXPORT void KlashLiveConnectExtension::started () {
    m_started = true;
}

KDE_NO_EXPORT void KlashLiveConnectExtension::finished () {
    if (m_started && m_enablefinish) {
        KParts::LiveConnectExtension::ArgList args;
        args.push_back (qMakePair (KParts::LiveConnectExtension::TypeString, QString("if (window.onFinished) onFinished();")));
        emit partEvent (0, "eval", args);
        m_started = true;
        m_enablefinish = false;
    }
}

KDE_NO_EXPORT bool KlashLiveConnectExtension::get
  (const unsigned long id, const QString & name,
   KParts::LiveConnectExtension::Type & type,
   unsigned long & rid, QString & rval) {
    const char * str = name.ascii ();
    kdDebug () << "[01;35mget[00m " << str << endl;
    const JSCommandEntry * entry = getJSCommandEntry (str);
    if (!entry)
        return false;
    rid = id;
    type = entry->rettype;
    switch (entry->command) {
        case prop_source:
            type = KParts::LiveConnectExtension::TypeString;
            rval = player->source ();
            break;
        case prop_volume:
            //if (player->view ())
            //    rval = QString::number (??);
            break;
        default:
            lastJSCommandEntry = entry;
            type = KParts::LiveConnectExtension::TypeFunction;
    }
    return true;
}

KDE_NO_EXPORT bool KlashLiveConnectExtension::put
  (const unsigned long, const QString & name, const QString & val) {
    kdDebug () << "[01;35mput[00m " << name << "=" << val << endl;
    const JSCommandEntry * entry = getJSCommandEntry (name.ascii ());
    if (!entry)
        return false;
    switch (entry->command) {
        case prop_source: {
            KURL url (val);
            if (player->allowRedir (url))
                player->openURL (url);
            break;
        }
        case prop_volume:
            //if (player->view ())
            //    ?? (val.toInt ());
            break;
        default:
            return false;
    }
    return true;
}

KDE_NO_EXPORT bool KlashLiveConnectExtension::call
  (const unsigned long id, const QString & name,
   const QStringList & args, KParts::LiveConnectExtension::Type & type,
   unsigned long & rid, QString & rval) {
    const JSCommandEntry * entry = lastJSCommandEntry;
    const char * str = name.ascii ();
    if (!entry || strcmp (entry->name, str))
        entry = getJSCommandEntry (str);
    if (!entry)
        return false;
    kdDebug () << "[01;35mentry[00m " << entry->name << endl;
    for (unsigned int i = 0; i < args.size (); ++i)
        kdDebug () << "      " << args[i] << endl;
    //Klash::View * view = static_cast <Klash::View*> (player->widget ());
    //if (!view)
    //    return false;
    rid = id;
    type = entry->rettype;
    switch (entry->command) {
        case notsupported:
            if (entry->rettype != KParts::LiveConnectExtension::TypeVoid)
                rval = entry->defaultvalue;
            break;
        case play:
            if (args.size ()) {
                KURL url (args.first ());
                if (player->allowRedir (url))
                    ;//player->openNewURL (url);
            } else
                player->play ();
            rval = "true";
            break;
        case start:
            player->play ();
            rval = "true";
            break;
        case stop:
            player->stop ();
            rval = "true";
            break;
        case jsc_pause:
            player->pause ();
            rval = "true";
            break;
        case length:
            rval.setNum (0 /*player->source ()->length ()*/);
            break;
        case width:
            rval.setNum (player->width ());
            break;
        case height:
            rval.setNum (player->height ());
            break;
        case setsource:
            rval ="false";
            if (args.size ()) {
                KURL url (args.first ());
                if (player->allowRedir (url) && player->openURL (url))
                    rval = "true";
            }
            break;
        case setvolume:
            if (!args.size ())
                return false;
            //?? (args.first ().toInt ());
            rval = "true";
            break;
        case source:
            rval = player->url ().url ();
            break;
        case volume:
            if (player->widget ())
                rval = QString::number (0);
            break;
        default:
            return false;
    }
    return true;
}

KDE_NO_EXPORT void KlashLiveConnectExtension::unregister (const unsigned long) {
}

KDE_NO_EXPORT void KlashLiveConnectExtension::setSize (int w, int h) {
    QCString jscode;
    //jscode.sprintf("this.width=%d;this.height=%d;klash", w, h);
    KParts::LiveConnectExtension::ArgList args;
    args.push_back (qMakePair (KParts::LiveConnectExtension::TypeString, QString("width")));
    args.push_back (qMakePair (KParts::LiveConnectExtension::TypeNumber, QString::number (w)));
    emit partEvent (0, "this.setAttribute", args);
    args.clear();
    args.push_back (qMakePair (KParts::LiveConnectExtension::TypeString, QString("height")));
    args.push_back (qMakePair (KParts::LiveConnectExtension::TypeNumber, QString::number (h)));
    emit partEvent (0, "this.setAttribute", args);
}

//-----------------------------------------------------------------------------

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
static const int XKeyPress = KeyPress;
#undef KeyPress
#undef Always
#undef Never
#undef Status
#undef Unsorted
#undef Bool

KlashEmbed::KlashEmbed (KlashView * view) : QXEmbed (view), m_view (view) {}

KlashEmbed::~KlashEmbed () {}

//-----------------------------------------------------------------------------

KlashView::KlashView (QWidget * parent)
    : QWidget (parent, "kde_klash_part"), m_embed (new KlashEmbed (this)) {}

KlashView::~KlashView () {}

void KlashView::resizeEvent (QResizeEvent *) {
    m_embed->setGeometry (0, 0, width (), height ());
}

WId KlashView::embedId () {
    return m_embed->winId ();
}

#include "klash_part.moc"
