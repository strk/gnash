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
    sprintf (SDL_windowhack,"SDL_WINDOWID=%d", id);
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

#if 1
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
    videoFlags |= SDL_RESIZABLE;       // /var/www/html/gnashEnable window resizing
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
    videoFlags |= SDL_ANYFORMAT;
    
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

int
drawGLScene( void )
{
    printf("%s: \n", __PRETTY_FUNCTION__);
    
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
  
   return(true);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
