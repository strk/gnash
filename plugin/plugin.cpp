//   Copyright (C) 2005 Free Software Foundation, Inc.
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

#include "plugin.h"
#define MIME_TYPES_HANDLED  "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME     "Shockwave Flash 7.0"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":swf:"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  PLUGIN_NAME

#include <GL/gl.h>              // Header File For The OpenGL32 Library
#include <GL/glu.h>             // Header File For The GLu32 Library

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <Xm/XmAll.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Sunkeysym.h>
#include <SDL.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

// This is our SDL surface
SDL_Surface *surface;

bool processing = false;
int  streamfd = 0;

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 16;
const int INBUFSIZE = 1024;

int main_loop();
void Quit(int ret);
int initGL(GLvoid);
int drawGLScene(GLvoid);
int resizeWindow(int width, int height);
void handleKeyPress(SDL_keysym *keysym);

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

GLuint	texture[3];		          // Storage for 3 textures.
GLuint  blend;                  // Turn blending on/off
GLuint	filter;                 // Which Filter To Use

char* NPP_GetMIMEDescription(void)
{
  return(MIME_TYPES_DESCRIPTION);
}

/////////////////////////////////////
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

// get values per plugin
NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
  NPError err = NPERR_NO_ERROR;
  switch (aVariable) {
  case NPPVpluginNameString:
    *((char **)aValue) = PLUGIN_NAME;
    break;
  case NPPVpluginDescriptionString:
    *((char **)aValue) = PLUGIN_DESCRIPTION;
    break;
  case NPPVpluginTimerInterval:
  case NPPVpluginNeedsXEmbed:
  case NPPVpluginKeepLibraryInMemory:
  default:
    err = NPERR_INVALID_PARAM;
    break;
  }
  return err;
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
  mInstance(aInstance),
  mInitialized(FALSE),
  mWindow(0),
  mXtwidget(0),
  mFontInfo(0)
{
}

nsPluginInstance::~nsPluginInstance()
{
}

static void
xt_event_handler(Widget xtwidget, nsPluginInstance *plugin, XEvent *xevent, Boolean *b)
{
  int        keycode;
  KeySym     keysym;
#if 0
  SDL_Event  sdl_event;
  SDL_keysym sdl_keysym;

  //    handleKeyPress((SDL_keysym)keysym);
  printf("Peep Event returned %d\n", SDL_PeepEvents(&sdl_event, 1, SDL_PEEKEVENT, SDL_USEREVENT|SDL_ACTIVEEVENT|SDL_KEYDOWN|SDL_KEYUP|SDL_MOUSEBUTTONUP|SDL_MOUSEBUTTONDOWN));
  
  if (SDL_PollEvent(&sdl_event)) {
    switch(sdl_event.type) {
    case SDL_ACTIVEEVENT:
    case SDL_VIDEORESIZE:
    case SDL_KEYDOWN:
      /* handle key presses */
      handleKeyPress( &sdl_event.key.keysym );
      break;
    default:
      break;
      
    }
  }
#endif
  
  switch (xevent->type) {
  case Expose:
    // get rid of all other exposure events
    if (plugin) {
      //while(XCheckTypedWindowEvent(plugin->Display(), plugin->Window(), Expose, xevent));
#if 1
      drawGLScene();
#else
      plugin->draw();
#endif
    }
  case ButtonPress:
//     fe.type = FeButtonPress;
    printf("Button Press\n");
    break;
  case ButtonRelease:
    //     fe.type = FeButtonRelease;
    printf("Button Release\n");
    break;
  case KeyPress:
    keycode = xevent->xkey.keycode;
    keysym = XLookupKeysym((XKeyEvent*)xevent, 0);
    printf ("%s(%d): Keysym is %s\n", __PRETTY_FUNCTION__, __LINE__,
        XKeysymToString(keysym));

    switch (keysym) {
    case XK_Up:
      printf("Key Up\n");
      break;
    case XK_Down:
      printf("Key Down\n");
      break;
    case XK_Left:
      printf("Key Left\n");
      break;
    case XK_Right:
      printf("Key Right\n");
      break;
    case XK_Return:
      printf("Key Return\n");
      break;
      
    default:
      break;
    }
  }
}

