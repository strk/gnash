#include "gnash.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <qapplication.h>
#include <qgl.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qmessagebox.h>

namespace gnash
{

class KdeGlue
{
  public:
    virtual ~KdeGlue() { };
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(QGLWidget *drawing_area) = 0;
    virtual render_handler* createRenderHandler() = 0;
    virtual void render() = 0;
  protected:
    QGLWidget     *_drawing_area;
};

} // namespace gnash
