#include "GameSWFLibType.h"




// gameswf_render_handler_d3d.cpp -- Thatcher Ulrich <http://tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A gameswf::render_handler that uses Xbox API's


#include <windows.h>
#include <assert.h>
#include "gameswf.h"
#include "gameswf/gameswf_log.h"
#include "gameswf_types.h"
#include "base/image.h"
#include "base/container.h"

#if defined (_DX8)
#include <d3d8.h>
#include <d3dx8tex.h>
#include <d3d8types.h>
#include <NiDX8RenderState.h>
extern NiDX8RenderState* renderState;
#else #if defined (_DX9)
#include <d3d9.h>
#include <d3dx9tex.h>
#include <d3d9types.h>
#include <NiDX9RenderState.h>
extern NiDX9RenderState* renderState;
#endif

#include <NiD3DRenderState.h>
#include <string.h>

#define MAX_SIZE_VERTEX_BUFFER  2500

#if defined (_DX8)
LPDIRECT3D8             m_pD3D;       // Used to create the D3DDevice
LPDIRECT3DDEVICE8       m_pd3dDevice; // Our rendering device
LPDIRECT3DVERTEXBUFFER8 g_pVB        = NULL; // Buffer to hold vertices
#else #if defined(_DX9)
LPDIRECT3D9             m_pD3D;       // Used to create the D3DDevice
LPDIRECT3DDEVICE9       m_pd3dDevice; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER9 g_pVB2       = NULL; // Buffer to hold vertices
#endif

D3DXMATRIX              modelViewMatrix, projMatrix, coordTransform ;

LPDIRECT3DTEXTURE9      g_pTexture   = NULL; // Our test texture

#define Z_DEPTH 1.0f


// A structure for our custom vertex type. We added texture coordinates
struct CUSTOMVERTEX
{
    FLOAT     x, y, z; // The transformed position for the vertex
    DWORD     color;        // The vertex color
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct CUSTOMVERTEX2
{
    FLOAT     x,y,z; // The position
    DWORD     color;    // The color
    FLOAT     tu, tv;   // The texture coordinates
};

#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)


namespace
{
  #if defined (_DX8)
  array<IDirect3DBaseTexture8*> s_d3d_textures;
  #else #if (_DX9)
  array<IDirect3DBaseTexture9*> s_d3d_textures;
  #endif

  DWORD s_vshader_handle = 0;
};


// bitmap_info_d3d declaration
struct bitmap_info_d3d : public gameswf::bitmap_info
{
  bitmap_info_d3d();
  void convert_to_argb(image::rgba* im);
  bitmap_info_d3d(image::rgb* im);
  bitmap_info_d3d(image::rgba* im);
  bitmap_info_d3d(int width, int height, Uint8* data);
};


struct render_handler_d3d : public gameswf::render_handler
{
  static gameswf::rgba   m_curColor;  // the current pipeline color
  
  // Some renderer state.

  gameswf::matrix m_viewport_matrix;
  gameswf::matrix m_current_matrix;
  gameswf::cxform m_current_cxform;
  
  #if defined (_DX8)
  static void SetD3DDevice( LPDIRECT3DDEVICE8 device )
  #else #if defined(_DX9)
  static void SetD3DDevice( LPDIRECT3DDEVICE9 device )
  #endif
  {
    m_pd3dDevice = device;
  }

  static void ReleaseD3DDevice( )
  {
    if (m_pd3dDevice)
    {
//      m_pd3dDevice->Release();
      m_pd3dDevice = 0;
    }
  }

  void set_antialiased(bool enable)
  {
    // not supported
  }

  static void make_next_miplevel(int* width, int* height, Uint8* data)
  // Utility.  Mutates *width, *height and *data to create the
  // next mip level.
  {
    assert(width);
    assert(height);
    assert(data);

    int new_w = *width >> 1;
    int new_h = *height >> 1;
    if (new_w < 1) new_w = 1;
    if (new_h < 1) new_h = 1;
    
    if (new_w * 2 != *width  || new_h * 2 != *height)
    {
      // Image can't be shrunk along (at least) one
      // of its dimensions, so don't bother
      // resampling.  Technically we should, but
      // it's pretty useless at this point.  Just
      // change the image dimensions and leave the
      // existing pixels.
    }
    else
    {
      // Resample.  Simple average 2x2 --> 1, in-place.
      for (int j = 0; j < new_h; j++) {
        Uint8*  out = ((Uint8*) data) + j * new_w;
        Uint8*  in = ((Uint8*) data) + (j << 1) * *width;
        for (int i = 0; i < new_w; i++) {
          int a;
          a = (*(in + 0) + *(in + 1) + *(in + 0 + *width) + *(in + 1 + *width));
          *(out) = (Uint8) (a >> 2);
          out++;
          in += 2;
        }
      }
    }

    // Munge parameters to reflect the shrunken image.
    *width = new_w;
    *height = new_h;
  }

  struct fill_style
  {
    enum mode
    {
      INVALID,
      COLOR,
      BITMAP_WRAP,
      BITMAP_CLAMP,
      LINEAR_GRADIENT,
      RADIAL_GRADIENT,
    };
    mode  m_mode;
    gameswf::rgba m_color;
    const gameswf::bitmap_info* m_bitmap_info;
    gameswf::matrix m_bitmap_matrix;
    gameswf::cxform m_bitmap_color_transform;
    bool  m_has_nonzero_bitmap_additive_color;

    fill_style()
      :
      m_mode(INVALID),
      m_has_nonzero_bitmap_additive_color(false)
    {
    }

