// gnash-view.cpp: Gtk view widget for gnash
// 
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "gnash-view.h"
#include "gtk_canvas.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkbutton.h>

#include "log.h"

#include "VM.h"
#include "movie_definition.h"
#include "MovieFactory.h"
#include "movie_root.h" // for Abstract callbacks
#include "Renderer.h"
#include "sound_handler.h"
#include "MediaHandler.h"
#include "RunResources.h" // for passing handlers and other data to the core.
#include "VirtualClock.h"
#include "SystemClock.h"
#include "DisplayObject.h"
#include "Movie.h"
#include "Global_as.h"
#include "NamingPolicy.h"
#include "StreamProvider.h"

enum
{
	PROP_0,
	PROP_URI
};

struct _GnashView {
	GtkBin base_instance;

    GnashCanvas *canvas;
    const gchar *uri;
    guint advance_timer;

    std::shared_ptr<gnash::media::MediaHandler> media_handler;
    std::shared_ptr<gnash::sound::sound_handler> sound_handler;

    /// Handlers (for sound etc) for a libcore run.
    //
    /// This must be kept alive for the entire lifetime of the movie_root
    /// (currently: of the Gui).
    std::unique_ptr<gnash::RunResources> run_info;

    boost::intrusive_ptr<gnash::movie_definition> movie_definition;
    gnash::Movie* movie;
    std::unique_ptr<gnash::movie_root> stage;
    std::unique_ptr<gnash::SystemClock> system_clock;
    std::unique_ptr<gnash::InterruptableVirtualClock> virtual_clock;
};

G_DEFINE_TYPE(GnashView, gnash_view, GTK_TYPE_BIN)

static GObjectClass *parent_class = NULL;

static void gnash_view_class_init(GnashViewClass *gnash_view_class);
static void gnash_view_init(GnashView *view);
static void gnash_view_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void gnash_view_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gnash_view_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gnash_view_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gnash_view_realize_cb(GtkWidget *widget, gpointer user_data);

static gboolean key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean key_release_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean button_press_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean button_release_event_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean motion_notify_event_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data);

static gnash::key::code gdk_to_gnash_key(guint key);
static gboolean gnash_view_advance_movie(GnashView *view);
static void gnash_view_display(GnashView *view);
static void gnash_view_load_movie(GnashView *view, const gchar *path);

GtkWidget *
gnash_view_new (void)
{
    return GTK_WIDGET(g_object_new (GNASH_TYPE_VIEW, NULL));
}

const gchar *
gnash_view_call (GnashView *view, const gchar *func_name, const gchar *input_data)
{
    gnash::VM& vm = view->stage->getVM();
	gnash::as_value obj;

    gnash::as_value func = getMember(*getObject(view->movie), getURI(vm, func_name));

    if( !func.is_function() ) {
        return NULL;
    }

    gnash::as_value result;
    if( input_data ) {
        result = callMethod(getObject(view->movie),
                getURI(vm, func_name), gnash::as_value(input_data));
    } else {
        result = callMethod(getObject(view->movie), getURI(vm, func_name));
    }
    if( !result.is_string() ) {
        return NULL;
    }

    return result.to_string().c_str();
}

