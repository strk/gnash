// klash.cpp:  KDE flash player plugin, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/* $Id: klash.cpp,v 1.28 2007/06/15 15:00:26 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <unistd.h>
#include <cstdlib> //For exit()
#include <cstdio>
#include <vector>
#include <sys/time.h>
#include <ctime>

#include <qapplication.h>
#include <qgl.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qmessagebox.h>

#include "gnash.h"
#include "log.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "xmlsocket.h"
#include "movie_definition.h"
#include "sprite_instance.h"
#include "movie_root.h"
#include "URL.h"
#include "rc.h"

using namespace std;
using namespace gnash;

static void usage ();
static void version_and_copyright();

int xml_fd;                     // FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.

#define OVERSIZE	1.0f

static int doneYet = 0;

static float	s_scale = 1.0f;
//static bool	s_antialiased = false;
//static bool	s_start_waiting = false;
static int	s_bit_depth = 16;
static bool	s_background = true;

//static QGLContext *qglcontext;
typedef enum {IDLE_MOVIE, PLAY_MOVIE, RESTART_MOVIE, PAUSE_MOVIE, STOP_MOVIE, STEP_FORWARD, STEP_BACKWARD, JUMP_FORWARD, JUMP_BACKWARD, QUIT_MOVIE} movie_state_e;

movie_state_e movie_menu_state;

int mouse_x;
int mouse_y;
int mouse_buttons;

int width;
int height;

unsigned long windowid = 0;

class EmbedWidget : public QGLWidget
{
    Q_OBJECT
public:
    EmbedWidget (WId embed) {
        create (embed);
        qwidget = this;
        qmenu = new QPopupMenu(this);
        qmenu->insertItem(_("Play Movie"), this, SLOT(menuitem_play_callback()));
        qmenu->insertItem(_("Pause Movie"), this, SLOT(menuitem_pause_callback()));
        qmenu->insertItem(_("Stop Movie"), this, SLOT(menuitem_stop_callback()));
        qmenu->insertItem(_("Restart Movie"), this, SLOT(menuitem_restart_callback()));
        qmenu->insertItem(_("Step Forward"), this, SLOT(menuitem_step_forward_callback()));
        qmenu->insertItem(_("Step Backward"), this, SLOT( menuitem_step_backward_callback()));
        qmenu->insertItem(_("Jump Forward"), this, SLOT(menuitem_jump_forward_callback()));
        qmenu->insertItem(_("Jump Backward"), this, SLOT(menuitem_jump_backward_callback()));
        qmenu->insertItem(_("Quit Gnash"), this, SLOT(menuitem_quit_callback()));
        
//        qmenu->insertItem("&About", this, SLOT(about()), CTRL+Key_H);
    }
    
public slots:
    void menuitem_restart_callback();
    void menuitem_quit_callback();
    void menuitem_play_callback();
    void menuitem_pause_callback();
    void menuitem_stop_callback();
    void menuitem_step_forward_callback();
    void menuitem_step_backward_callback();
    void menuitem_jump_forward_callback();
    void menuitem_jump_backward_callback();
    void about();
    
protected:
    void resizeEvent( QResizeEvent * );
    void contextMenuEvent(QContextMenuEvent *event);
signals:
    void explain( const QString& );
private:
    QPopupMenu *qmenu;
    QGLWidget  *qwidget;
};

#include "klash.moc"

void
EmbedWidget::contextMenuEvent(QContextMenuEvent*)
{
//    printf("Got Right Click!\n");
    qmenu->exec();
}

void EmbedWidget::about()
{
    QMessageBox::about( this, _("Klash"),
                        _("The Gnash Flash player for KDE.\n"));
}

void 
EmbedWidget::menuitem_restart_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = RESTART_MOVIE;
}
void 
EmbedWidget::menuitem_quit_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = QUIT_MOVIE;
}
void 
EmbedWidget::menuitem_play_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = PLAY_MOVIE;
}
void 
EmbedWidget::menuitem_pause_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = PAUSE_MOVIE;
}
void 
EmbedWidget::menuitem_stop_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STOP_MOVIE;
}
void 
EmbedWidget::menuitem_step_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STEP_FORWARD;
}
void 
EmbedWidget::menuitem_step_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STEP_BACKWARD;
}
void 
EmbedWidget::menuitem_jump_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = JUMP_FORWARD;
}
void 
EmbedWidget::menuitem_jump_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = JUMP_BACKWARD;
}

void EmbedWidget::resizeEvent(QResizeEvent *event)
{
//    GNASH_REPORT_FUNCTION;
//    qwidget->resize(width, height);
//    this->resize(width, height);
}