    void  apply(const gameswf::matrix& current_matrix) const
    // Push our style into D3D.
    {
      assert(m_mode != INVALID);

      if (m_mode == COLOR)
      {
        apply_color(m_color);
        renderState->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        renderState->SetRenderState( D3DRS_COLORVERTEX, TRUE);
        renderState->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
      }
      else if (m_mode == BITMAP_WRAP
         || m_mode == BITMAP_CLAMP)
      {
        assert(m_bitmap_info != NULL);

        apply_color(m_color);

        if (m_bitmap_info == NULL)
        {
          assert(0);
          renderState->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        }
        else
        {

          // Set up the texture for rendering.

          // Do the modulate part of the color
          // transform in the first pass.  The
          // additive part, if any, needs to
          // happen in a second pass.

          m_pd3dDevice->SetTexture(0, s_d3d_textures[m_bitmap_info->m_texture_id]);
          renderState->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
          renderState->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
          renderState->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
          renderState->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
          renderState->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        
          if (m_mode == BITMAP_CLAMP)
          {
//            renderState->SetSamplerState(0, NiD3DRenderState::NISAMP_ADDRESSU, NiTexturingProperty::CLAMP_S_CLAMP_T);
//            renderState->SetSamplerState(0, NiD3DRenderState::NISAMP_ADDRESSV, NiTexturingProperty::CLAMP_S_CLAMP_T);
          }
          else
          {
            assert(m_mode == BITMAP_WRAP);
            renderState->SetSamplerState(0, NiD3DRenderState::NISAMP_ADDRESSU, NiTexturingProperty::WRAP_S_WRAP_T);
            renderState->SetSamplerState(0, NiD3DRenderState::NISAMP_ADDRESSV, NiTexturingProperty::WRAP_S_WRAP_T);
          }

          // Set up the bitmap matrix for texgen.
          float inv_width = 1.0f / m_bitmap_info->m_original_width;
          float inv_height = 1.0f / m_bitmap_info->m_original_height;

          gameswf::matrix m = m_bitmap_matrix;
          gameswf::matrix m_cm_inv;
          m_cm_inv.set_inverse(current_matrix);
          m.concatenate(m_cm_inv);

          D3DXMATRIXA16 mat;
          D3DXMatrixIdentity(&mat);

          mat._11 = m.m_[0][0] * inv_width;             mat._12 = m.m_[1][0] * inv_height;  mat._13 = 0.00f; mat._14 = 0.00f;
          mat._21 = m.m_[0][1] * inv_width;             mat._22 = m.m_[1][1] * inv_height;  mat._23 = 0.00f; mat._24 = 0.00f;
          mat._31 = 0.00f;                              mat._32 = 0.00f;                    mat._33 = 0.00f; mat._34 = 0.00f;
          mat._41 = m.m_[0][2] * inv_width;             mat._42 = m.m_[1][2] * inv_height;  mat._43 = 0.00f; mat._44 = 1.00f;

          coordTransform = coordTransform * mat;

          m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat );
          renderState->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 
            D3DTTFF_COUNT2 );
          renderState->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        }
      }
    }


    bool  needs_second_pass() const
    // Return true if we need to do a second pass to make
    // a valid color.  This is for cxforms with additive
    // parts.
    {
      if (m_mode == BITMAP_WRAP
          || m_mode == BITMAP_CLAMP)
      {
        return m_has_nonzero_bitmap_additive_color;
      }
      else
      {
        return false;
      }
    }

    void  apply_second_pass() const
    // Set D3D state for a necessary second pass.
    {
      assert(0);   // not sure if this is ever used.  this code hasn't been tested.
      assert(needs_second_pass());

      // Additive color.
//pk
      //m_pd3dDevice->SetVertexData4f(
      //  D3DVSDE_DIFFUSE,
      //  m_bitmap_color_transform.m_[0][1] / 255.0f,
      //  m_bitmap_color_transform.m_[1][1] / 255.0f,
      //  m_bitmap_color_transform.m_[2][1] / 255.0f,
      //  m_bitmap_color_transform.m_[3][1] / 255.0f
      //  );

      renderState->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      renderState->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

#if 0
      glDisable(GL_TEXTURE_2D);
      glColor4f(
        m_bitmap_color_transform.m_[0][1] / 255.0f,
        m_bitmap_color_transform.m_[1][1] / 255.0f,
        m_bitmap_color_transform.m_[2][1] / 255.0f,
        m_bitmap_color_transform.m_[3][1] / 255.0f
        );

      glBlendFunc(GL_ONE, GL_ONE);
#endif // 0
    }

    void  cleanup_second_pass() const
    {
      assert(0);   // not sure if this is ever used.  this code hasn't been tested.
      renderState->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      renderState->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

#if 0
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // 0
    }


    void  disable() { m_mode = INVALID; }
    void  set_color(gameswf::rgba color) { m_mode = COLOR; m_color = color; }
    void  set_bitmap(const gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
    {
      m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
      m_color = gameswf::rgba();
      m_bitmap_info = bi;
      m_bitmap_matrix = m;
      m_bitmap_color_transform = color_transform;

      if (m_bitmap_color_transform.m_[0][1] > 1.0f
          || m_bitmap_color_transform.m_[1][1] > 1.0f
          || m_bitmap_color_transform.m_[2][1] > 1.0f
          || m_bitmap_color_transform.m_[3][1] > 1.0f)
      {
        m_has_nonzero_bitmap_additive_color = true;
      }
      else
      {
        m_has_nonzero_bitmap_additive_color = false;
      }
    }
    bool  is_valid() const { return m_mode != INVALID; }
  };

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
#if 0
    // Create the D3D object, which is needed to create the D3DDevice.
#if defined (_DX8)
    if( NULL == ( m_pD3D = Direct3DCreate8( D3D_SDK_VERSION ) ) )
#else #if defined (_DX9)
    if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
#endif
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. Most parameters are
    // zeroed out. We set Windowed to TRUE, since we want to do D3D in a
    // window, and then set the SwapEffect to "discard", which is the most
    // efficient method of presenting the back buffer to the display.  And 
    // we request a back buffer format that matches the current desktop display 
    // format.
    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;



    // Create the Direct3D device. Here we are using the default adapter (most
    // systems only have one, unless they have multiple graphics hardware cards
    // installed) and requesting the HAL (which is saying we want the hardware
    // device rather than a software one). Software vertex processing is 
    // specified since we know it will work on all cards. On cards that support 
    // hardware vertex processing, though, we would see a big performance gain 
    // by specifying hardware vertex processing.
    //HRESULT result = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
    //                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    //                                  &d3dpp, &m_pd3dDevice );

    //if (FAILED(result))
    //{
    //    return E_FAIL;
    //}

    // Device state would normally be set here

    // Turn off culling
    renderState->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    renderState->SetRenderState( D3DRS_LIGHTING, FALSE);

    // Turn on the zbuffer
//    renderState->SetRenderState( D3DRS_ZENABLE, TRUE );
    renderState->SetRenderState( D3DRS_ZENABLE, TRUE );

#endif

//    renderState->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME);

    // test textures
//   if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, "data/textures/banana.bmp", &g_pTexture ) ) )
//   if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, "data/textures/alpha.dds", &g_pTexture ) ) )
   //if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, "data/textures/7.dds", &g_pTexture ) ) )
   //{
   //  assert(0);
   //}


