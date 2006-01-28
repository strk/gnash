// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GL/gl.h>              // Header File For The OpenGL32 Library
#include <GL/glu.h>             // Header File For The GLu32 Library

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ogl.h"
#include "ogl_sdl.h"

// This is our SDL surface
SDL_Surface *surface = NULL;
SDL_cond *cond = NULL;
SDL_mutex *mutex = NULL;
SDL_PixelFormat *pixelformat = NULL;

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a)<0) ? -(a) : (a))
#define sign(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

static void line8(SDL_Surface *s, int x1, int y1, 
                  int x2, int y2, Uint32 color);
static void line16(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color);
static void line24(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color);
static void line32(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color);
static void line(SDL_Surface *s, 
                 int x1, int y1, 
                 int x2, int y2, 
                 Uint32 color);


const GLfloat s_scale = 1.0f;
const int s_bit_depth = 16;
const GLfloat tex_lod_bias = -1.2f;
const GLfloat OVERSIZE = 1.0f;

// lighting on/off (1 = on, 0 = off)
bool light;
// L pressed (1 = yes, 0 = no)
bool lp;
// F pressed (1 = yes, 0 = no)
bool fp;

GLfloat	xrot;                   // X Rotation
GLfloat	yrot;                   // Y Rotation
GLfloat xspeed;                 // X Rotation Speed
GLfloat yspeed;                 // Y Rotation Speed
GLfloat	z=-5.0f;                // Depth Into The Screen

// white ambient light at half intensity (rgba)
GLfloat LightAmbient[]=		{ 0.5f, 0.5f, 0.5f, 1.0f };
// super bright, full intensity diffuse light.
GLfloat LightDiffuse[]=		{ 1.0f, 1.0f, 1.0f, 1.0f };
// position of light (x, y, z, (position of light))
GLfloat LightPosition[]=	{ 0.0f, 0.0f, 2.0f, 1.0f };

GLuint	texture[3];             // Storage for 3 textures.
GLuint  blend;                  // Turn blending on/off
GLuint	filter;                 // Which Filter To Use

// sweepLine animates a line on a surface based on the
// elapsed time.

class sweepLine
{
private:
    SDL_Surface *s;             // The surface to draw on.
    Uint32 color;               // The color of the line.
    int last;                   // last time update() was
    // called.
    int maxx;                   // Maximum valid X value.
    int maxy;                   // Maximum valid Y value.
    float x1, y1;               // The current location
    float dx1, dy1;             // and velocity of the line
    float x2, y2;               // end points.
    float dx2, dy2;

    // movePoint computes the new location of a point based
    // on its initial location, its velocity, and the
    // elapsed time.

    void movePoint(float &x, float &y, 
                   float &dx, float &dy,
                   int dt)
        {
            // Compute the new X location.

            x += (dx * dt);

            // if the X value is off of the screen, move it back
            // on and reverse the velocity in the X direction.

            if (x >= maxx)
                {
                    x = maxx;
                    dx = -dx;
                }
            else if (x <= 0)
                {
                    x = 0;
                    dx = -dx;
                }

            // Same thing for Y.
            y += (dy * dt);
            if (y >= maxy)
                {
                    y = maxy;
                    dy = -dy;
                }
            else if (y <= 0)
                {
                    y = 0;
                    dy = -dy;
                }
        }

public:

    // sweepLine animates a line on a surface. It is
    // initialized with a pointer to the surface to draw the
    // line on, a pixel value that specifies the color of
    // the line, the current time, and the initial locations
    // of the line end points and their
    // velocities. Velocities are specified in
    // pixels/millisecond.

    // This method initializes the class and forces the end
    // points of the lines to be inside the boundaries of
    // the surface. If it didn't do that the line drawing
    // code would try to write outside of the surface and
    // crash the program.