static void
gnash_view_class_init(GnashViewClass *gnash_view_class)
{
    GNASH_REPORT_FUNCTION;

    parent_class = (GObjectClass *) g_type_class_peek_parent(gnash_view_class);

	GObjectClass *g_object_class = G_OBJECT_CLASS (gnash_view_class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (gnash_view_class);

    widget_class->size_allocate = gnash_view_size_allocate;
    widget_class->size_request = gnash_view_size_request;

	g_object_class->get_property = gnash_view_get_property;
	g_object_class->set_property = gnash_view_set_property;

	g_object_class_install_property (g_object_class,
					 PROP_URI,
					 g_param_spec_string ("uri",
							      "URI to movie",
							      "URI to the SWF movie to display",
							      NULL,
							      (GParamFlags)G_PARAM_READWRITE));
}

static void
gnash_view_set_property (GObject      *object,
		         guint         prop_id,
		         const GValue *value,
		         GParamSpec   *pspec)
{
    GnashView *view = GNASH_VIEW (object);

    switch (prop_id)
    {
    case PROP_URI:
        if(view->movie_definition.get() != NULL) {
            g_warning("Cannot change the movie URI once the view has been initialized.");
            return;
        }
        view->uri = g_strdup(g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gnash_view_get_property (GObject *object,
		         guint prop_id,
		         GValue *value,
		         GParamSpec *pspec)
{
    GnashView *view = GNASH_VIEW (object);

	switch (prop_id)
	{
	case PROP_URI:
		g_value_set_string (value, view->uri);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
gnash_view_init(GnashView *view)
{
    GNASH_REPORT_FUNCTION;

    view->uri = NULL;
    view->advance_timer = 0;

	g_signal_connect (GTK_WIDGET(view), "realize",
	                  G_CALLBACK (gnash_view_realize_cb), NULL);

    // Initializations that can happen before realization come here. The rest
    // come after realize, in gnash_view_realize_cb.
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity(3);

    // Use the default media handler.
    // TODO: allow setting this.
    view->media_handler.reset(gnash::media::MediaFactory::instance().get(""));

    // Init sound
#ifdef SOUND_SDL
    try {
        view->sound_handler.reset(gnash::sound::create_sound_handler_sdl(
                view->media_handler.get()));
    } catch (gnash::SoundException& ex) {
        gnash::log_error(_("Could not create sound handler: %s."
                           " Will continue without sound."), ex.what());
    }
    gnash::log_error(_("Sound requested but no sound support compiled in"));
#endif

    view->canvas = GNASH_CANVAS(gnash_canvas_new());
    std::string nullstr;
    gnash_canvas_setup(view->canvas, nullstr, nullstr, 0, NULL);
    gtk_container_add (GTK_CONTAINER (view), GTK_WIDGET(view->canvas));
    gtk_widget_show (GTK_WIDGET(view->canvas));

    gtk_widget_add_events(GTK_WIDGET(view->canvas), GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_KEY_RELEASE_MASK
                        | GDK_KEY_PRESS_MASK        
                        | GDK_POINTER_MOTION_MASK);

    g_signal_connect_object(GTK_WIDGET(view->canvas), "key-press-event",
                   G_CALLBACK(key_press_event_cb), view, (GConnectFlags)0);
    g_signal_connect_object(GTK_WIDGET(view->canvas), "key-release-event",
                   G_CALLBACK(key_release_event_cb), view, (GConnectFlags)0);
    g_signal_connect_object(GTK_WIDGET(view->canvas), "button-press-event",
                   G_CALLBACK(button_press_event_cb), view, (GConnectFlags)0);
    g_signal_connect_object(GTK_WIDGET(view->canvas), "button-release-event",
                   G_CALLBACK(button_release_event_cb), view, (GConnectFlags)0);
    g_signal_connect_object(GTK_WIDGET(view->canvas), "motion-notify-event",
                   G_CALLBACK(motion_notify_event_cb), view, (GConnectFlags)0);
}

static void
gnash_view_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    GnashView *view = GNASH_VIEW(widget);
    widget->allocation = *allocation;
    gtk_widget_size_allocate (GTK_BIN(widget)->child, allocation);

    if( view->stage.get() != NULL) {
    	view->stage->setDimensions(allocation->width, allocation->height);

        std::shared_ptr<gnash::Renderer> renderer = gnash_canvas_get_renderer(view->canvas);
        float xscale = allocation->width / view->movie_definition->get_width_pixels();
        float yscale = allocation->height / view->movie_definition->get_height_pixels();
		renderer->set_scale(xscale, yscale);
    }
}

static void
gnash_view_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    GnashView *view = GNASH_VIEW(widget);
    if( view->movie_definition.get() == NULL ) {
        requisition->width = 0;
        requisition->height = 0;
    } else {
        requisition->width = view->movie_definition->get_width_pixels();
        requisition->height = view->movie_definition->get_height_pixels();
    }
}

static void
gnash_view_realize_cb(GtkWidget *widget, gpointer /*user_data*/)
{
    GNASH_REPORT_FUNCTION;
    GnashView *view = GNASH_VIEW(widget);

    // Some initializations need to happen after the widget has been realized.
    if(view->movie_definition.get() == NULL) {
        gtk_widget_realize(GTK_WIDGET(view->canvas));
        gnash_view_load_movie(view, view->uri);
    }
}

static gboolean
key_press_event_cb(GtkWidget */*widget*/, GdkEventKey *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    GnashView *view = GNASH_VIEW(data);
    if (view->stage.get() == NULL)
        return FALSE;

    gnash::key::code c = gdk_to_gnash_key(event->keyval);
    
    if (c != gnash::key::INVALID) {
        if( view->stage->keyEvent(c, true) )
            gnash_view_display(view);
        return TRUE;
    }
    
    return FALSE;
}

static gboolean
key_release_event_cb(GtkWidget */*widget*/, GdkEventKey *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    GnashView *view = GNASH_VIEW(data);
    if (view->stage.get() == NULL)
        return FALSE;

    gnash::key::code c = gdk_to_gnash_key(event->keyval);
    
    if (c != gnash::key::INVALID) {
        if( view->stage->keyEvent(c, false) )
            gnash_view_display(view);
        return TRUE;
    }
    
    return FALSE;
}

static gboolean
button_press_event_cb(GtkWidget */*widget*/, GdkEventButton *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    GnashView *view = GNASH_VIEW(data);
    if (view->stage.get() == NULL)
        return FALSE;

    /// Double- and triple-clicks should not send an extra event!
    /// Flash has no built-in double click.
    if (event->type != GDK_BUTTON_PRESS) return FALSE;

    gtk_widget_grab_focus(GTK_WIDGET(view->canvas));

    view->stage->mouseClick(true);

    return TRUE;
}

static gboolean
button_release_event_cb(GtkWidget* /*widget*/, GdkEventButton* /*event*/,
        gpointer data)
{
    GNASH_REPORT_FUNCTION;
    GnashView *view = GNASH_VIEW(data);
    if (view->stage.get() == NULL)
        return FALSE;

    view->stage->mouseClick(false);

    return TRUE;
}

static gboolean
motion_notify_event_cb(GtkWidget */*widget*/, GdkEventMotion *event, gpointer data)
{
    //GNASH_REPORT_FUNCTION;

    GtkWidget *widget = GTK_WIDGET(data);
    GnashView *view = GNASH_VIEW(data);
    float xscale = widget->allocation.width / view->movie_definition->get_width_pixels();
    float yscale = widget->allocation.height / view->movie_definition->get_height_pixels();

	// A stage pseudopixel is user pixel / _xscale wide
	boost::int32_t x = event->x / xscale;

	// A stage pseudopixel is user pixel / _yscale high
	boost::int32_t y = event->y / yscale;

	if ( view->stage->mouseMoved(x, y) )
	{
		// any action triggered by the
		// event required screen refresh
        gnash_view_display(view);
	}

	gnash::DisplayObject* activeEntity = view->stage->getActiveEntityUnderPointer();
	if ( activeEntity )
	{
		if ( activeEntity->isSelectableTextField() )
		{
		    GdkCursor *gdkcursor = gdk_cursor_new(GDK_XTERM);
		    gdk_window_set_cursor (widget->window, NULL);
            gdk_cursor_unref(gdkcursor);
		}
		else if ( activeEntity->allowHandCursor() )
		{
		    GdkCursor *gdkcursor = gdk_cursor_new(GDK_HAND2);
		    gdk_window_set_cursor (widget->window, NULL);
            gdk_cursor_unref(gdkcursor);
		}
		else
		{
		    gdk_window_set_cursor (widget->window, NULL);
		}
	}
	else
	{
	    gdk_window_set_cursor (widget->window, NULL);
	}

    return TRUE;
}

static void
gnash_view_load_movie(GnashView *view, const gchar *uri)
{

    gnash::URL url(uri);

    // The RunResources should be populated before parsing.
    view->run_info.reset(new gnash::RunResources());
    view->run_info->setSoundHandler(view->sound_handler);

    std::unique_ptr<gnash::NamingPolicy> np(new gnash::IncrementalRename(url));
    std::shared_ptr<gnash::StreamProvider> sp(
	    new gnash::StreamProvider(url, url, std::move(np)));
    view->run_info->setStreamProvider(sp);

    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    if ( url.protocol() == "file" ) {
        const std::string& str_path(url.path());

        size_t lastSlash = str_path.find_last_of('/');
        std::string dir = str_path.substr(0, lastSlash + 1);
        rcfile.addLocalSandboxPath(dir);
        gnash::log_debug(_("%s appended to local sandboxes"), dir.c_str());

        rcfile.addLocalSandboxPath(str_path);
        gnash::log_debug(_("%s appended to local sandboxes"), str_path.c_str());
    }

    // Load the actual movie.
    view->movie_definition = gnash::MovieFactory::makeMovie(url,
            *view->run_info, url.str().c_str(), false);

    g_return_if_fail(view->movie_definition.get() != NULL);

    // NOTE: it's important that _systemClock is constructed
    //       before and destroyed after _virtualClock !
    view->system_clock.reset(new gnash::SystemClock());
    view->virtual_clock.reset(new gnash::InterruptableVirtualClock(*view->system_clock));
    view->stage.reset(new gnash::movie_root(*view->virtual_clock, *view->run_info));
    
    view->movie_definition->completeLoad();

    view->advance_timer = g_timeout_add_full(G_PRIORITY_LOW, 10,
            (GSourceFunc)gnash_view_advance_movie, view, NULL);

    gtk_widget_queue_resize (GTK_WIDGET(view));

    //view->movie.reset (view->movie_definition->createMovie());
    
    std::map<std::string, std::string> variables;
    gnash::URL::parse_querystring(url.querystring(), variables);

    gnash::Movie* m = view->stage->init(view->movie_definition.get(), variables);
    view->movie = m;

    view->stage->set_background_alpha(1.0f);

    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
    gnash::sound::sound_handler* s = view->stage->runResources().soundHandler();
    if ( s ) s->unpause();
    
    gnash::log_debug("Starting virtual clock");
    view->virtual_clock->resume();

    gnash_view_advance_movie(view);
}

static gboolean
gnash_view_advance_movie(GnashView *view)
{
    view->stage->advance();

    gnash_view_display(view);

    return TRUE;
}

static void
gnash_view_display(GnashView *view)
{
    gnash::InvalidatedRanges changed_ranges;
    changed_ranges.setWorld();

    std::shared_ptr<gnash::Renderer> renderer = gnash_canvas_get_renderer(view->canvas);
    renderer->set_invalidated_regions(changed_ranges);
    gdk_window_invalidate_rect(GTK_WIDGET(view->canvas)->window, NULL, false);

    gnash_canvas_before_rendering(view->canvas, view->stage.get());
	view->stage->display();

    gdk_window_process_updates(GTK_WIDGET(view->canvas)->window, false);
}

static gnash::key::code
gdk_to_gnash_key(guint key)
{
    gnash::key::code c(gnash::key::INVALID);

    // ascii 32-126 in one range:    
    if (key >= GDK_space && key <= GDK_asciitilde) {
        c = (gnash::key::code) ((key - GDK_space) + gnash::key::SPACE);
    }

    // Function keys:
    else if (key >= GDK_F1 && key <= GDK_F15)	{
        c = (gnash::key::code) ((key - GDK_F1) + gnash::key::F1);
    }

    // Keypad:
    else if (key >= GDK_KP_0 && key <= GDK_KP_9) {
        c = (gnash::key::code) ((key - GDK_KP_0) + gnash::key::KP_0);
    }

    // Extended ascii:
    else if (key >= GDK_nobreakspace && key <= GDK_ydiaeresis) {
        c = (gnash::key::code) ((key - GDK_nobreakspace) + 
                gnash::key::NOBREAKSPACE);
    }

    // non-character keys don't correlate, so use a look-up table.
    else {
        struct {
            guint             gdk;
            gnash::key::code  gs;
        } table[] = {
            { GDK_BackSpace, gnash::key::BACKSPACE },
            { GDK_Tab, gnash::key::TAB },
            { GDK_Clear, gnash::key::CLEAR },
            { GDK_Return, gnash::key::ENTER },
            
            { GDK_Shift_L, gnash::key::SHIFT },
            { GDK_Shift_R, gnash::key::SHIFT },
            { GDK_Control_L, gnash::key::CONTROL },
            { GDK_Control_R, gnash::key::CONTROL },
            { GDK_Alt_L, gnash::key::ALT },
            { GDK_Alt_R, gnash::key::ALT },
            { GDK_Caps_Lock, gnash::key::CAPSLOCK },
            
            { GDK_Escape, gnash::key::ESCAPE },
            
            { GDK_Page_Down, gnash::key::PGDN },
            { GDK_Page_Up, gnash::key::PGUP },
            { GDK_Home, gnash::key::HOME },
            { GDK_End, gnash::key::END },
            { GDK_Left, gnash::key::LEFT },
            { GDK_Up, gnash::key::UP },
            { GDK_Right, gnash::key::RIGHT },
            { GDK_Down, gnash::key::DOWN },
            { GDK_Insert, gnash::key::INSERT },
            { GDK_Delete, gnash::key::DELETEKEY },
            
            { GDK_Help, gnash::key::HELP },
            { GDK_Num_Lock, gnash::key::NUM_LOCK },

            { GDK_VoidSymbol, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].gdk != GDK_VoidSymbol; i++) {
            if (key == table[i].gdk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    return c;
}

#if 0 // UNUSED
 static int gdk_to_gnash_modifier(int state); 
static int
gdk_to_gnash_modifier(int state)
{
    int modifier = gnash::key::GNASH_MOD_NONE;

    if (state & GDK_SHIFT_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & GDK_CONTROL_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if (state & GDK_MOD1_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    return modifier;
}
#endif