    #if defined (_DX8)
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( MAX_SIZE_VERTEX_BUFFER*sizeof(CUSTOMVERTEX),
                                                  D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB
                                                  ) ) )
    #else #if defined(_DX9)

    // need two vertex buffers since two vertex formats are used:  one for the TCI verts and one for the verts with color and uv's

    if( FAILED( m_pd3dDevice->CreateVertexBuffer( MAX_SIZE_VERTEX_BUFFER*sizeof(CUSTOMVERTEX),
                                                  D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB,
                                                  NULL ) ) )
    {
      assert(0);
    }
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( MAX_SIZE_VERTEX_BUFFER*sizeof(CUSTOMVERTEX2),
                                                  D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX2,
                                                  D3DPOOL_DEFAULT, &g_pVB2,
                                                  NULL ) ) )
    {
      assert(0);
    }
    #endif


    return S_OK;
}





 void init(HWND hWnd)
 {
 }





  render_handler_d3d(HWND hWnd, LPDIRECT3DDEVICE9 device)
  // Constructor.
  {

    SetD3DDevice( device );

    if (InitD3D(hWnd) != S_OK)
    {
      assert(0);
    }
  
  }

  ~render_handler_d3d()
  // Destructor.
  {
    for (int i = 0; i < s_d3d_textures.size(); i++)
    {
      s_d3d_textures[i]->Release();
      s_d3d_textures[i] = 0;
    }
    

    if (g_pVB)
    {
      g_pVB->Release();
      g_pVB = 0;
    }

    if (g_pVB2)
    {
      g_pVB2->Release();
      g_pVB2 = 0;
    }

    ReleaseD3DDevice();
  }

  // Style state.
  enum style_index
  {
    LEFT_STYLE = 0,
    RIGHT_STYLE,
    LINE_STYLE,

    STYLE_COUNT
  };
  fill_style  m_current_styles[STYLE_COUNT];

  gameswf::bitmap_info* create_bitmap_info_alpha(int w, int h, unsigned char* data)
  {
    return new bitmap_info_d3d(w, h, data);
  }
  

  gameswf::bitmap_info* create_bitmap_info_rgb(image::rgb* im)
  // Given an image, returns a pointer to a bitmap_info struct
  // that can later be passed to fill_styleX_bitmap(), to set a
  // bitmap fill style.
  {
    return new bitmap_info_d3d(im);
  }


  gameswf::bitmap_info* create_bitmap_info_rgba(image::rgba* im)
  // Given an image, returns a pointer to a bitmap_info struct
  // that can later be passed to fill_style_bitmap(), to set a
  // bitmap fill style.
  //
  // This version takes an image with an alpha channel.
  {
    return new bitmap_info_d3d(im);
  }


  gameswf::bitmap_info* create_bitmap_info_empty()
  // Creates and returns an empty bitmap_info structure.  Image data
  // can be bound to this info later, via set_alpha_image().
  {
    assert(0); //pk this function not implemented.
//pk    return new bitmap_info_d3d(gameswf::bitmap_info::empty);
    return NULL;
  }


  void  set_alpha_image(gameswf::bitmap_info* bi, int w, int h, Uint8* data)
  // Set the specified bitmap_info so that it contains an alpha
  // texture with the given data (1 byte per texel).
  //
  // Munges *data (in order to make mipmaps)!!
  {
    assert(bi);
      assert(0);  // not tested

//pk    bi->set_alpha_image(w, h, data);
  }


  void  delete_bitmap_info(gameswf::bitmap_info* bi)
  // Delete the given bitmap info struct.
  {
    delete bi;
  }

  void  begin_display(
    gameswf::rgba background_color,
    int viewport_x0, int viewport_y0,
    int viewport_width, int viewport_height,
    float x0, float x1, float y0, float y1)
  // Set up to render a full frame from a movie and fills the
  // background.  Sets up necessary transforms, to scale the
  // movie to fit within the given dimensions.  Call
  // end_display() when you're done.
  //
  // The rectangle (viewport_x0, viewport_y0, viewport_x0 +
  // viewport_width, viewport_y0 + viewport_height) defines the
  // window coordinates taken up by the movie.
  //
  // The rectangle (x0, y0, x1, y1) defines the pixel
  // coordinates of the movie that correspond to the viewport
  // bounds.
  {

    D3DXMatrixIdentity(&modelViewMatrix);
    D3DXMatrixIdentity(&projMatrix);

    // invert coordinate system from lower left to upper left

    float gsWidthDiv  = 1.0f / viewport_width;
    float gsHeightDiv = 1.0f / viewport_height;

    modelViewMatrix._11 = 2.0f / (x1 - x0);
    modelViewMatrix._22 = -2.0f / (y1 - y0);
    modelViewMatrix._41 = -((x1 + x0) / (x1 - x0));
    modelViewMatrix._42 = ((y1 + y0) / (y1 - y0));

    projMatrix._11 = viewport_width * gsWidthDiv;
    projMatrix._22 = viewport_height * gsHeightDiv;
    projMatrix._41 = -1.0f + viewport_x0 * 2.0f * gsWidthDiv + viewport_width * gsWidthDiv;
    projMatrix._42 = 1.0f - viewport_y0 * 2.0f * gsHeightDiv - viewport_height * gsHeightDiv;

    coordTransform = modelViewMatrix * projMatrix;

    // Matrix setup
    D3DXMATRIX  ident;
    D3DXMatrixIdentity(&ident);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &modelViewMatrix);

    m_pd3dDevice->SetTransform(D3DTS_VIEW, &projMatrix);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &ident);

    // turn on alpha bending.
    renderState->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);

    if( NULL == m_pd3dDevice )
    {
      assert(0);
      return;
    }


    renderState->SetRenderState( D3DRS_LIGHTING, FALSE);


    // Clear the backbuffer to a blue color