    sweepLine(SDL_Surface *s, 
              Uint32 color,
              int time,
              float x1,  float y1,
              float dx1, float dy1,
              float x2,  float y2,
              float dx2, float dy2): 
        s(s),
        color(color),
        last(time),
        x1(x1), y1(y1),
        dx1(dx1), dy1(dy1),
        x2(x2), y2(y2),
        dx2(dx2), dy2(dy2)
        {

            // Set the values of maxx and maxy to one less than
            // the width and height. Do this makes clipping easier
            // to code.

            maxx = 0;
            maxy = 0;

            if (NULL != s)
                {
                    maxx = s->w - 1;
                    maxy = s->h - 1;
                }

            // Force the line end points onto the screen.

            x1 = max(x1, 0);
            y1 = max(y1, 0);

            x2 = max(x2, 0);
            y2 = max(y2, 0);

            x1 = min(x1, maxx);
            y1 = min(y1, maxy);

            x2 = min(x2, maxx);
            y2 = min(y2, maxy);
        }

    void update(long now)
        {
            int dt = now - last;
            last = now;

            // Update the locations of the line end points.

            movePoint(x1, y1, dx1, dy1, dt);
            movePoint(x2, y2, dx2, dy2, dt);

            // Draw the line at its new location.

            line(s, 
                 (int)x1, (int)y1, 
                 (int)x2, (int)y2, 
                 color);
        }

};

// gameTime keeps track of game time as opposed to real
// time. Game time can start and stop and even change its
// speed while real time just keeps ticking along.

class gameTime
{
private:
    int startTime;              // Last time the clock was
    // started.
    int baseTime;               // How much game time passed
    // before the last time the
    // clock was started.
    bool running;               // Is the clock running or
    // not?

public:

    // Initialize the class variables. At this point no game
    // time has elapsed and the clock is not running.

    gameTime()
        {
            startTime = 0;
            baseTime = 0;
            running = false;
        }

    // Start the clock.

    void start()
        {
            if (!running)
                {
                    startTime = SDL_GetTicks();
                    running = true;
                }
        }

    // stop the clock

    void stop()
        {
            if (running)
                {
                    baseTime = baseTime + (SDL_GetTicks() - startTime);
                    running = false;
                }
        }

    // True if the clock is paused.

    bool stopped()
        {
            return !running;
        }

    // Get this clocks current time in milliseconds.

    int time()
        {
            if (running)
                {
                    return baseTime + (SDL_GetTicks() - startTime);
                }
            else
                {
                    return baseTime;
                }
        }
};

// function to release/destroy our resources and restoring the old desktop
void
Quit( int returnCode )
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);

    // clean up the window
    SDL_Quit( );

    // and exit appropriately
    exit( returnCode );
}

// function to reset our viewport after a window resize 
int
resizeWindow( int width, int height )
{
    printf("%s(%d): Width = %d, Height = %d\n", __PRETTY_FUNCTION__, __LINE__, width, height);

    // Height / width ration 
    GLfloat ratio;
  
    // Protect against a divide by zero 
    if ( height == 0 )
        height = 1;
  
    ratio = ( GLfloat )width / ( GLfloat )height;
  
    // Setup our viewport. 
    glViewport( 0, 0, ( GLint )width, ( GLint )height );
  
    // change to the projection matrix and set our viewing volume. 
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
  
    // Set our perspective 
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
  
    // Make sure we're changing the model view and not the projection 
    glMatrixMode( GL_MODELVIEW );
  
    // Reset The View 
    glLoadIdentity( );
  
    return( true );
}