void
nsPluginInstance::draw()
{
  unsigned int h = mHeight/2;
  unsigned int w = 3 * mWidth/4;
  int x = (mWidth - w)/2; // center
  int y = h/2;
  if (x >= 0 && y >= 0) {
    GC gc = XCreateGC(mDisplay, mWindow, 0, NULL);
    if (!gc) 
      return;
    XDrawRectangle(mDisplay, mWindow, gc, x, y, w, h);
    const char *string = getVersion();
    if (string && *string) {
      int l = strlen(string);
      int fmba = mFontInfo->max_bounds.ascent;
      int fmbd = mFontInfo->max_bounds.descent;
      int fh = fmba + fmbd;
      y += fh;
      x += 32;
      XDrawString(mDisplay, mWindow, gc, x, y, string, l); 
    }
    XFreeGC(mDisplay, gc);
  }
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
  if(aWindow == NULL)
    return FALSE;
  
  if (SetWindow(aWindow))
  mInitialized = TRUE;
	
  return mInitialized;
}

void nsPluginInstance::shut()
{
  mInitialized = FALSE;
}

const char * nsPluginInstance::getVersion()
{
  return NPN_UserAgent(mInstance);
}

NPError nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  NPError err = NPERR_NO_ERROR;
  switch (aVariable) {
    case NPPVpluginNameString:
    case NPPVpluginDescriptionString:
      return NS_PluginGetValue(aVariable, aValue) ;
      break;
    default:
      err = NPERR_INVALID_PARAM;
      break;
  }
  return err;

}

NPError nsPluginInstance::SetWindow(NPWindow* aWindow)
{
  if(aWindow == NULL)
    return FALSE;

  mX = aWindow->x;
  mY = aWindow->y;
  mWidth = aWindow->width;
  mHeight = aWindow->height;

  printf("%s: X origin = %d, Y Origin = %d, Width = %d, Height = %d\n",
         __PRETTY_FUNCTION__, mX, mY, mWidth, mHeight);
  
  if (mWindow == (Window) aWindow->window) {
    // The page with the plugin is being resized.
    // Save any UI information because the next time
    // around expect a SetWindow with a new window id.
  } else {
    mWindow = (Window) aWindow->window;
    NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct *)aWindow->ws_info;
    mDisplay = ws_info->display;
    mVisual = ws_info->visual;
    mDepth = ws_info->depth;
    mColormap = ws_info->colormap;

    if (!mFontInfo) {
      if (!(mFontInfo = XLoadQueryFont(mDisplay, "9x15")))
        printf("Cannot open 9X15 font\n");
    }
    // add xt event handler
    Widget xtwidget = XtWindowToWidget(mDisplay, mWindow);
    if (xtwidget && mXtwidget != xtwidget) {
      mXtwidget = xtwidget;
      // mask values are:
      // KeyPress, KeyRelease, ButtonPress, ButtonRelease,
      // PointerMotion, Button1Motion, Button2Motion, Button3Motion,
      // Button4Motion, Button5Motion 
      long event_mask = ExposureMask|KeyPress|KeyRelease|ButtonPress|ButtonRelease;
      XSelectInput(mDisplay, mWindow, event_mask);
      XtAddEventHandler(xtwidget, event_mask, False, (XtEventHandler)xt_event_handler, this);
    }
  }
  
#if 0
  if (aWindow->type == NPWindowTypeWindow) {
    WriteStatus("Window type is \"Windowed\"");
  }
  if (aWindow->type == NPWindowTypeDrawable) {
    WriteStatus("Window type is \"Drawable\"");
  }
#endif
  
#if 1
  // Flags to pass to SDL_SetVideoMode 
  int videoFlags;
  // main loop variable 
  int done = FALSE;
  // used to collect events 
  SDL_Event event;
  // this holds some info about our display 
  const SDL_VideoInfo *videoInfo;
  // whether or not the window is active 
  int isActive = TRUE;
  
  char SDL_windowhack[32];
  sprintf (SDL_windowhack,"SDL_WINDOWID=%ld", mWindow);
  putenv (SDL_windowhack);
 
  // initialize SDL 
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD | SDL_INIT_AUDIO) < 0 )
    {
	    fprintf( stderr, "Video initialization failed: %s\n",
               SDL_GetError( ) );
	    Quit( 1 );
    }
  
  // Fetch the video info 
  videoInfo = SDL_GetVideoInfo( );
  
  if ( !videoInfo )
    {
	    fprintf( stderr, "Video query failed: %s\n",
               SDL_GetError( ) );
	    Quit( 1 );
    }
  
  // the flags to pass to SDL_SetVideoMode 
  videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL 
  videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering 
  videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware 
  videoFlags |= SDL_RESIZABLE;       // Enable window resizing 
  
  // This checks to see if surfaces can be stored in memory 
  if ( videoInfo->hw_available )
    videoFlags |= SDL_HWSURFACE;
  else
      videoFlags |= SDL_SWSURFACE;
  
  // This checks if hardware blits can be done 
  if ( videoInfo->blit_hw )
    videoFlags |= SDL_HWACCEL;
  
  // Sets up OpenGL double buffering 
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  // get a SDL surface 
  surface = SDL_SetVideoMode( mWidth, mHeight, SCREEN_BPP,
                              videoFlags );
  if ( !surface )  // Verify there is a surface
    {
      printf("ERROR: Video mode set failed: %s\n", SDL_GetError( ) );
      Quit(1);
    }
  
  // initialize OpenGL 
  initGL();
  
  // resize the initial window 
  resizeWindow( mWidth, mHeight );

  drawGLScene();
  