//    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

    if( FAILED( m_pd3dDevice->BeginScene() ) )
    {
        // Rendering of scene objects can happen here
    
        // End the scene
        m_pd3dDevice->EndScene();
        assert(0);
    }

    


    // Viewport.
    #if defined (_DX8)
    D3DVIEWPORT8  vp;
    #else #if defined (_DX9)
    D3DVIEWPORT9  vp;
    #endif
    vp.X = viewport_x0;
    vp.Y = viewport_y0;
    vp.Width = viewport_width;
    vp.Height = viewport_height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;   // minZ = maxZ = 0 forces all polys to be in foreground
    m_pd3dDevice->SetViewport(&vp);

    // Matrix to map from SWF movie (TWIPs) coords to
    // viewport coordinates.
    m_pd3dDevice->GetViewport(&vp);
    float dx = x1 - x0;
    float dy = y1 - y0;
    if (dx < 1) { dx = 1; }
    if (dy < 1) { dy = 1; }
    m_viewport_matrix.set_identity();
    //m_viewport_matrix.m_[0][0] = vp.Width / dx;
    //m_viewport_matrix.m_[1][1] = vp.Height / dy;
    //m_viewport_matrix.m_[0][0] = viewport_width / dx;
    //m_viewport_matrix.m_[1][1] = viewport_height / dy;
    //m_viewport_matrix.m_[0][2] = viewport_x0 - m_viewport_matrix.m_[0][0] * x0;
    //m_viewport_matrix.m_[1][2] = viewport_y0 - m_viewport_matrix.m_[1][1] * y0;

    // Blending renderstates
    //pk not sure if we need these.
    //renderState->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA, false );
    //renderState->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA, false );
    //renderState->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE, false );
//    renderState->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//    renderState->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//    renderState->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    // Textures off by default.
    renderState->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

    // @@ for sanity's sake, let's turn of backface culling...
    renderState->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);//xxxxx
    renderState->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);  //xxxxx

    // Vertex format.
//pk    m_pd3dDevice->SetVertexShader(s_vshader_handle);

    // No pixel shader.
//pk    m_pd3dDevice->SetPixelShaderProgram(NULL);

#if 0
    glViewport(viewport_x0, viewport_y0, viewport_width, viewport_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glOrtho(x0, x1, y0, y1, -1, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  // GL_MODULATE

    glDisable(GL_TEXTURE_2D);
#endif // 0

    // Clear the background, if background color has alpha > 0.
    if (background_color.m_a > 0)
    {
      // @@ for testing
      static int  bobo = 0;
      m_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0,0,255),
        0.0f,
        0);

//      renderState->SetRenderState( D3DRS_COLORVERTEX, TRUE);
//      renderState->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);


      // Draw a big quad.
      apply_color(background_color);
      set_matrix(gameswf::matrix::identity);
      apply_matrix(m_current_matrix);
//pk
      //m_pd3dDevice->Begin(D3DPT_TRIANGLESTRIP);
      //m_pd3dDevice->SetVertexData2f(D3DVSDE_POSITION, x0, y0);
      //m_pd3dDevice->SetVertexData2f(D3DVSDE_POSITION, x1, y0);
      //m_pd3dDevice->SetVertexData2f(D3DVSDE_POSITION, x1, y1);
      //m_pd3dDevice->SetVertexData2f(D3DVSDE_POSITION, x0, y1);
      //m_pd3dDevice->End();
//      CUSTOMVERTEX2 triangle_strip[6];// =
    //{
    //    { 150.0f,  50.0f, 0.5f, 1.0f, 0xffff0000, }, // x, y, z, rhw, color
    //    { 250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00, },
    //    {  50.0f, 275.0f, 0.5f, 1.0f, 0xff00ffff, },
    //};

      //CUSTOMVERTEX  triangle_strip[3] =
    //{
    //    {    0.0f,   0.0f, 0.5f, 0xffff0000, }, // x, y, z, rhw, color
    //    {    0.0f, 150.0f, 0.5f, 0xff00ff00, },
    //    {  300.0f,   0.0f, 0.5f, 0xff0000ff, },
    //};

    //if( FAILED( m_pd3dDevice->CreateVertexBuffer( 6*sizeof(CUSTOMVERTEX),
    //                                              0, D3DFVF_CUSTOMVERTEX,
    //                                              D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    //{
    //    assert(0);
    //}

//pk
    CUSTOMVERTEX  *triangle_strip;
    #if defined(_DX8)
    if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&triangle_strip, D3DLOCK_DISCARD ) ) )
    #else #if defined(_DX9)
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&triangle_strip, D3DLOCK_DISCARD ) ) )
    #endif
    {
      assert(0);
    }


      triangle_strip[0].x = x0;
      triangle_strip[0].y = y0;
      triangle_strip[0].z = Z_DEPTH;
      triangle_strip[0].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      triangle_strip[1].x = x1;
      triangle_strip[1].y = y0;
      triangle_strip[1].z = Z_DEPTH;
      triangle_strip[1].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      triangle_strip[2].x = x0;
      triangle_strip[2].y = y1;
      triangle_strip[2].z = Z_DEPTH;
      triangle_strip[2].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      triangle_strip[3].x = x1;
      triangle_strip[3].y = y1;
      triangle_strip[3].z = Z_DEPTH;
      triangle_strip[3].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      triangle_strip[4].x = x0;
      triangle_strip[4].y = y1;
      triangle_strip[4].z = Z_DEPTH;
      triangle_strip[4].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      triangle_strip[5].x = x1;
      triangle_strip[5].y = y0;
      triangle_strip[5].z = Z_DEPTH;
      triangle_strip[5].color = D3DCOLOR_ARGB( m_curColor.m_a,  m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);

      g_pVB->Unlock();

        m_pd3dDevice->SetStreamSource( 0, g_pVB, 
          #ifdef _DX9
          0, 
          #endif
          sizeof(CUSTOMVERTEX) );
        
        renderState->SetFVF( D3DFVF_CUSTOMVERTEX );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2);

    }
  }


  void  end_display()
  // Clean up after rendering a frame.  Client program is still
  // responsible for calling glSwapBuffers() or whatever.
  {
    // End the scene
    m_pd3dDevice->EndScene();

    // Present the backbuffer contents to the display
//    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );

    //m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
    //renderState->SetSamplerState( 0, NISAMP_MIPFILTER, D3DTEXF_LINEAR, false );

    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat);