// function to handle key press events 
void
handleKeyPress( SDL_keysym *keysym )
{
    printf("%s: \n", __PRETTY_FUNCTION__);
  
    switch ( keysym->sym ) {
      case SDLK_ESCAPE:
          // ESC key was pressed 
          Quit( 0 );
          break;
      case SDLK_b:
          // 'b' key was pressed this toggles blending
     
          blend = !blend;
          if ( blend ) {
              glEnable( GL_BLEND );
              glDisable( GL_DEPTH_TEST );
          } else {
              glDisable( GL_BLEND );
              glEnable( GL_DEPTH_TEST );
          }
          break;
      case SDLK_f:
          // 'f' key was pressed this pages through the different filters
     
          filter = ( ++filter ) % 3;
          break;
      case SDLK_l:
          // 'l' key was pressed this toggles the light
	     
          light = !light;
          if ( !light )
              glDisable( GL_LIGHTING );
          else
              glEnable( GL_LIGHTING );
          break;
      case SDLK_PAGEUP:
          printf("SDL Key Page Up\n");
          // PageUp key was pressed this zooms into the scene
     
          z -= 0.02f;
          break;
      case SDLK_PAGEDOWN:
          printf("SDL Key Page Down\n");
          // PageDown key was pressed this zooms out of the scene
     
          z += 0.02f;
          break;
      case SDLK_UP:
          printf("SDL Key Arrow Up\n");
          // Up arrow key was pressed this affects the x rotation
     
          xspeed -= 0.01f;
          break;
      case SDLK_DOWN:
          printf("SDL Key Arrow Up\n");
          // Down arrow key was pressed this affects the x rotation
     
          xspeed += 0.01f;
          break;
      case SDLK_RIGHT:
          printf("SDL Key Arrow Right\n");
          // Right arrow key was pressed this affects the y rotation
     
          yspeed += 0.01f;
          break;
      case SDLK_LEFT:
          printf("SDL Key Arrow Left\n");
          // Left arrow key was pressed this affects the y rotation
     
          yspeed -= 0.01f;
          break;
#if 0
      case SDLK_F1:
          // 'f' key was pressed this toggles fullscreen mode
     
          SDL_WM_ToggleFullScreen( surface );
          break;
#endif
      default:
          break;
    }

    return;
}

// general OpenGL initialization function
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=02
int
initGL(nsPluginInstance *winst)
{
    int depth, width, height, id;
    
    // Flags to pass to SDL_SetVideoMode
    int videoFlags;
    // this holds some info about our display
    const SDL_VideoInfo *videoInfo;

    depth = winst->getDepth();
    width = winst->getWidth();
    height = winst->getHeight();
    id = winst->getWindow();
    
    printf("%s(%d) id is %d,\n\t width is %d, height is %d, depth is %d\n",
           __PRETTY_FUNCTION__, __LINE__,
           id, width, height, depth);
    
    char SDL_windowhack[32];
    sprintf (SDL_windowhack,"SDL_WINDOWID=%ld", id);
    putenv (SDL_windowhack);
    
    SDL_mutexP(mutex);

    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
        printf("Can't initialize SDL\n");
        exit(1);
    }
    
    // Safety first. If the program exits in an unexpected
    // way the atexit() call should ensure that SDL will be
    // shut down properly and the screen returned to a
    // reasonable state.
    //atexit(SDL_Quit);

#if 0
    // 16-bit color, surface creation is likely to succeed.
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
#endif
//    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8 );
    // Sets up OpenGL double buffering
    
    // Fetch the video info
    videoInfo = SDL_GetVideoInfo( );
    
    if ( !videoInfo ) {
        fprintf( stderr, "Video query failed: %s\n",
                 SDL_GetError( ) );
        Quit( 1 );
    }

    // the flags to pass to SDL_SetVideoMode
    videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    videoFlags |= SDL_GL_DOUBLEBUFFER;
    videoFlags |= SDL_RESIZABLE;       // Enable window resizing
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
//     videoFlags |= SDL_ANYFORMAT;
    
    // This checks to see if surfaces can be stored in memory
    if ( videoInfo->hw_available ) {
        videoFlags |= SDL_HWSURFACE;
    } else {
        videoFlags |= SDL_SWSURFACE;
    }

    // This checks if hardware blits can be done
    if ( videoInfo->blit_hw ) {
        videoFlags |= SDL_HWACCEL;
    }    
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    surface = SDL_SetVideoMode(width, height,
                               16,
                               videoFlags);
    
    // Grab the pixel format for the screen. SDL_MapRGB()
    // needs the pixel format to create pixels that are laid
    // out correctly for the screen.    
    pixelformat = surface->format;

    // resize the initial window
    resizeWindow(width, height);
    
    // Enable smooth shading 
    glShadeModel( GL_SMOOTH );
  
    // Set the background black 
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  
    // Depth buffer setup 
    glClearDepth( 1.0f );
  
    // Enables Depth Testing 
    glEnable( GL_DEPTH_TEST );
  
    // The Type Of Depth Test To Do 
    glDepthFunc( GL_LEQUAL );

    // Really Nice Perspective Calculations 
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format,0,0,0,0));
    // resize the initial window
    resizeWindow(width, height);

    SDL_mutexV(mutex);
  
    return( true );
}