#else
  main_loop();

  draw();
#endif
  

  return TRUE;
}

// Write a status message to the status line and the console.
NPError
nsPluginInstance::WriteStatus(char *msg) const
{
  NPN_Status(mInstance, msg);
  printf("%s\n", msg);
}

// Open a new incoming data stream, which is the flash movie we want to play.
// A URL can be pretty ugly, like in this example:
// http://www.shockwave.com/swf/navbar/navbar_sw.swf?atomfilms=http%3a//www.atomfilms.com/af/home/&shockwave=http%3a//www.shockwave.com&gameblast=http%3a//gameblast.shockwave.com/gb/gbHome.jsp&known=0
NPError
nsPluginInstance::NewStream(NPMIMEType type, NPStream * stream,
				    NPBool seekable, uint16 * stype)
{
  char tmp[100];
  memset(tmp, 0, 100);
  string url = stream->url;
  string fname;
  int start, end;

  end   = url.find(".swf", 0) + 4;
  start = url.rfind("/", end) + 1;
  fname = "/tmp/";
  fname += url.substr(start, end - start);

  //  printf("%s: URL is %s\n", __PRETTY_FUNCTION__, url.c_str());
  printf("%s: Open stream for %s (%d, %d)\n", __PRETTY_FUNCTION__, fname.c_str(), start, end);

  sprintf(tmp, "Loading Shockwave file %s", fname.c_str());
  WriteStatus(tmp);
  
  streamfd = open(fname.c_str(), O_CREAT | O_WRONLY, S_IRUSR|S_IRGRP|S_IROTH);
  if (streamfd < 0) {
    sprintf(tmp,"%s can't be opened, check your permissions!\n", fname.c_str());
    WriteStatus(tmp);
    streamfd = open(fname.c_str(), O_TRUNC | O_WRONLY, S_IRUSR|S_IRGRP|S_IROTH);
    if (streamfd < 0) {
      sprintf(tmp,"%s can't be created, check your permissions!\n", fname.c_str());
      WriteStatus(tmp);
    }
  }
  
  processing = true;
  return NPERR_NO_ERROR;
}

NPError
nsPluginInstance::DestroyStream(NPStream * stream, NPError reason)
{
    int playable, all_retrieved, all_above_cache;
    char *tmp;

    printf("%s (%i): %s\n", __PRETTY_FUNCTION__, reason, stream->url);
    processing = false;
    
    SDL_Quit();
}

void
nsPluginInstance::URLNotify(const char *url, NPReason reason,
				 void *notifyData)
{
    bool isHttpStream = false;

    printf("URL: %s\nReason %i\n", url, reason);
}

// Return how many bytes we can read into the buffer
int32
nsPluginInstance::WriteReady(NPStream * stream)
{
  printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
  printf("Stream for %s is ready\n", stream->url);

  return INBUFSIZE;
}

// Read the daat stream from Mozilla/Firefox
int32
nsPluginInstance::Write(NPStream * stream, int32 offset, int32 len,
			      void *buffer)
{
  printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
  printf("Reading Stream %s, offset is %d, length = %d \n",
         stream->url, offset, len);

  write(streamfd, buffer, len);
}

// function to release/destroy our resources and restoring the old desktop
void
Quit( int returnCode )
{
  // clean up the window
    SDL_Quit( );

    // and exit appropriately
    exit( returnCode );
}