//    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &ident);

    m_pd3dDevice->SetTransform(D3DTS_VIEW, &mat);


  }


  void  set_matrix(const gameswf::matrix& m)
  // Set the current transform for mesh & line-strip rendering.
  {
    m_current_matrix = m;
  }


  void  set_cxform(const gameswf::cxform& cx)
  // Set the current color transform for mesh & line-strip rendering.
  {
    m_current_cxform = cx;
  }
  
  void  apply_matrix(const gameswf::matrix& mat_in)
  // Set the given transformation matrix.
  {
    gameswf::matrix m(m_viewport_matrix);
    m.concatenate(mat_in);

    D3DXMATRIX  mat,mat1;
    D3DXMatrixIdentity( &mat );
    // row 0
    mat._11 = m.m_[0][0];             mat._12 = m.m_[1][0];  mat._13 = 0.00f; mat._14 = 0.00f;
    mat._21 = m.m_[0][1];             mat._22 = m.m_[1][1];  mat._23 = 0.00f; mat._24 = 0.00f;
    mat._31 = 0.00f;                  mat._32 = 0.00f;                    mat._33 = 1.00f; mat._34 = 0.00f;
    mat._41 = m.m_[0][2];             mat._42 = m.m_[1][2];  mat._43 = 0.00f; mat._44 = 1.00f;

//pk 7-11    mat = mat * modelViewMatrix;

    coordTransform = mat * projMatrix;

//pk 7-11    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
  }

  static void apply_color(const gameswf::rgba& c)
  // Set the given color.
  {
    renderState->SetRenderState( D3DRS_COLORVERTEX, TRUE);
    renderState->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);

    m_curColor = c;
  }

  void  fill_style_disable(int fill_side)
  // Don't fill on the {0 == left, 1 == right} side of a path.
  {
    assert(fill_side >= 0 && fill_side < 2);

    m_current_styles[fill_side].disable();
  }


  void  line_style_disable()
  // Don't draw a line on this path.
  {
    m_current_styles[LINE_STYLE].disable();
  }


  void  fill_style_color(int fill_side, gameswf::rgba color)
  // Set fill style for the left interior of the shape.  If
  // enable is false, turn off fill for the left interior.
  {
    assert(fill_side >= 0 && fill_side < 2);

    m_current_styles[fill_side].set_color(m_current_cxform.transform(color));
  }


  void  line_style_color(gameswf::rgba color)
  // Set the line style of the shape.  If enable is false, turn
  // off lines for following curve segments.
  {
    m_current_styles[LINE_STYLE].set_color(m_current_cxform.transform(color));
  }


  void  fill_style_bitmap(int fill_side, const gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm)
  {
    assert(fill_side >= 0 && fill_side < 2);
    m_current_styles[fill_side].set_bitmap(bi, m, wm, m_current_cxform);
  }
  
  void  line_style_width(float width)
  {
    // WK: what to do here???
  }


  void  draw_mesh_strip(const void* coords, int vertex_count)
  {
//    assert(vertex_count <= 4);
    Sint16 *coords16 = (Sint16*) coords;

    // Set up current style.
    m_current_styles[LEFT_STYLE].apply(m_current_matrix);

    apply_matrix(m_current_matrix);

    CUSTOMVERTEX  *triangle_strip;
    #if defined(_DX8)
    if( FAILED( g_pVB->Lock( 0, 0, 
      (BYTE**)&triangle_strip, 
      D3DLOCK_DISCARD ) ) )
    #else #if defined(_DX9)
    if( FAILED( g_pVB->Lock( 0, 0, 
      (void**)&triangle_strip, 
      D3DLOCK_DISCARD ) ) )
    #endif
    {
      assert(0);
    }

    for (int i = 0; i < vertex_count; i++)
    {
      triangle_strip[i].x = coords16[ i * 2 ];
      triangle_strip[i].y = coords16[ i * 2 + 1 ];
      triangle_strip[i].z = Z_DEPTH;
      triangle_strip[i].color = D3DCOLOR_ARGB( m_curColor.m_a,  m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    }


    g_pVB->Unlock();


    m_pd3dDevice->SetStreamSource( 0, g_pVB, 
      #ifdef _DX9
      0, 
      #endif
      sizeof(CUSTOMVERTEX) );
    renderState->SetFVF(D3DFVF_CUSTOMVERTEX, false );
    assert( vertex_count < MAX_SIZE_VERTEX_BUFFER );
    HRESULT result = m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, vertex_count - 2);

    if (FAILED( result ))
    {
      assert(0);
    }

    if (m_current_styles[LEFT_STYLE].needs_second_pass())
    {
      assert(0);
      // 2nd pass, if necessary.
      m_current_styles[LEFT_STYLE].apply_second_pass();
      m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vertex_count - 2, coords, sizeof(CUSTOMVERTEX2));
      m_current_styles[LEFT_STYLE].cleanup_second_pass();
    }

    D3DXMATRIX  mat;
    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat);
//    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
    renderState->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                            D3DTTFF_DISABLE );
    renderState->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

  }


  void  draw_line_strip(const void* coords, int vertex_count)
  // Draw the line strip formed by the sequence of points.
  {
    Sint16 *coords16 = (Sint16*) coords;

    // Set up current style.
    m_current_styles[LINE_STYLE].apply(m_current_matrix);

    apply_matrix(m_current_matrix);


    CUSTOMVERTEX  *triangle_strip;
    #if defined(_DX8)
    if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&triangle_strip, D3DLOCK_DISCARD ) ) )
    #else #if defined(_DX9)
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&triangle_strip, D3DLOCK_DISCARD ) ) )
    #endif
    {
      assert(0);
    }

    for (int i = 0; i < vertex_count; i++)
    {
      triangle_strip[i].x = coords16[ i * 2 ];
      triangle_strip[i].y = coords16[ i * 2 + 1 ];
      triangle_strip[i].z = Z_DEPTH;
      triangle_strip[i].color = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    }

    g_pVB->Unlock();


    m_pd3dDevice->SetStreamSource( 0, g_pVB, 
      #ifdef _DX9
      0, 
      #endif
      sizeof(CUSTOMVERTEX) );
//    m_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
    renderState->SetFVF(D3DFVF_CUSTOMVERTEX, false );
    HRESULT result = m_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, vertex_count-1);


//    m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, vertex_count - 1, coords, sizeof(Sint16) * 2);
//    HRESULT result = m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, vertex_count - 1, (const void*)triangle_strip, sizeof(CUSTOMVERTEX));
    if (FAILED( result ))
    {
      assert(0);
    }

    D3DXMATRIX  mat;
    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