// Registering a file opener has been obsoleted
#if 0
static tu_file*
file_opener(const char* url)
// Callback function.  This opens files for the library.
{
//    GNASH_REPORT_FUNCTION;

    if (strcmp(url, "-") == 0) {
        FILE *newin = fdopen(dup(0),"rb");
        return new tu_file(newin, false);
    } else {
        return new tu_file(url, "rb");
    }
}
#endif
// 0

static void
fs_callback(gnash::sprite_instance* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    log_msg("fs_callback: '");
    log_msg(command);
    log_msg("' '");
    log_msg(args);
    log_msg("'\n");
}

int
main(int argc, char *argv[])
{
    int c;
    int render_arg;
    std::vector<const char*> infiles;
    string url;
    QApplication *app = new QApplication (argc, argv);
    QGLWidget *qwidget;
    assert(tu_types_validate());
    
    float	exit_timeout = 0;
    bool        do_render = true;
    bool        do_sound = true;
    bool        do_loop = true;
    bool        sdl_abort = true;
    int         delay = 31;
    float	tex_lod_bias;
    struct timeval now;
    static struct timeval start;

    unsigned int ticks;
    
    // -1.0 tends to look good.
    tex_lod_bias = -1.2f;
    
    // scan for the two main long GNU options
    for (c=0; c<argc; c++) {
        if (strcmp("--help", argv[c]) == 0) {
            version_and_copyright();
            printf("\n");
            usage();
            exit(0);
        }
        if (strcmp("--version", argv[c]) == 0) {
            version_and_copyright();
            exit(0);
        }
    }

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(false);
    
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    rcfile.loadFiles();
//    rcfile.dump();

    if (rcfile.useWriteLog()) {
        dbglogfile.setWriteDisk(true);
    }
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }
    
    if (rcfile.useActionDump()) {
        dbglogfile.setActionDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.useParserDump()) {
        dbglogfile.setParserDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.getTimerDelay() > 0) {
        delay = rcfile.getTimerDelay();
        log_msg (_("Timer delay set to %d milliseconds"), delay);
    }

    while ((c = getopt (argc, argv, "hvaps:cd:m:x:r:t:b:1wj:k:u:")) != -1) {
	switch (c) {
	  case 'h':
	      usage ();
	      break;
	  case 'v':
              dbglogfile.setVerbosity();
	      log_msg (_("Verbose output turned on"));
	      break;
	  case 'w':
              dbglogfile.setWriteDisk(true);
	      log_msg (_("Logging to disk enabled."));
	      break;
	  case 'a':
	      dbglogfile.setActionDump(true);              
	      break;
	  case 'p':
	      dbglogfile.setParserDump(true);
	      break;
          case 's':
              s_scale = fclamp((float) atof(optarg), 0.01f, 100.f);
              break;
          case 'c':
              sdl_abort = false;
              break;
          case 'd':
              delay = strtol(optarg, NULL, 0);
              break;
          case 'u':
              url = optarg;
              log_msg(_("Setting root URL to: %s"), url);
              break;
          case 'j':
              width = strtol(optarg, NULL, 0);
              log_msg (_("Setting width to: %d", width);
              break;
          case 'k':
              height = strtol(optarg, NULL, 0);
              log_msg (_("Setting height to: %d", height);
              break;
          case 'x':
              windowid = strtol(optarg, NULL, 0);
              break;
          case 'm':
              tex_lod_bias = (float) atof(optarg);
              break;
          case '1':
              do_loop = false;
              break;
          case 'r':
              render_arg = strtol(optarg, NULL, 0);
              switch (render_arg) {
                case 0:
                    // Disable both
                    do_render = false;
                    do_sound = false;
                    break;
                case 1:
                    // Enable both
                    do_render = true;
                    do_sound = true;
                    break;
                case 2:
                    // Disable just sound
                    do_render = true;
                    do_sound = false;
                    break;
                default:
                    log_error (
		        _("-r must be followed by 0, 1 or 2 (%d is invalid)"),
                        render_arg);
                    break;
              };
              break;
          case 't':
              exit_timeout = (float) atof(optarg);
              break;
          case 'b':
              s_bit_depth = atoi(optarg);
              break;
	}
    }
    
    // get the file name from the command line
    while (optind < argc) {
        // Some options set variables, like ip=127.0.0.1
        if (strchr(argv[optind], '=')) {
            log_error (_("Got variable option on command line!");
        } else {
            infiles.push_back(argv[optind]);
        }
	optind++;
    }

    // Remove the logfile that's created by default, since leaving a short
    // file is confusing.
    if (dbglogfile.getWriteDisk() == false) {
        dbglogfile.removeLog();
    }

    // No file names were supplied
    if (infiles.size() == 0) {
	printf(_("No input files"));
	usage();
	exit(1);
    }

    // registering a file_opener has been obsoleted
    //gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);
    
    gnash::sound_handler  *sound = NULL;
    gnash::render_handler *render = NULL;
    if (do_render) {
        if (do_sound) {
#ifdef SOUND_SDL
            sound = gnash::create_sound_handler_sdl();
            gnash::set_sound_handler(sound);
#endif
#ifdef SOUND_GST
            sound = gnash::create_sound_handler_gst();
            gnash::set_sound_handler(sound);
#endif
        }
        render = gnash::create_render_handler_ogl();
        gnash::set_render_handler(render);
    }
    
    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float	movie_fps = 30.0f;
    gnash::get_movie_info(URL(infiles[0]), &movie_version, &movie_width,
                          &movie_height, &movie_fps, NULL, NULL);
    if (movie_version == 0) {
        fprintf(stderr, _("error: can't get info about %s\n"), infiles[0]);
        exit(1);
    }

    if (!width) {    
        width = int(movie_width * s_scale);
    }
    if (!height) {
        height = int(movie_height * s_scale);
    }
    
    if (do_render) {
        qwidget = new EmbedWidget (windowid);
        qwidget->makeCurrent();
        qwidget->resize(width, height);
        qwidget->show();

        // Change the LOD BIAS values to tweak blurriness.
        if (tex_lod_bias != 0.0f) {
#ifdef FIX_I810_LOD_BIAS	
            // If 2D textures weren't previously enabled, enable
            // them now and force the driver to notice the update,
            // then disable them again.
            if (!glIsEnabled(GL_TEXTURE_2D)) {
                // Clearing a mask of zero *should* have no
                // side effects, but coupled with enbling
                // GL_TEXTURE_2D it works around a segmentation
                // fault in the driver for the Intel 810 chip.
                glEnable(GL_TEXTURE_2D);
                glClear(0);
                glDisable(GL_TEXTURE_2D);
            }
#endif // FIX_I810_LOD_BIAS
            glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, tex_lod_bias);
        }
        
        // Turn on alpha blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Turn on line smoothing.  Antialiased lines can be used to
        // smooth the outsides of shapes.
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE
        
        glMatrixMode(GL_PROJECTION);
        glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // We don't need lighting effects
        glDisable(GL_LIGHTING);
        // glColorPointer(4, GL_UNSIGNED_BYTE, 0, *);
        // glInterleavedArrays(GL_T2F_N3F_V3F, 0, *)
        glPushAttrib (GL_ALL_ATTRIB_BITS);		
    }
    
    // Load the actual movie.
    gnash::movie_definition*	md = gnash::create_library_movie(URL(infiles[0]));
    if (md == NULL) {
        fprintf(stderr, _("error: can't create a movie from '%s'\n"), infiles[0]);
        exit(1);
    }
    gnash::sprite_instance* m = create_library_movie_inst(md);
    if (m == NULL) {
        fprintf(stderr, _("error: can't create movie instance\n"));
        exit(1);
    }
    gnash::set_current_root(m);
    
    // Mouse state.
    float	speed_scale = 1.0f;
    unsigned int start_ticks = 0;
    gettimeofday(&start, NULL);
    if (do_render) {
        gettimeofday(&now, NULL);
	start_ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;        
    }
    unsigned int last_ticks = start_ticks;
    int	frame_counter = 0;
    int	last_logged_fps = last_ticks;    
    
    for (;;) {
        uint32_t	ticks;
        if (do_render) {
            gettimeofday(&now, NULL);
            ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;      
        } else {
            // Simulate time.
            ticks = last_ticks + (uint32_t) (1000.0f / movie_fps);
        }
        int	delta_ticks = ticks - last_ticks;
        float	delta_t = delta_ticks / 1000.f;
        last_ticks = ticks;
        
        // Check auto timeout counter.
        if (exit_timeout > 0
            && ticks - start_ticks > (uint32_t) (exit_timeout * 1000)) {
            // Auto exit now.
            break;
        }
        
        if (do_render) {
        }

//    printf("%s(%d): Frame count is %d\n", __PRETTY_FUNCTION__, __LINE__,
//           md->get_frame_count());
//        movie_menu_state = IDLE_MOVIE;
        QApplication::eventLoop()->processEvents(QEventLoop::AllEvents, 3);
        switch (movie_menu_state) {
          case PLAY_MOVIE:
              m->set_play_state(gnash::sprite_instance::PLAY);
              break;
              // Control-R restarts the movie
          case RESTART_MOVIE:
              m->restart();
              break;
          case STOP_MOVIE:
              m->set_play_state(gnash::sprite_instance::STOP);
              break; 
          case PAUSE_MOVIE:
              if (m->get_play_state() == gnash::sprite_instance::STOP) {
                  m->set_play_state(gnash::sprite_instance::PLAY);
              } else {
                  m->set_play_state(gnash::sprite_instance::STOP);
              }
              break;
              // go backward one frame
          case STEP_BACKWARD:
              m->goto_frame(m->get_current_frame()-1);                
              break;
              // go forward one frame
          case STEP_FORWARD:
              m->goto_frame(m->get_current_frame()+1);
              break;
              // jump goes backward 10 frames
          case JUMP_BACKWARD:
              m->goto_frame(m->get_current_frame()-10);
              break;
              // jump goes forward 10 frames
          case JUMP_FORWARD:
              if ((m->get_current_frame()+10) < md->get_frame_count()) {
                  m->goto_frame(m->get_current_frame()+10);
              }
              break;
          case QUIT_MOVIE:
              goto done;
              break;
          default:
              break;
        };
        m = gnash::get_current_root();
        
	movie_root* root = dynamic_cast<movie_root*>(m);
	assert(root);
        root->set_display_viewport(0, 0, width, height);
        root->set_background_alpha(s_background ? 1.0f : 0.05f);
        
        root->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);
        
        m->advance(delta_t *speed_scale);

        if (do_render) {
            glDisable(GL_DEPTH_TEST);	// Disable depth testing.
            glDrawBuffer(GL_BACK);
        }
        m->display();
        frame_counter++;

        qwidget->swapBuffers();
        
        // See if we should exit.
        if (do_loop == false
            && m->get_current_frame() + 1 == md->get_frame_count()) {
            // We're reached the end of the movie; exit.
            break;
        }
    }

  done:
    
    doneYet = 1;
    
    if (md) {
        md->drop_ref();
    }
    
    if (m) {
        m->drop_ref();
    }
    delete sound;
    delete render;
    
    // For testing purposes, throw some keypresses
    // to make sure the key handler is properly using weak
    // references to listeners.
    gnash::notify_key_event(gnash::key::A, true);
    gnash::notify_key_event(gnash::key::B, true);
    gnash::notify_key_event(gnash::key::C, true);
    
    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();
    
    return 0;
}


