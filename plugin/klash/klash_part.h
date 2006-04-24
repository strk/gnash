// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

// Written by Koos Vriezen <koos ! vriezen ? xs4all ! nl>

#ifndef KLASH_PART_H
#define KLASH_PART_H

#include <qxembed.h>
#include <kprocess.h>
#include <kparts/browserextension.h>

#if __GNUC__ - 0 > 3
#define KLASH_NO_EXPORT __attribute__ ((visibility("hidden")))
#else
#define KLASH_NO_EXPORT
#endif
#define KDE_NO_CDTOR_EXPORT KLASH_NO_EXPORT
#ifndef KDE_NO_EXPORT
#define KDE_NO_EXPORT KLASH_NO_EXPORT
#endif

class KAboutData;
class KlashPart;
class KInstance;
class KProcess;
class JSCommandEntry;
class KlashView;

/*
 * Part notifications to hosting application
 */
class KLASH_NO_EXPORT KlashBrowserExtension : public KParts::BrowserExtension {
    Q_OBJECT
public:
    KlashBrowserExtension(KlashPart *parent);
    KDE_NO_CDTOR_EXPORT ~KlashBrowserExtension () {}
    void urlChanged (const QString & url);
    void setLoadingProgress (int percentage);

    void saveState (QDataStream & stream);
    void restoreState (QDataStream & stream);
    void requestOpenURL (const KURL & url, const QString & target, const QString & service);
};

/*
 * Part javascript support
 */
class KLASH_NO_EXPORT KlashLiveConnectExtension : public KParts::LiveConnectExtension {
    Q_OBJECT
public:
    KlashLiveConnectExtension (KlashPart * parent);
    ~KlashLiveConnectExtension ();

    // LiveConnect interface
    bool get (const unsigned long, const QString &,
            KParts::LiveConnectExtension::Type &, unsigned long &, QString &);
    bool put (const unsigned long, const QString &, const QString &);
    bool call (const unsigned long, const QString &,
            const QStringList &, KParts::LiveConnectExtension::Type &, 
            unsigned long &, QString &);
    void unregister (const unsigned long);
    void sendEvent(const unsigned long objid, const QString & event, const KParts::LiveConnectExtension::ArgList & args ) {
        emit partEvent(objid, event, args);
    }

    void enableFinishEvent (bool b = true) { m_enablefinish = b; }
signals:
    void partEvent (const unsigned long, const QString &,
                    const KParts::LiveConnectExtension::ArgList &);
public slots:
    void setSize (int w, int h);
    void started ();
    void finished ();
private:
    KlashPart * player;
    const JSCommandEntry * lastJSCommandEntry;
    bool m_started : 1;
    bool m_enablefinish : 1;
};

class KLASH_NO_EXPORT KlashEmbed : public QXEmbed {
    KlashView * m_view;
public:
    KlashEmbed (KlashView * parent);
    ~KlashEmbed ();
};

class KLASH_NO_EXPORT KlashView : public QWidget {
    KlashEmbed * m_embed;
public:
    KlashView (QWidget * parent);
    ~KlashView ();
    WId embedId ();
protected:
    void resizeEvent (QResizeEvent *);
};

/*
 * Part that gets created when used a KPart
 */
class KLASH_NO_EXPORT KlashPart : public KParts::ReadOnlyPart {
    Q_OBJECT
    friend struct GroupPredicate;
public:
    KlashPart (QWidget * wparent, const char * wname,
              QObject * parent, const char * name, const QStringList &args);
    ~KlashPart ();

    KDE_NO_EXPORT KlashBrowserExtension * browserextension() const
        { return m_browserextension; }
    KlashLiveConnectExtension * liveconnectextension () const
        { return m_liveconnectextension; }
    bool allowRedir (const KURL & url) const;
    void fullScreen ();
    void setLoaded (int percentage);
    const QString & source () const { return m_src_url; }
public slots:
    virtual bool openURL (const KURL & url);
    virtual bool closeURL ();
    void play ();
    void stop ();
    void pause ();
    int width () const {return m_width; }
    int height () const {return m_height; }
protected slots:
    void playingStarted ();
    void playingStopped ();
    void processStopped (KProcess *);
protected:
    virtual bool openFile();
private:
    KlashBrowserExtension * m_browserextension;
    KlashLiveConnectExtension * m_liveconnectextension;
    KProcess * m_process;
    KURL m_docbase;
    QString m_src_url;
    QString m_file_name;
    int m_width;
    int m_height;
    //bool m_noresize : 1;
    bool m_autostart : 1;
    bool m_fullscreen : 1;
    bool m_started_emited : 1;
};


#endif