//    delete triangle_strip;
  }


  void  draw_bitmap(
    const gameswf::matrix& m,
    const gameswf::bitmap_info* bi,
    const gameswf::rect& coords,
    const gameswf::rect& uv_coords,
    gameswf::rgba color)
  // Draw a rectangle textured with the given bitmap, with the
  // given color.  Apply given transform; ignore any currently
  // set transforms.
  //
  // Intended for textured glyph rendering.
  {
    assert(bi);

    apply_color(color);

    gameswf::point a, b, c, d;
    m.transform(&a, gameswf::point(coords.m_x_min, coords.m_y_min));
    m.transform(&b, gameswf::point(coords.m_x_max, coords.m_y_min));
    m.transform(&c, gameswf::point(coords.m_x_min, coords.m_y_max));
    d.m_x = b.m_x + c.m_x - a.m_x;
    d.m_y = b.m_y + c.m_y - a.m_y;



    // Set texture.
    m_pd3dDevice->SetTexture(0, s_d3d_textures[bi->m_texture_id]);
//    m_pd3dDevice->SetTexture(0, g_pTexture);
    renderState->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
    renderState->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    renderState->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    renderState->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    renderState->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    renderState->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
    renderState->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
    renderState->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);


    // @@ TODO this is wrong; needs fixing!  Options:
    //
    // * compute texgen parameters for the bitmap
    //
    // * change to a vshader which passes the texcoords through

    // No texgen; just pass through.

    // Draw the quad.

//pk
    CUSTOMVERTEX2 *pVertices;
    HRESULT result;

#if 1
    #if defined(_DX8)
    if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD ) ) )
    #else #if defined(_DX9)
    if( FAILED( g_pVB2->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD ) ) )
    #endif
    {
      assert(0);
    }

    pVertices[0].x = a.m_x ; 
    pVertices[0].y = a.m_y ;
    pVertices[0].z = Z_DEPTH;
    pVertices[0].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[0].tu       = uv_coords.m_x_min ;
    pVertices[0].tv       = uv_coords.m_y_min ;

    pVertices[1].x = b.m_x ; 
    pVertices[1].y = b.m_y ;
    pVertices[1].z = Z_DEPTH;
    pVertices[1].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[1].tu       = uv_coords.m_x_max ;
    pVertices[1].tv       = uv_coords.m_y_min ;

    pVertices[2].x = c.m_x ; 
    pVertices[2].y = c.m_y ;
    pVertices[2].z = Z_DEPTH;
    pVertices[2].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[2].tu       = uv_coords.m_x_min ;
    pVertices[2].tv       = uv_coords.m_y_max ;

    pVertices[3].x = d.m_x ; 
    pVertices[3].y = d.m_y ;
    pVertices[3].z = Z_DEPTH;
    pVertices[3].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[3].tu       = uv_coords.m_x_max ;
    pVertices[3].tv       = uv_coords.m_y_max ;

    g_pVB2->Unlock();

    // Render the vertex buffer contents
    m_pd3dDevice->SetStreamSource( 0, g_pVB2, 
      #ifdef _DX9
      0, 
      #endif
      sizeof(CUSTOMVERTEX2) );
    renderState->SetFVF(D3DFVF_CUSTOMVERTEX2, false );
    result = m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    if (FAILED(result))
    {
      assert(0);
    }
#endif

#if 0

    #if defined(_DX8)
    if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD ) ) )
    #else #if defined(_DX9)
    if( FAILED( g_pVB2->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD ) ) )
    #endif
    {
      assert(0);
    }

    pVertices[0].x = 0.0f; 
    pVertices[0].y = 4000.0f ;
    pVertices[0].z = 0.5f;
    pVertices[0].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    //pVertices[0].color    = D3DCOLOR_ARGB( 0xff, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[0].tu       = 0.0f;
    pVertices[0].tv       = 1.0f;

    pVertices[1].x = 0.0f ; 
    pVertices[1].y = 0.0f ;
    pVertices[1].z = 0.5f;
    pVertices[1].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
//    pVertices[1].color    = D3DCOLOR_ARGB( 0xff, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[1].tu       = 0.0f ;
    pVertices[1].tv       = 0.0f ;

    pVertices[2].x = 4000.0f ; 
    pVertices[2].y = 4000.0f ;
    pVertices[2].z = 0.5f;
    pVertices[2].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
//    pVertices[2].color    = D3DCOLOR_ARGB( 0xff, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[2].tu       = 1.0f ;
    pVertices[2].tv       = 1.0f ;

    pVertices[3].x = 4000.0f ; 
    pVertices[3].y = 0.0f ;
    pVertices[3].z = 0.5f;
    pVertices[3].color    = D3DCOLOR_ARGB( m_curColor.m_a, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
//    pVertices[3].color    = D3DCOLOR_ARGB( 0xff, m_curColor.m_r, m_curColor.m_g, m_curColor.m_b);
    pVertices[3].tu       = 1.0f ;
    pVertices[3].tv       = 0.0f ;


    g_pVB2->Unlock();



    // Render the vertex buffer contents
    m_pd3dDevice->SetStreamSource( 0, g_pVB2, 
      #ifdef _DX9
      0, 
      #endif
      sizeof(CUSTOMVERTEX2) );
//    m_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX2 );
    renderState->SetFVF(D3DFVF_CUSTOMVERTEX2, false );
    result = m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    if (FAILED(result))
    {
      assert(0);
    }

#endif

  }
  
  void begin_submit_mask()
  {

    renderState->SetRenderState(
      D3DRS_ZWRITEENABLE,
                FALSE, false );

    // Enable stencil testing
    renderState->SetRenderState(D3DRS_STENCILENABLE, true, false);

    //renderState->SetRenderState(
    //  D3DRS_ZWRITEENABLE,
    //            FALSE, false );


    // Clear stencil buffer values to zero
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0f, 0);

    // Specify the stencil comparison function
    renderState->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS, false);

    // Set the comparison reference value
    renderState->SetRenderState(D3DRS_STENCILREF, 1, false);

    //  Specify a stencil mask 
    renderState->SetRenderState(D3DRS_STENCILMASK, 0x1 , false);

    // A write mask controls what is written
    renderState->SetRenderState(D3DRS_STENCILWRITEMASK, 0x1, false);

    renderState->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP, false);

    renderState->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE, false);

    renderState->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP, false);

#if 0
    glEnable(GL_STENCIL_TEST); 
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glColorMask(0,0,0,0); // disable framebuffer writes
    glEnable(GL_STENCIL_TEST);  // enable stencil buffer for "marking" the mask
    glStencilFunc(GL_ALWAYS, 1, 1); // always passes, 1 bit plane, 1 as mask
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // we set the stencil buffer to 1 where we draw any polygon
              // keep if test fails, keep if test passes but buffer test fails
              // replace if test passes 