// function to load in bitmap as a GL texture
int
LoadGLTextures( )
{
  // Status indicator 
  int ret = FALSE;
  
  printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);

  // Create storage space for the texture 
  SDL_Surface *TextureImage[1];

  // Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit 
  if ( ( TextureImage[0] = SDL_LoadBMP( "/home/rob/projects/gnu/nehe/linux/Data/lesson8/glass.bmp" ) ) )
    {
      
      printf("%s(%d): Loaded texture bitmap\n", __PRETTY_FUNCTION__, __LINE__);
      
      // Set the status to true 
      ret = TRUE;
      
      // Create The Texture 
      glGenTextures( 3, &texture[0] );
      
      // Load in texture 1 
      // Typical Texture Generation Using Data From The Bitmap 
      glBindTexture( GL_TEXTURE_2D, texture[0] );
      
      // Generate The Texture 
      glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w,
                    TextureImage[0]->h, 0, GL_BGR,
                    GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
      
      // Nearest Filtering 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       GL_NEAREST );
      
      // Load in texture 2 
      // Typical Texture Generation Using Data From The Bitmap 
      glBindTexture( GL_TEXTURE_2D, texture[1] );
      
      // Linear Filtering 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       GL_LINEAR );
      
      // Generate The Texture 
      glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w,
                    TextureImage[0]->h, 0, GL_BGR,
                    GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
      
      // Load in texture 3 
      // Typical Texture Generation Using Data From The Bitmap 
      glBindTexture( GL_TEXTURE_2D, texture[2] );
      
      // Mipmapped Filtering 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       GL_LINEAR_MIPMAP_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       GL_LINEAR );
      
      // Generate The MipMapped Texture ( NEW ) 
      gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[0]->w,
                         TextureImage[0]->h, GL_BGR,
                         GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
    }
  
  // Free up any memory we may have used 
  if ( TextureImage[0] ) 
    {
      SDL_FreeSurface( TextureImage[0] );
    } else 
      {
        printf("%s(%d): Couldn't Loaded texture bitmap\n", __PRETTY_FUNCTION__, __LINE__);
      }  
  
  return ret;
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
  
  // Make sure we're chaning the model view and not the projection 
  glMatrixMode( GL_MODELVIEW );
  
  // Reset The View 
  glLoadIdentity( );
  
  return( TRUE );
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
int
initGL( GLvoid )
{

  // Load in the texture 
  if ( !LoadGLTextures( ) )
    return FALSE;
  
  // Enable Texture Mapping ( NEW ) 
  glEnable( GL_TEXTURE_2D );
  
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
  
  // Setup The Ambient Light 
  glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );
  
  // Setup The Diffuse Light 
  glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );
  
  // Position The Light 
  glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );
  
  // Enable Light One 
  glEnable( GL_LIGHT1 );
  
  // Full Brightness, 50% Alpha ( NEW ) 
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);
  
  // Blending Function For Translucency Based On Source Alpha Value  
  glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  
  return( TRUE );
}

