#ifndef __ROSUP_H__
#define __ROSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
#include "tu_config.h"

#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#ifdef RENDERER_AGG
#include "ro_glue_agg.h"
#endif

#include "gui.h"

namespace gnash
{

class DSOEXPORT RiscosGui : public Gui
{
 public:
    RiscosGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~RiscosGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool createWindow(const char *title, int width, int height);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    virtual void set_invalidated_region(const rect& bounds);


 private:
    bool create_window();
    int valid_coord(int coord, int max);

    wimp_t _task;
    wimp_w _window;
    bool _quit;
    os_t _timeout;

    int m_draw_minx;
    int m_draw_miny;
    int m_draw_maxx;
    int m_draw_maxy;

    int _screen_height;
    int _screen_width;

#ifdef RENDERER_AGG
    RiscosAggGlue glue;
#endif
};

}

#endif