#endif // 0
  }
  
  void end_submit_mask()
  {

    // Specify when to write stencil data
    renderState->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP, false);

    renderState->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP, false);

    renderState->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP, false);

    // Specify the stencil comparison function
    renderState->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL, false);

    renderState->SetRenderState(
      D3DRS_ZWRITEENABLE,
      TRUE, false );

#if 0
    glColorMask(1,1,1,1); // enable framebuffer writes
    glStencilFunc(GL_EQUAL, 1, 1);  // we draw only where the stencil is 1 (where the mask was drawn)
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // don't change the stencil buffer    
#endif // 0
  }
  
  void disable_mask()
  {
    renderState->SetRenderState(D3DRS_STENCILENABLE, FALSE, false);

#if 0
      glDisable(GL_STENCIL_TEST); 
#endif // 0
  }
  
};  // end struct render_handler_d3d


// bitmap_info_d3d implementation


bitmap_info_d3d::bitmap_info_d3d()
//bitmap_info_d3d::bitmap_info_d3d(create_empty e)
{
  // A null texture.  Needs to be initialized later.
  m_texture_id = 0;
  m_original_width = 0;
  m_original_height = 0;
}


bitmap_info_d3d::bitmap_info_d3d(image::rgb* im)
// Image with no alpha.
{
  assert(im);

  // Rescale.
  m_original_width = im->m_width;
  m_original_height = im->m_height;

  int w = 1; while (w < im->m_width) { w <<= 1; }
  int h = 1; while (h < im->m_height) { h <<= 1; }

  image::rgb* rescaled = image::create_rgb(w, h);
  image::resample(rescaled, 0, 0, w - 1, h - 1,
      im, 0, 0, (float) im->m_width, (float) im->m_height);

  // Need to insert a dummy alpha byte in the image data, for
  // D3DXLoadSurfaceFromMemory.
  // @@ this sucks :(
  int pixel_count = w * h;
  Uint8*  expanded_data = new Uint8[pixel_count * 4];
  for (int y = 0; y < h; y++)
  {
    Uint8*  scanline = image::scanline(rescaled, y);
    for (int x = 0; x < w; x++)
    {
      expanded_data[((y * w) + x) * 4 + 3] = 255; // alpha
      expanded_data[((y * w) + x) * 4 + 2] = scanline[x * 3 + 0]; // red
      expanded_data[((y * w) + x) * 4 + 1] = scanline[x * 3 + 1]; // green
      expanded_data[((y * w) + x) * 4 + 0] = scanline[x * 3 + 2]; // blue
    }
  }

  // Create the texture.
  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

#if defined (_DX8)
  IDirect3DTexture8*  tex;
#else #if defined (_DX9)
  IDirect3DTexture9*  tex;
#endif
  #if defined (_DX8)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    D3DUSAGE_DYNAMIC,      // Usage
    D3DFMT_A8R8G8B8,  // Format
//    D3DFMT_DXT1,
    D3DPOOL_DEFAULT,
    &tex);
  #else #if defined(_DX9)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    D3DUSAGE_DYNAMIC,      // Usage
    D3DFMT_A8R8G8B8,  // Format
//    D3DFMT_DXT1,
    D3DPOOL_DEFAULT,
    &tex,
    NULL);
  #endif
  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

#if defined (_DX8)
  IDirect3DSurface8*  surf = NULL;
#else #if defined (_DX9)
  IDirect3DSurface9*  surf = NULL;
#endif
  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  RECT  source_rect;
  source_rect.left = 0;
  source_rect.top = 0;
  source_rect.right = w;
  source_rect.bottom = h;
  D3DLOCKED_RECT rectLock;

  surf->LockRect(&rectLock,NULL,D3DLOCK_DISCARD);
 
  for (unsigned i=0; i < h; i++)
  {
    memcpy((unsigned char *)rectLock.pBits+(i*4*w),expanded_data + (i*4 *w),4* w);
  }
  surf->UnlockRect();

  delete [] expanded_data;
  if (result != S_OK)
  {
    gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
    return;
  }

//  if (surf) { surf->Release(); }
#if 0
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, (GLuint*)&m_texture_id);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /* LINEAR_MIPMAP_LINEAR */);

  m_original_width = im->m_width;
  m_original_height = im->m_height;

  int w = 1; while (w < im->m_width) { w <<= 1; }
  int h = 1; while (h < im->m_height) { h <<= 1; }

  image::rgb* rescaled = image::create_rgb(w, h);
  image::resample(rescaled, 0, 0, w - 1, h - 1,
      im, 0, 0, (float) im->m_width, (float) im->m_height);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rescaled->m_data);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rescaled->m_width, rescaled->m_height, GL_RGB, GL_UNSIGNED_BYTE, rescaled->m_data);

  delete rescaled;
#endif // 0
}

typedef struct
{
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} RGBA;

void bitmap_info_d3d::convert_to_argb(image::rgba* im)
{
  for (int h = 0; h < im->m_height; h++)
  {
    for (int w = 0; w < im->m_width; w++)
    {
      RGBA c;
      c.r = im->m_data[((h * im->m_width) + w ) * 4];
      c.g = im->m_data[(((h * im->m_width) + w ) * 4) + 1];
      c.b = im->m_data[(((h * im->m_width) + w ) * 4) + 2];
      c.a = im->m_data[(((h * im->m_width) + w ) * 4) + 3];
      im->m_data[((h * im->m_width) + w ) * 4 + 3] = c.a;
      im->m_data[(((h * im->m_width) + w ) * 4) + 2] = c.r;
      im->m_data[(((h * im->m_width) + w ) * 4) + 1] = c.g;
      im->m_data[(((h * im->m_width) + w ) * 4) + 0] = c.b;
    }
  }
}