// Here goes our drawing code 
int
drawGLScene( void )
{
    static gameTime gt;
    printf("%s: \n", __PRETTY_FUNCTION__);

#if 1
    SDL_mutexP(mutex);
    
    // Clear The Screen And The Depth Buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Move Left 1.5 Units And Into The Screen 6.0
    glLoadIdentity();
    glTranslatef( -1.5f, 0.0f, -6.0f );
    
    glBegin( GL_TRIANGLES );            // Drawing Using Triangles
    glVertex3f(  0.0f,  1.0f, 0.0f ); // Top
    glVertex3f( -1.0f, -1.0f, 0.0f ); // Botom Left
    glVertex3f(  1.0f, -1.0f, 0.0f ); // Bottom Rigt
    glEnd( );                           // Finished Drawing The Triangle
    
    /* Move Right 3 Units */
    glTranslatef( 3.0f, 0.0f, 0.0f );
    
    glBegin( GL_QUADS );                // Draw A Quad
    glVertex3f( -1.0f,  1.0f, 0.0f ); // Top Left
    glVertex3f(  1.0f,  1.0f, 0.0f ); // Top Right
    glVertex3f(  1.0f, -1.0f, 0.0f ); // Bottom Right
    glVertex3f( -1.0f, -1.0f, 0.0f ); // Bottom Left
    glEnd( );                   // Done Drawing The Quad
    
    // Draw it to the screen
    SDL_GL_SwapBuffers( );
    
    SDL_mutexV(mutex);

    int t = gt.time();
#else
    Uint32 black;
    Uint32 red;
    Uint32 green;
    Uint32 blue;
    bool done = false;
    bool init = false;
    static gameTime gt;

    // Start the game clock.
    if (!init) {
        init = true;
        gt.start();
    }
    
    sweepLine *rl = NULL;
    sweepLine *gl = NULL;
    sweepLine *bl = NULL;
    
    //Create the pixel values used in the program. Black is
    //for clearing the background and the other three are
    //for line colors. Note that in SDL you specify color
    //intensities in the rang 0 to 255 (hex ff). That
    //doesn't mean that you always get 24 or 32 bits of
    //color. If the format doesn't support the full color
    //range, SDL scales it to the range that is correct for
    //the pixel format.
    black = SDL_MapRGB(pixelformat, 0x00, 0x00, 0x00);
    red = SDL_MapRGB(pixelformat, 0xff, 0x00, 0x00);
    green = SDL_MapRGB(pixelformat, 0x00, 0xff, 0x00);
    blue = SDL_MapRGB(pixelformat, 0x00, 0x00, 0xff);

    // Create the three animating lines. It is amazing to
    // see the different kinds of behavior you can get from
    // such a simple animation object.
    
    rl = new sweepLine(surface, 
                       red, 
                       gt.time(),
                       surface->w - 1, 0,
                       -0.3, 0,
                       0, surface->h - 1,
                       0.3, 0);
    gl = new sweepLine(surface, 
                       green, 
                       gt.time(),
                       0, 0,
                       0, 0.1,
                       surface->w - 1, surface->h - 1,
                       0, -0.1);
    bl = new sweepLine(surface, 
                       blue, 
                       gt.time(),
                       surface->w - 1, 0,
                       -0.1, -0.5,
                       0, surface->h - 1,
                       0.4, 0.2);

    // Erase the old picture by painting the whole buffer
    // black.
    SDL_FillRect(surface, NULL, black);

    // Get the current game time. Note that if the clock
    // is stopped this method will return the same value
    // over and over.
    int t = gt.time();

    printf("%s(%d): t is %d\n", __PRETTY_FUNCTION__, __LINE__, t);
    
    // Based on the current time update the location of
    // each line and draw the line into the buffer.
    rl->update(t);
    gl->update(t);
    bl->update(t);
    
#if 1
    // Since I'm using a software buffer the call to
    // SDL_Flip() copies the software buffer to the
    // display. That gives you the effect of double
    // buffering without asking for it and without the
    // speed you would get from a hardware double buffered
    // display.
    SDL_Flip(surface);
#else
    // Swap the front and back buffers
    SDL_GL_SwapBuffers();
#endif
#endif
  
   return(true);
}

