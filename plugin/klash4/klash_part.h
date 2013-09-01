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

#ifndef KLASH_PART_H
#define KLASH_PART_H

#include <QX11EmbedWidget>
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
struct JSCommandEntry;
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
    void requestOpenUrl (const KUrl & url, const QString & target, const QString & service);
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

class KLASH_NO_EXPORT KlashEmbed : public QX11EmbedWidget {
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
    KlashPart (QWidget * wparent, QObject * parent, const QStringList &args);
    ~KlashPart ();

    KDE_NO_EXPORT KlashBrowserExtension * browserextension() const
        { return m_browserextension; }
    KlashLiveConnectExtension * liveconnectextension () const
        { return m_liveconnectextension; }
    bool allowRedir (const KUrl & url) const;
    void fullScreen ();
    void setLoaded (int percentage);
    const QString & source () const { return m_src_url; }
public slots:
    virtual bool openUrl (const KUrl & url);
    virtual bool closeUrl ();
    void play ();
    void stop ();
    void pause ();
    int width () const {return m_width; }
    int height () const {return m_height; }
protected slots:
    void playingStarted ();
    void playingStopped ();
    void processStopped (int, QProcess::ExitStatus);
protected:
    virtual bool openFile();
private:
    QStringList m_args;
    KlashBrowserExtension * m_browserextension;
    KlashLiveConnectExtension * m_liveconnectextension;
    KProcess * m_process;
    KUrl m_docbase;
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