// Here goes our drawing code 
int
drawGLScene( GLvoid )
{
  // These are to calculate our fps 
  static GLint T0     = 0;
  static GLint Frames = 0;
  
  // Clear The Screen And The Depth Buffer 
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  // Reset the view 
  glLoadIdentity( );
  
  // Translate Into/Out Of The Screen By z 
  glTranslatef( 0.0f, 0.0f, z );
  
  glRotatef( xrot, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis By xrot 
  glRotatef( yrot, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis By yrot 
  
  // Select A Texture Based On filter 
  glBindTexture( GL_TEXTURE_2D, texture[filter] );
  
  // Start Drawing Quads 
  glBegin( GL_QUADS );
  // Front Face 
  // Normal Pointing Towards Viewer 
  glNormal3f( 0.0f, 0.0f, 1.0f );
  // Point 1 (Front) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
  // Point 2 (Front) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
  // Point 3 (Front) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
  // Point 4 (Front) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
  
  // Back Face 
  // Normal Pointing Away From Viewer 
  glNormal3f( 0.0f, 0.0f, -1.0f);
  // Point 1 (Back) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
  // Point 2 (Back) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
  // Point 3 (Back) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
  // Point 4 (Back) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
  
  // Top Face 
  // Normal Pointing Up 
  glNormal3f( 0.0f, 1.0f, 0.0f );
  // Point 1 (Top) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
  // Point 2 (Top) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
  // Point 3 (Top) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
  // Point 4 (Top) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
  
  // Bottom Face 
  // Normal Pointing Down 
  glNormal3f( 0.0f, -1.0f, 0.0f );
  // Point 1 (Bottom) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
  // Point 2 (Bottom) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
  // Point 3 (Bottom) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
  // Point 4 (Bottom) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
  
  // Right face 
  // Normal Pointing Right 
  glNormal3f( 1.0f, 0.0f, 0.0f);
  // Point 1 (Right) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );
  // Point 2 (Right) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );
  // Point 3 (Right) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );
  // Point 4 (Right) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );
  
  // Left Face
  // Normal Pointing Left 
  glNormal3f( -1.0f, 0.0f, 0.0f );
  // Point 1 (Left) 
  glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
  // Point 2 (Left) 
  glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
  // Point 3 (Left) 
  glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
  // Point 4 (Left) 
  glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
  glEnd();
  
  // Draw it to the screen 
  SDL_GL_SwapBuffers( );

#if 1
  // Gather our frames per second 
  Frames++;
  {
    GLint t = SDL_GetTicks();
    if (t - T0 >= 5000) {
      GLfloat seconds = (t - T0) / 1000.0;
      GLfloat fps = Frames / seconds;
      printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
      T0 = t;
      Frames = 0;
    }
  }
#endif
  xrot += xspeed; // Add xspeed To xrot 
  yrot += yspeed; // Add yspeed To yrot 
  
  return( TRUE );
}

int
main_loop( )
{
  // Flags to pass to SDL_SetVideoMode 
  int videoFlags;
  // main loop variable 
  int done = FALSE;
  // used to collect events 
  SDL_Event event;
  // this holds some info about our display 
  const SDL_VideoInfo *videoInfo;
  // whether or not the window is active 
  int isActive = TRUE;
  
  char SDL_windowhack[32];
//   sprintf (SDL_windowhack,"SDL_WINDOWID=%ld", window_id);
//   putenv (SDL_windowhack);
 
  // initialize SDL 
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
	    fprintf( stderr, "Video initialization failed: %s\n",
               SDL_GetError( ) );
	    Quit( 1 );
    }
  
  // Fetch the video info 
  videoInfo = SDL_GetVideoInfo( );
  
  if ( !videoInfo )
    {
	    fprintf( stderr, "Video query failed: %s\n",
               SDL_GetError( ) );
	    Quit( 1 );
    }
  
  // the flags to pass to SDL_SetVideoMode 
  videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL 
  videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering 
  videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware 
  videoFlags |= SDL_RESIZABLE;       // Enable window resizing 
  
  // This checks to see if surfaces can be stored in memory 
  if ( videoInfo->hw_available )
    videoFlags |= SDL_HWSURFACE;
  else
      videoFlags |= SDL_SWSURFACE;
  
  // This checks if hardware blits can be done 
  if ( videoInfo->blit_hw )
    videoFlags |= SDL_HWACCEL;
  
  // Sets up OpenGL double buffering 
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  // get a SDL surface 
  surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                              videoFlags );
  if ( !surface )  // Verify there is a surface
    {
      fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
      Quit( 1 );
    }
  
  // initialize OpenGL 
  initGL( );
  
  // resize the initial window 
  resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );
  
  // wait for events 
  while ( !done )
    {
#if 0
      // handle the events in the queue 
      
      while ( SDL_PollEvent( &event ) )
        {
          switch( event.type )
            {
            case SDL_ACTIVEEVENT:
              // Something's happend with our focus
              // If we lost focus or we are iconified, we
              // shouldn't draw the screen 
              if ( event.active.gain == 0 )
                isActive = FALSE;
              else
                isActive = TRUE;
              break;			    
            case SDL_VIDEORESIZE:
              // handle resize event 
              surface = SDL_SetVideoMode( event.resize.w,
                                          event.resize.h,
                                          16, videoFlags );
              if ( !surface )
                {
                  fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
                  Quit( 1 );
                }
              resizeWindow( event.resize.w, event.resize.h );
              break;
            case SDL_KEYDOWN:
              // handle key presses 
              handleKeyPress( &event.key.keysym );
              break;
            case SDL_QUIT:
              // handle quit requests 
              done = TRUE;
              break;
            default:
              break;
            }
        }
#endif
      // draw the scene 
      if ( isActive )
        drawGLScene( );
    }
  
  // clean ourselves up and exit 
  Quit( 0 );
  
  // Should never get here 
  return( 0 );
}