bitmap_info_d3d::bitmap_info_d3d(image::rgba* im)
// Version of the constructor that takes an image with alpha.
{
  assert(im);

  convert_to_argb(im);

  m_original_width = im->m_width;
  m_original_height = im->m_height;

  int w = 1; while (w < im->m_width) { w <<= 1; }
  int h = 1; while (h < im->m_height) { h <<= 1; }

  // Create the texture.
  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

#if defined (_DX8)
  IDirect3DTexture8*  tex;
#else #if defined (_DX9)
  IDirect3DTexture9*  tex;
#endif
//original  //HRESULT result = m_pd3dDevice->CreateTexture(
  //  w,
  //  h,
  //  0,
  //  D3DUSAGE_BORDERSOURCE_TEXTURE,
  //  D3DFMT_DXT1,
  //  NULL,
  //  &tex);
  #if defined(_DX8)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    0,      // Usage
    D3DFMT_A8R8G8B8,  // Format
    D3DPOOL_DEFAULT,
    &tex);
  #else #if (_DX9)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    D3DUSAGE_DYNAMIC,      // Usage
    D3DFMT_A8R8G8B8,  // Format
    D3DPOOL_DEFAULT,
    &tex,
    NULL);
  #endif
  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

#if defined (_DX8)
  IDirect3DSurface8*  surf = NULL;
#else if defined (_DX9)
  IDirect3DSurface9*  surf = NULL;
#endif

  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  RECT  source_rect;
  source_rect.left = 0;
  source_rect.top = 0;
  source_rect.right = w;
  source_rect.bottom = h;

  // Set the actual data.
  if (w != im->m_width
      || h != im->m_height)
  {
    image::rgba*  rescaled = image::create_rgba(w, h);
    image::resample(rescaled, 0, 0, w - 1, h - 1,
        im, 0, 0, (float) im->m_width, (float) im->m_height);

    D3DLOCKED_RECT rectLock;
    HRESULT retVal = surf->LockRect(&rectLock,NULL,0);// D3DLOCK_DISCARD);

    for (unsigned i=0; i < h; i++)
    {
      memcpy((unsigned char *)rectLock.pBits+(i*4*w),rescaled->m_data + (i*4 *w),4* w);
    }
    surf->UnlockRect();

    if (result != S_OK)
    {
      gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
      return;
    }
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaled->m_data);

    delete  rescaled;
  }
  else
  {
    // Use original image directly.
    D3DLOCKED_RECT rectLock;
    surf->LockRect(&rectLock,NULL,D3DLOCK_DISCARD);

    for (unsigned i=0; i < h; i++)
    {
      memcpy((unsigned char *)rectLock.pBits+(i*im->m_pitch),im->m_data + (i*im->m_pitch),4* w);
    }
    surf->UnlockRect();

    if (result != S_OK)
    {
      gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
      return;
    }
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->m_data);
  }

  if (surf) { surf->Release(); }

#if 0
  // Create the texture.

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, (GLuint*)&m_texture_id);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST ?
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /* LINEAR_MIPMAP_LINEAR */);

  m_original_width = im->m_width;
  m_original_height = im->m_height;

  int w = 1; while (w < im->m_width) { w <<= 1; }
  int h = 1; while (h < im->m_height) { h <<= 1; }

  if (w != im->m_width
      || h != im->m_height)
  {
    image::rgba*  rescaled = image::create_rgba(w, h);
    image::resample(rescaled, 0, 0, w - 1, h - 1,
        im, 0, 0, (float) im->m_width, (float) im->m_height);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaled->m_data);

    delete  rescaled;
  }
  else
  {
    // Use original image directly.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->m_data);
  }
#endif // 0
}


bitmap_info_d3d::bitmap_info_d3d(int width, int height, Uint8* data)
// Initialize this bitmap_info to an alpha image
// containing the specified data (1 byte per texel).
//
// !! Munges *data in order to create mipmaps !!
{
  assert(m_texture_id == 0);  // only call this on an empty bitmap_info
  assert(data);
  
  // Create the texture.
  
  m_original_width = width;
  m_original_height = height;

  // You must use power-of-two dimensions!!
  int w = 1; while (w < width) { w <<= 1; }
  int h = 1; while (h < height) { h <<= 1; }
  assert(w == width);
  assert(h == height);

  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

#if defined (_DX8)
  IDirect3DTexture8*  tex;
#else #if defined (_DX9)
  IDirect3DTexture9*  tex;
#endif

//original  //HRESULT result = m_pd3dDevice->CreateTexture(
  //  width,
  //  height,
  //  0,
  //  D3DUSAGE_BORDERSOURCE_TEXTURE,
  //  D3DFMT_A8,
  //  NULL,
  //  &tex);
  #if defined(_DX8)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    D3DUSAGE_DYNAMIC,      // Usage
    D3DFMT_A8,  // Format
    D3DPOOL_DEFAULT,
    &tex);
  #else #if defined(_DX9)
  HRESULT result = m_pd3dDevice->CreateTexture(
    w,
    h,
    1,
    D3DUSAGE_DYNAMIC,      // Usage
    D3DFMT_A8,  // Format
    D3DPOOL_DEFAULT,
    &tex,
    NULL);
  #endif
  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

#if defined (_DX8)
  IDirect3DSurface8*  surf = NULL;
#else #if defined (_DX9)
  IDirect3DSurface9*  surf = NULL;
#endif

  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  D3DLOCKED_RECT rectLock;

   surf->LockRect(&rectLock,NULL,D3DLOCK_DISCARD);
 
  for (unsigned i=0; i < height; i++)
  {
    memcpy((unsigned char *)rectLock.pBits+i*width,data +
      i * width,width);
  }
  surf->UnlockRect();

  if (result != S_OK)
  {
    gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
    return;
  }

  if (surf) { surf->Release(); }

//  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

//  // Build mips.
//  int level = 1;
//  while (width > 1 || height > 1)
//  {
//    render_handler_d3d::make_next_miplevel(&width, &height, data);
//    glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
//    level++;
//  }

#if 0
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, (GLuint*)&m_texture_id);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST ?
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  m_original_width = width;
  m_original_height = height;

  #ifndef NDEBUG
  // You must use power-of-two dimensions!!
  int w = 1; while (w < width) { w <<= 1; }
  int h = 1; while (h < height) { h <<= 1; }
  assert(w == width);
  assert(h == height);
  #endif // not NDEBUG

  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

  // Build mips.
  int level = 1;
  while (width > 1 || height > 1)
  {
    render_handler_d3d::make_next_miplevel(&width, &height, data);
    glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    level++;
  }
#endif // 0
}


  
gameswf::render_handler*  gameswf::create_render_handler_d3d(HWND hWnd, LPDIRECT3DDEVICE9 device)
// Factory.
{
  render_handler_d3d *hndlr = new render_handler_d3d( hWnd, device );
  hndlr->init(hWnd);
  return hndlr;
}


gameswf::rgba render_handler_d3d::m_curColor;  // the current pipeline color




// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