//----------------------------------------------------------

// The following code implements a Bresenham line drawing
// algorithm. There are 4 separate routines each optimized
// for one of the four pixel depths supported by SDL. SDL
// support many pixel formats, but it only support 8, 16,
// 24, and 32 bit pixels.

//----------------------------------------------------------

// Draw lines in 8 bit surfaces.

static void line8(SDL_Surface *s, 
                  int x1, int y1, 
                  int x2, int y2, 
                  Uint32 color)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    int d;
    int x;
    int y;
    int ax;
    int ay;
    int sx;
    int sy;
    int dx;
    int dy;

    Uint8 *lineAddr;
    Sint32 yOffset;

    dx = x2 - x1;  
    ax = abs(dx) << 1;  
    sx = sign(dx);

    dy = y2 - y1;  
    ay = abs(dy) << 1;  
    sy = sign(dy);
    yOffset = sy * s->pitch;

    x = x1;
    y = y1;

    lineAddr = ((Uint8 *)(s->pixels)) + (y * s->pitch);
    if (ax>ay)
        {                      /* x dominant */
            d = ay - (ax >> 1);
            for (;;)
                {
                    *(lineAddr + x) = (Uint8)color;

                    if (x == x2)
                        {
                            return;
                        }
                    if (d>=0)
                        {
                            y += sy;
                            lineAddr += yOffset;
                            d -= ax;
                        }
                    x += sx;
                    d += ay;
                }
        }
    else
        {                      /* y dominant */
            d = ax - (ay >> 1);
            for (;;)
                {
                    *(lineAddr + x) = (Uint8)color;

                    if (y == y2)
                        {
                            return;
                        }
                    if (d>=0) 
                        {
                            x += sx;
                            d -= ay;
                        }
                    y += sy;
                    lineAddr += yOffset;
                    d += ax;
                }
        }
}

//----------------------------------------------------------

// Draw lines in 16 bit surfaces. Note that this code will
// also work on 15 bit surfaces.

static void line16(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    int d;
    int x;
    int y;
    int ax;
    int ay;
    int sx;
    int sy;
    int dx;
    int dy;

    Uint8 *lineAddr;
    Sint32 yOffset;

    dx = x2 - x1;  
    ax = abs(dx) << 1;  
    sx = sign(dx);

    dy = y2 - y1;  
    ay = abs(dy) << 1;  
    sy = sign(dy);
    yOffset = sy * s->pitch;

    x = x1;
    y = y1;

    lineAddr = ((Uint8 *)s->pixels) + (y * s->pitch);
    if (ax>ay)
        {                      /* x dominant */
            d = ay - (ax >> 1);
            for (;;)
                {
                    *((Uint16 *)(lineAddr + (x << 1))) = (Uint16)color;

                    if (x == x2)
                        {
                            return;
                        }
                    if (d>=0)
                        {
                            y += sy;
                            lineAddr += yOffset;
                            d -= ax;
                        }
                    x += sx;
                    d += ay;
                }
        }
    else
        {                      /* y dominant */
            d = ax - (ay >> 1);
            for (;;)
                {
                    *((Uint16 *)(lineAddr + (x << 1))) = (Uint16)color;

                    if (y == y2)
                        {
                            return;
                        }
                    if (d>=0) 
                        {
                            x += sx;
                            d -= ay;
                        }
                    y += sy;
                    lineAddr += yOffset;
                    d += ax;
                }
        }
}

//----------------------------------------------------------

// Draw lines in 24 bit surfaces. 24 bit surfaces require
// special handling because the pixels don't fall on even
// address boundaries. Instead of being able to store a
// single byte, word, or long you have to store 3
// individual bytes. As a result 24 bit graphics is slower
// than the other pixel sizes.

