#include "gnash.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

namespace gnash
{

class GtkGlue
{
  public:
    virtual ~GtkGlue() { };
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(GtkWidget *drawing_area) = 0;
    virtual render_handler* createRenderHandler() = 0;
    virtual void render() = 0;
    virtual void configure(GtkWidget *const widget,
                           GdkEventConfigure *const event) = 0;
  protected:
    GtkWidget *_drawing_area;
};

} // namespace gnash