void
version_and_copyright()
{
    printf (_(
"Gnash " VERSION "\n"
"Copyright (C) 2006, 2007 Free Software Foundation, Inc.\n"
"Gnash comes with NO WARRANTY, to the extent permitted by law.\n"
"You may redistribute copies of Gnash under the terms of the GNU General\n"
"Public License.  For more information, see the file named COPYING.\n"
	));
}


void
usage()
{
    printf("%s%s%s", _(
        "usage: gnash [options] movie_file.swf\n"
        "\n"
        "Plays a SWF (Shockwave Flash) movie\n"
        "options:\n"
        "\n"
        "  -h, --help  Print this info.\n"
        "  -s <factor> Scale the movie up/down by the specified factor\n"
        "  -c          Produce a core file instead of letting SDL trap it\n"
        "  -d num      Number of milliseconds to delay in main loop\n"
        "  -v          Be verbose; i.e. print log messages to stdout\n"
        "  -va         Be verbose about movie Actions\n"
        "  -vp         Be verbose about parsing the movie\n"), _(
        "  -m <bias>   Specify the texture LOD bias (float, default is -1)\n"
        "  -x <ID>     X11 Window ID for display\n"
        "  -w          Produce the disk based debug log\n"
        "  -1          Play once; exit when/if movie reaches the last frame\n"
        "  -r <0|1|2>  0 disables rendering & sound (good for batch tests)\n"
        "              1 enables rendering & sound (default setting)\n"
        "              2 enables rendering & disables sound\n"
        "  -t <sec>    Timeout and exit after the specified number of seconds\n"
        "  -b <bits>   Bit depth of output window (16 or 32, default is 16)\n"
        "  --version   Print gnash's version number and exit\n"
        "\n"), _(
        "keys:\n"
        "  CTRL-Q, CTRL-W, ESC   Quit/Exit\n"
        "  CTRL-P          Toggle Pause\n"
        "  CTRL-R          Restart the movie\n"
        "  CTRL-[ or kp-   Step back one frame\n"
        "  CTRL-] or kp+   Step forward one frame\n"
//        "  CTRL-A          Toggle antialiasing (doesn't work)\n"
//        "  CTRL-T          Debug.  Test the set_variable() function\n"
//        "  CTRL-G          Debug.  Test the get_variable() function\n"
//        "  CTRL-M          Debug.  Test the call_method() function\n"
        "  CTRL-B          Toggle background color\n"
        ));
}