static void line24(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    int d;
    int x;
    int y;
    int ax;
    int ay;
    int sx;
    int sy;
    int dx;
    int dy;

    Uint8 *lineAddr;
    Sint32 yOffset;

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    color <<= 8;
#endif

    dx = x2 - x1;  
    ax = abs(dx) << 1;  
    sx = sign(dx);

    dy = y2 - y1;  
    ay = abs(dy) << 1;  
    sy = sign(dy);
    yOffset = sy * s->pitch;

    x = x1;
    y = y1;

    lineAddr = ((Uint8 *)(s->pixels)) + (y * s->pitch);
    if (ax>ay)
        {                      /* x dominant */
            d = ay - (ax >> 1);
            for (;;)
                {
                    Uint8 *p = (lineAddr + (x * 3));
                    memcpy(p, &color, 3);

                    if (x == x2)
                        {
                            return;
                        }
                    if (d>=0)
                        {
                            y += sy;
                            lineAddr += yOffset;
                            d -= ax;
                        }
                    x += sx;
                    d += ay;
                }
        }
    else
        {                      /* y dominant */
            d = ax - (ay >> 1);
            for (;;)
                {
                    Uint8 *p = (lineAddr + (x * 3));
                    memcpy(p, &color, 3);

                    if (y == y2)
                        {
                            return;
                        }
                    if (d>=0) 
                        {
                            x += sx;
                            d -= ay;
                        }
                    y += sy;
                    lineAddr += yOffset;
                    d += ax;
                }
        }
}

//----------------------------------------------------------

// Draw lines in 32 bit surfaces. Note that this routine
// ignores alpha values. It writes them into the surface
// if they are included in the pixel, but does nothing
// else with them.

static void line32(SDL_Surface *s, 
                   int x1, int y1, 
                   int x2, int y2, 
                   Uint32 color)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
    int d;
    int x;
    int y;
    int ax;
    int ay;
    int sx;
    int sy;
    int dx;
    int dy;

    Uint8 *lineAddr;
    Sint32 yOffset;

    dx = x2 - x1;  
    ax = abs(dx) << 1;  
    sx = sign(dx);

    dy = y2 - y1;  
    ay = abs(dy) << 1;  
    sy = sign(dy);
    yOffset = sy * s->pitch;

    x = x1;
    y = y1;

    lineAddr = ((Uint8 *)(s->pixels)) + (y * s->pitch);
    if (ax>ay)
        {                      /* x dominant */
            d = ay - (ax >> 1);
            for (;;)
                {
                    *((Uint32 *)(lineAddr + (x << 2))) = (Uint32)color;

                    if (x == x2)
                        {
                            return;
                        }
                    if (d>=0)
                        {
                            y += sy;
                            lineAddr += yOffset;
                            d -= ax;
                        }
                    x += sx;
                    d += ay;
                }
        }
    else
        {                      /* y dominant */
            d = ax - (ay >> 1);
            for (;;)
                {
                    *((Uint32 *)(lineAddr + (x << 2))) = (Uint32)color;

                    if (y == y2)
                        {
                            return;
                        }
                    if (d>=0) 
                        {
                            x += sx;
                            d -= ay;
                        }
                    y += sy;
                    lineAddr += yOffset;
                    d += ax;
                }
        }
}

//----------------------------------------------------------

// Examine the depth of a surface and select a line
// drawing routine optimized for the bytes/pixel of the
// surface.

static void line(SDL_Surface *s, 
                 int x1, int y1, 
                 int x2, int y2, 
                 Uint32 color)
{
    printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);

#if 0
    switch (s->format->BytesPerPixel)
        {
          case 1:
              line8(s, x1, y1, x2, y2, color);
              break;
          case 2:
              line16(s, x1, y1, x2, y2, color);
              break;
          case 3:
              line24(s, x1, y1, x2, y2, color);
              break;
          case 4:
              line32(s, x1, y1, x2, y2, color);
              break;
        }
#endif
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
