// gameswf_render_handler_d3d.cpp  Paul Kelly & Bob Ives, 2006

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A gameswf::render_handler that uses DX8 or DX9.  
// Based on the xbox code by Thatcher Ulrich <http://tulrich.com> 2003


#include <windows.h>
#include <cassert>
#include "gameswf.h"
#include "gameswf/gameswf_log.h"
#include "gameswf_types.h"
#include "base/image.h"
#include "base/container.h"
#include "render_handler_tri.h"

//#define _DX8

#if defined (_DX8)
# include <d3d8.h>
#else
# include <d3d9.h>
#endif

#if DIRECT3D_VERSION < 0x0900
# include <d3dx8tex.h>
# include <d3d8types.h>
  typedef IDirect3D8              IDirect3D;
  typedef IDirect3DDevice8        IDirect3DDevice;
  typedef IDirect3DTexture8       IDirect3DTexture;
  typedef IDirect3DBaseTexture8   IDirect3DBaseTexture;
  typedef IDirect3DSurface8       IDirect3DSurface;
  typedef IDirect3DVertexBuffer8  IDirect3DVertexBuffer;
  typedef D3DVIEWPORT8            D3DVIEWPORT;
  typedef BYTE                    tLock;
#else 
# include <d3dx9tex.h>
# include <d3d9types.h>
  typedef IDirect3D9              IDirect3D;
  typedef IDirect3DDevice9        IDirect3DDevice;
  typedef IDirect3DTexture9       IDirect3DTexture;
  typedef IDirect3DBaseTexture9   IDirect3DBaseTexture;
  typedef IDirect3DSurface9       IDirect3DSurface;
  typedef IDirect3DVertexBuffer9  IDirect3DVertexBuffer;
  typedef D3DVIEWPORT9            D3DVIEWPORT;
  typedef void                    tLock;
#endif

#include <cstring>

#define INIT_SIZE_VERTEX_BUFFER  128

#define Z_DEPTH 1.0f


// A structure for our custom vertex type. We added texture coordinates
struct CUSTOMVERTEX
{
  float x,y,z; // transformed position for the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ)

struct CUSTOMVERTEX2
{
  float x,y,z;    // position
  float tu, tv;   // texture coordinates
};

#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZ|D3DFVF_TEX1)


namespace
{
  std::vector<IDirect3DBaseTexture*> s_d3d_textures;
};

static inline float clamp( float x,float min,float max ) {
  union { float f; int hex; };
  f = x - min;
  hex &= ~hex>>31;
  f += min - max;
  hex &= hex>>31;
  f += max;
  return f;
}


// bitmap_info_d3d declaration
class bitmap_info_d3d : public gameswf::bitmap_info
{
public:
  bitmap_info_d3d();
  void convert_to_argb(image::rgba* im);
  bitmap_info_d3d(image::rgb* im);
  bitmap_info_d3d(image::rgba* im);
  bitmap_info_d3d(int width, int height, uint8_t* data);
};


class render_handler_d3d : public gameswf::triangulating_render_handler
{
public
  // Some renderer state.

  gameswf::matrix m_current_matrix;
  gameswf::cxform m_current_cxform;

  IDirect3DVertexBuffer* m_pVB; // Buffer to hold vertices
  IDirect3DVertexBuffer* m_pVB2; // Buffer to hold vertices
  DWORD m_nMaxVertices;  // Max Vertices held in m_pVB
  D3DVIEWPORT m_origVP;

  static IDirect3DDevice* m_pd3dDevice; // Our rendering device
  static D3DFORMAT m_FormatRGB;
  static D3DFORMAT m_FormatRGBA;
  static D3DFORMAT m_FormatA;
  static D3DXMATRIX m_ModelViewMatrix;
  static D3DXMATRIX m_ProjMatrix;

  void set_antialiased(bool enable)
  {
    // not supported
  }

  static void make_next_miplevel(int* width, int* height, uint8_t* data)
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
        uint8_t*  out = ((uint8_t*) data) + j * new_w;
        uint8_t*  in = ((uint8_t*) data) + (j << 1) * *width;
        for (int i = 0; i < new_w; i++) {
          int a;
          a = (*(in + 0) + *(in + 1) + *(in + 0 + *width) + *(in + 1 + *width));
          *(out) = (uint8_t) (a >> 2);
          out++;
          in += 2;
        }
      }
    }

    // Munge parameters to reflect the shrunken image.
    *width = new_w;
    *height = new_h;
  }

  class fill_style
  {
  public:
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
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
      }
      else if (m_mode == BITMAP_WRAP
        || m_mode == BITMAP_CLAMP)
      {
        assert(m_bitmap_info != NULL);

        apply_color(m_color);

        if (m_bitmap_info == NULL)
        {
          abort();
          m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
          m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        }
        else
        {

          // Set up the texture for rendering.

          // Do the modulate part of the color
          // transform in the first pass.  The
          // additive part, if any, needs to
          // happen in a second pass.

          m_pd3dDevice->SetTexture(0, s_d3d_textures[m_bitmap_info->m_texture_id]);
          m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
          m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

          if (m_mode == BITMAP_CLAMP)
          {
#if DIRECT3D_VERSION >= 0x0900
            m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
            m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP); 
            m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif
          }
          else
          {
            assert(m_mode == BITMAP_WRAP);
#if DIRECT3D_VERSION >= 0x0900
            m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
            m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP); 
            m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif
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

          m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat );
          m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
          m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
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
      assert(needs_second_pass());

      // Additive color.
      apply_color(gameswf::rgba(
        uint8_t(m_bitmap_color_transform.m_[0][1]), 
        uint8_t(m_bitmap_color_transform.m_[1][1]), 
        uint8_t(m_bitmap_color_transform.m_[2][1]), 
        uint8_t(m_bitmap_color_transform.m_[3][1])));

      m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE );
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
    }

    void  cleanup_second_pass() const
    {
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }


    void  disable() { m_mode = INVALID; }
    void  set_color(gameswf::rgba color) 
    { 
      m_mode = COLOR; 
      m_color = color; 
    }
    void  set_bitmap(const gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
    {
      m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
      m_bitmap_info = bi;
      m_bitmap_matrix = m;

      m_bitmap_color_transform.m_[0][0] = clamp(color_transform.m_[0][0], 0, 1);
      m_bitmap_color_transform.m_[1][0] = clamp(color_transform.m_[1][0], 0, 1);
      m_bitmap_color_transform.m_[2][0] = clamp(color_transform.m_[2][0], 0, 1);
      m_bitmap_color_transform.m_[3][0] = clamp(color_transform.m_[3][0], 0, 1);

      m_bitmap_color_transform.m_[0][1] = clamp(color_transform.m_[0][1], -255.0f, 255.0f);
      m_bitmap_color_transform.m_[1][1] = clamp(color_transform.m_[1][1], -255.0f, 255.0f);
      m_bitmap_color_transform.m_[2][1] = clamp(color_transform.m_[2][1], -255.0f, 255.0f);
      m_bitmap_color_transform.m_[3][1] = clamp(color_transform.m_[3][1], -255.0f, 255.0f);
      
      m_color = gameswf::rgba(
        uint8_t(m_bitmap_color_transform.m_[0][0]*255.0f), 
        uint8_t(m_bitmap_color_transform.m_[1][0]*255.0f), 
        uint8_t(m_bitmap_color_transform.m_[2][0]*255.0f), 
        uint8_t(m_bitmap_color_transform.m_[3][0]*255.0f));

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

  // Constructor.
  render_handler_d3d(IDirect3DDevice* device)
    : m_pVB(NULL)
    , m_pVB2(NULL)
    , m_nMaxVertices(INIT_SIZE_VERTEX_BUFFER)
  {
    HRESULT hr;

    m_pd3dDevice = device;
    m_pd3dDevice->AddRef();

    hr = m_pd3dDevice->CreateVertexBuffer( m_nMaxVertices*sizeof(CUSTOMVERTEX)
      , D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVB
#if DIRECT3D_VERSION >= 0x0900
      , NULL 
#endif
      );
    assert(hr==S_OK);

    hr = m_pd3dDevice->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX2)
      , D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX2, D3DPOOL_DEFAULT, &m_pVB2
#if DIRECT3D_VERSION >= 0x0900
      , NULL 
#endif
      );
    assert(hr==S_OK);

    // Determine texture formats
    IDirect3D* l_pD3D;
    hr = m_pd3dDevice->GetDirect3D( &l_pD3D );
    assert(hr==S_OK);

#if DIRECT3D_VERSION < 0x0900
    D3DCAPS8 l_DeviceCaps;
#else
    D3DCAPS9 l_DeviceCaps;
#endif
    ZeroMemory( &l_DeviceCaps, sizeof(l_DeviceCaps) );  
    hr = m_pd3dDevice->GetDeviceCaps( &l_DeviceCaps );
    assert(hr==S_OK);

    D3DDISPLAYMODE l_DisplayMode;
    hr = m_pd3dDevice->GetDisplayMode( 
#if DIRECT3D_VERSION >= 0x0900
      0, 
#endif
      &l_DisplayMode );

    if (SUCCEEDED(l_pD3D->CheckDeviceFormat(l_DeviceCaps.AdapterOrdinal, l_DeviceCaps.DeviceType, l_DisplayMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1))) 
      m_FormatRGB = D3DFMT_DXT1;
    else 
      m_FormatRGB = D3DFMT_R8G8B8;

      if (SUCCEEDED(l_pD3D->CheckDeviceFormat(l_DeviceCaps.AdapterOrdinal, l_DeviceCaps.DeviceType, l_DisplayMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5))) 
        m_FormatRGBA = D3DFMT_DXT5;
      else
        m_FormatRGBA  = D3DFMT_A8R8G8B8;

    if (SUCCEEDED(l_pD3D->CheckDeviceFormat(l_DeviceCaps.AdapterOrdinal, l_DeviceCaps.DeviceType, l_DisplayMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8))) 
      m_FormatA = D3DFMT_A8;
    else if (SUCCEEDED(l_pD3D->CheckDeviceFormat(l_DeviceCaps.AdapterOrdinal, l_DeviceCaps.DeviceType, l_DisplayMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8L8))) 
      m_FormatA = D3DFMT_A8L8;
    else if (SUCCEEDED(l_pD3D->CheckDeviceFormat(l_DeviceCaps.AdapterOrdinal, l_DeviceCaps.DeviceType, l_DisplayMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8P8))) 
      m_FormatA = D3DFMT_A8P8;
    else 
      m_FormatA = D3DFMT_A8R8G8B8;

  }

  ~render_handler_d3d()
    // Destructor.
  {
    for (int i = 0; i < s_d3d_textures.size(); i++)
    {
      s_d3d_textures[i]->Release();
      s_d3d_textures[i] = 0;
    }


    if (m_pVB)
    {
      m_pVB->Release();
      m_pVB = 0;
    }

    if (m_pVB2)
    {
      m_pVB2->Release();
      m_pVB2 = 0;
    }

    if (m_pd3dDevice)
      m_pd3dDevice->Release();

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
    // Given an image, returns a pointer to a bitmap_info class
    // that can later be passed to fill_styleX_bitmap(), to set a
    // bitmap fill style.
  {
    return new bitmap_info_d3d(im);
  }


  gameswf::bitmap_info* create_bitmap_info_rgba(image::rgba* im)
    // Given an image, returns a pointer to a bitmap_info class
    // that can later be passed to fill_style_bitmap(), to set a
    // bitmap fill style.
    //
    // This version takes an image with an alpha channel.
  {
    return new bitmap_info_d3d(im);
  }

  void  set_alpha_image(gameswf::bitmap_info* bi, int w, int h, uint8_t* data)
    // Set the specified bitmap_info so that it contains an alpha
    // texture with the given data (1 byte per texel).
    //
    // Munges *data (in order to make mipmaps)!!
  {
    assert(bi);
    abort();  // not tested

    //pk    bi->set_alpha_image(w, h, data);
  }


  void  delete_bitmap_info(gameswf::bitmap_info* bi)
    // Delete the given bitmap info class.
  {
    delete bi;
  }

  void prepare_vertex_buffer(const int16_t* coords, int vertex_count)
  {
    HRESULT hr;

    if( vertex_count>(int)m_nMaxVertices )
    {
      // resize mesh
      m_pVB->Release();
      m_nMaxVertices = vertex_count;
      hr = m_pd3dDevice->CreateVertexBuffer( m_nMaxVertices*sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC
        , D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVB
#if DIRECT3D_VERSION >= 0x0900
        , NULL 
#endif
        );
      assert(hr==S_OK);
    }

    CUSTOMVERTEX* verts;
    hr = m_pVB->Lock( 0, 0, (tLock**)&verts, D3DLOCK_DISCARD );
    assert(hr==S_OK);
    int coord_count = vertex_count * 2;
    for(int i=0; i<coord_count; i+=2)
    {
      verts->x = coords[i];
      verts->y = coords[i+1];
      verts->z = Z_DEPTH;
      verts++;
    }
    hr = m_pVB->Unlock();
    assert(hr==S_OK);

    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB, 
#if DIRECT3D_VERSION >= 0x0900
      0,
#endif
     sizeof(CUSTOMVERTEX) );
    assert(hr==S_OK);

#if DIRECT3D_VERSION < 0x0900
    hr = m_pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
#else
    hr = m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
#endif
    assert(hr==S_OK);
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
    HRESULT hr;

    D3DXMatrixIdentity(&m_ModelViewMatrix);
    D3DXMatrixIdentity(&m_ProjMatrix);

    // invert coordinate system from lower left to upper left

    float gsWidthDiv  = 1.0f / viewport_width;
    float gsHeightDiv = 1.0f / viewport_height;

    m_ModelViewMatrix._11 = 2.0f / (x1 - x0);
    m_ModelViewMatrix._22 = -2.0f / (y1 - y0);
    m_ModelViewMatrix._41 = -((x1 + x0) / (x1 - x0));
    m_ModelViewMatrix._42 = ((y1 + y0) / (y1 - y0));

    m_ProjMatrix._11 = viewport_width * gsWidthDiv;
    m_ProjMatrix._22 = viewport_height * gsHeightDiv;
    m_ProjMatrix._41 = -1.0f + viewport_x0 * 2.0f * gsWidthDiv + viewport_width * gsWidthDiv;
    m_ProjMatrix._42 = 1.0f - viewport_y0 * 2.0f * gsHeightDiv - viewport_height * gsHeightDiv;

    // Matrix setup
    D3DXMATRIX  ident;
    D3DXMatrixIdentity(&ident);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_ModelViewMatrix);

    m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_ProjMatrix);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &ident);

    // turn on alpha bending.
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);

    if( NULL == m_pd3dDevice )
    {
      abort();
      return;
    }


    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);

    // Viewport.
    hr = m_pd3dDevice->GetViewport(&m_origVP);
    assert(SUCCEEDED(hr));  // pure device?

    D3DVIEWPORT  vp;
    vp.X = viewport_x0;
    vp.Y = viewport_y0;
    vp.Width = viewport_width;
    vp.Height = viewport_height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;   // minZ = maxZ = 0 forces all polys to be in foreground
    m_pd3dDevice->SetViewport(&vp);

    // Matrix to map from SWF movie (TWIPs) coords to
    // viewport coordinates.
    // Clear the backbuffer to a blue color
    //    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

    if( FAILED( m_pd3dDevice->BeginScene() ) )
    {
      // Rendering of scene objects can happen here

      // End the scene
      m_pd3dDevice->EndScene();
      abort();
    }


    // Blending renderstates
    //pk not sure if we need these.
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

    
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    // Textures off by default.
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

    // @@ for sanity's sake, let's turn of backface culling...
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);//xxxxx
    m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);  //xxxxx

    // Vertex format.
#if DIRECT3D_VERSION >= 0x0900
    m_pd3dDevice->SetVertexShader(NULL);
#endif

    // No pixel shader.
    m_pd3dDevice->SetPixelShader(NULL);

    // Clear the background, if background color has alpha > 0.
    if (background_color.m_a > 0)
    {
      if( background_color.m_a==255 )
      {
        D3DRECT rect = {LONG(x0),LONG(y0),LONG(x1),LONG(y1)};
        m_pd3dDevice->Clear(1, &rect,D3DCLEAR_TARGET,
          D3DCOLOR_XRGB(background_color.m_r, background_color.m_g, background_color.m_b),
          0.0f, 0 );
      }
      else
      {
        const int16_t backgroundCoords[] = {
          (int16_t)x0,(int16_t)y0,
          (int16_t)x1,(int16_t)y0,
          (int16_t)x0,(int16_t)y1,
          (int16_t)x1,(int16_t)y1};
        apply_color(background_color);
        set_matrix(gameswf::matrix::identity);
        apply_matrix(m_current_matrix);
        prepare_vertex_buffer(backgroundCoords, 4);
        HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        assert(hr==S_OK);
      }      
    }
  }


  void  end_display()
    // Clean up after rendering a frame.  Client program is still
    // responsible for calling glSwapBuffers() or whatever.
  {
    // End the scene
    m_pd3dDevice->EndScene();

    // Present the backbuffer contents to the display
    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );
    m_pd3dDevice->SetTransform(D3DTS_VIEW, &mat);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);

#if DIRECT3D_VERSION >= 0x0900
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif

    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );

    m_pd3dDevice->SetViewport(&m_origVP);
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

  void  apply_matrix(const gameswf::matrix& m)
    // Set the given transformation matrix.
  {
    D3DXMATRIX  mat;
    D3DXMatrixIdentity( &mat );
    // row 0
    mat._11 = m.m_[0][0];             mat._12 = m.m_[1][0];  mat._13 = 0.00f; mat._14 = 0.00f;
    mat._21 = m.m_[0][1];             mat._22 = m.m_[1][1];  mat._23 = 0.00f; mat._24 = 0.00f;
    mat._31 = 0.00f;                  mat._32 = 0.00f;                    mat._33 = 1.00f; mat._34 = 0.00f;
    mat._41 = m.m_[0][2];             mat._42 = m.m_[1][2];  mat._43 = 0.00f; mat._44 = 1.00f;

    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
  }

  static void apply_color(const gameswf::rgba& c)
    // Set the given color.
  {
    HRESULT hr = m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(c.m_a, c.m_r, c.m_g, c.m_b));
    assert(hr==S_OK);
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
    // Set up current style.
    m_current_styles[LEFT_STYLE].apply(m_current_matrix);

    apply_matrix(m_current_matrix);
    prepare_vertex_buffer((int16_t*)coords, vertex_count);
    HRESULT hr = m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, vertex_count - 2);
    assert(hr==S_OK);

    if (m_current_styles[LEFT_STYLE].needs_second_pass())
    {
      // 2nd pass, if necessary.
      m_current_styles[LEFT_STYLE].apply_second_pass();
      hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, vertex_count-2);
      assert(hr==S_OK);
      m_current_styles[LEFT_STYLE].cleanup_second_pass();
    }
  }


  void  draw_line_strip(const void* coords, int vertex_count)
    // Draw the line strip formed by the sequence of points.
  {
    // Set up current style.
    m_current_styles[LINE_STYLE].apply(m_current_matrix);

    apply_matrix(m_current_matrix);
    prepare_vertex_buffer((int16_t*)coords, vertex_count);
    HRESULT hr = m_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, vertex_count-1);
    assert(hr==S_OK);
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
    D3DXMATRIX  mat;
    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);

    gameswf::point a, b, c, d;
    m.transform(&a, gameswf::point(coords.m_x_min, coords.m_y_min));
    m.transform(&b, gameswf::point(coords.m_x_max, coords.m_y_min));
    m.transform(&c, gameswf::point(coords.m_x_min, coords.m_y_max));
    d.x = b.x + c.x - a.x;
    d.y = b.y + c.y - a.y;

    // Set texture.
    m_pd3dDevice->SetTexture(0, s_d3d_textures[bi->m_texture_id]);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

    m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
    m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

    // No texgen; just pass through.
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );

    // Draw the quad.

    //pk
    CUSTOMVERTEX2 *pVertices;
    HRESULT result;

    if( FAILED( m_pVB->Lock( 0, 0, (tLock**)&pVertices, D3DLOCK_DISCARD ) ) )
    {
      abort();
    }

    pVertices[0].x = a.x ;
    pVertices[0].y = a.y ;
    pVertices[0].z = Z_DEPTH;
    pVertices[0].tu       = uv_coords.m_x_min ;
    pVertices[0].tv       = uv_coords.m_y_min ;

    pVertices[1].x = b.x ;
    pVertices[1].y = b.y ;
    pVertices[1].z = Z_DEPTH;
    pVertices[1].tu       = uv_coords.m_x_max ;
    pVertices[1].tv       = uv_coords.m_y_min ;

    pVertices[2].x = c.x ;
    pVertices[2].y = c.y ;
    pVertices[2].z = Z_DEPTH;
    pVertices[2].tu       = uv_coords.m_x_min ;
    pVertices[2].tv       = uv_coords.m_y_max ;

    pVertices[3].x = d.x ;
    pVertices[3].y = d.y ;
    pVertices[3].z = Z_DEPTH;
    pVertices[3].tu       = uv_coords.m_x_max ;
    pVertices[3].tv       = uv_coords.m_y_max ;

    m_pVB2->Unlock();

    // Render the vertex buffer contents
    m_pd3dDevice->SetStreamSource( 0, m_pVB2,
#if DIRECT3D_VERSION >= 0x0900
      0,
#endif
      sizeof(CUSTOMVERTEX2) );
#if DIRECT3D_VERSION < 0x0900
    m_pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX2);
#else
    m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX2);
#endif
    result = m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    assert(result==S_OK);
  }

  void begin_submit_mask()
  {

    m_pd3dDevice->SetRenderState(
      D3DRS_ZWRITEENABLE,
      FALSE );

    // Enable stencil testing
    m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

    //m_pd3dDevice->SetRenderState(
    //  D3DRS_ZWRITEENABLE,
    //            FALSE, false );


    // Clear stencil buffer values to zero
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0f, 0);

    // Specify the stencil comparison function
    m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS );

    // Set the comparison reference value
    m_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 1);

    //  Specify a stencil mask
    m_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0x1);

    // A write mask controls what is written
    m_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0x1);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);

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
    m_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);

    // Specify the stencil comparison function
    m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);

    m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);

#if 0
    glColorMask(1,1,1,1); // enable framebuffer writes
    glStencilFunc(GL_EQUAL, 1, 1);  // we draw only where the stencil is 1 (where the mask was drawn)
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // don't change the stencil buffer
#endif // 0
  }

  void disable_mask()
  {
    m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

#if 0
    glDisable(GL_STENCIL_TEST);
#endif // 0
  }

};  // end class render_handler_d3d


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

  // Need to insert a dummy alpha byte in the image data, for
  // D3DXLoadSurfaceFromMemory.
  // @@ this sucks :(
  uint8_t*  expanded_data = new uint8_t[m_original_width * m_original_height * 4];
  uint8_t*  pdata = expanded_data;
  for (int y = 0; y < m_original_height; y++)
  {
    uint8_t*  scanline = image::scanline(im, y);
    for (int x = 0; x < m_original_width; x++)
    {
      *pdata++ = scanline[x * 3 + 2]; // blue
      *pdata++ = scanline[x * 3 + 1]; // green
      *pdata++ = scanline[x * 3 + 0]; // red
      *pdata++ = 255; // alpha
    }
  }

  // Create the texture.
  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

  IDirect3DTexture*  tex;
  HRESULT result = render_handler_d3d::m_pd3dDevice->CreateTexture(
    w, h, 1, 0,      // Usage
    render_handler_d3d::m_FormatRGB,  // Format
    D3DPOOL_MANAGED, &tex
#if DIRECT3D_VERSION >= 0x0900
    , NULL
#endif
    );

  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

  IDirect3DSurface*  surf = NULL;
  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  RECT  source_rect;
  source_rect.left    = 0;
  source_rect.top     = 0;
  source_rect.right   = m_original_width;
  source_rect.bottom  = m_original_height;
  result = D3DXLoadSurfaceFromMemory( surf, NULL, NULL, expanded_data, 
    D3DFMT_A8R8G8B8, m_original_width * 4, NULL, &source_rect, D3DX_DEFAULT, 0 );

  // test
  //D3DXSaveSurfaceToFile( "image.png", D3DXIFF_PNG, surf, NULL, NULL );

  delete [] expanded_data;
  if (result != S_OK)
    gameswf::log_error("error: can't load surface from memory, result = %d\n", result);

  surf->Release();
}

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
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

#if DIRECT3D_VERSION < 0x0900
  convert_to_argb(im);
#endif

  m_original_width = im->m_width;
  m_original_height = im->m_height;

  int w = 1; while (w < im->m_width) { w <<= 1; }
  int h = 1; while (h < im->m_height) { h <<= 1; }

  // Create the texture.
  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

  IDirect3DTexture*  tex;
  HRESULT result = render_handler_d3d::m_pd3dDevice->CreateTexture(
    w, h, 1, 0,      // Usage
    render_handler_d3d::m_FormatRGBA,  // Format
    D3DPOOL_MANAGED, &tex
#if DIRECT3D_VERSION >= 0x0900
    , NULL
#endif    
    );

  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

  IDirect3DSurface*  surf = NULL;
  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  RECT  source_rect;
  source_rect.left    = 0;
  source_rect.top     = 0;
  source_rect.right   = m_original_width;
  source_rect.bottom  = m_original_height;

  // Set the actual data.
  result = D3DXLoadSurfaceFromMemory( surf, NULL, NULL, im->m_data,
#if DIRECT3D_VERSION < 0x0900
    D3DFMT_A8R8G8B8, 
#else
    D3DFMT_A8B8G8R8, 
#endif
    im->m_pitch, NULL, &source_rect, D3DX_DEFAULT, 0);

  if (result != S_OK)
    gameswf::log_error("error: can't load surface from memory, result = %d\n", result);

  surf->Release();
}


bitmap_info_d3d::bitmap_info_d3d(int width, int height, uint8_t* data)
// Initialize this bitmap_info to an alpha image
// containing the specified data (1 byte per texel).
//
// !! Munges *data in order to create mipmaps !!
{
  assert(data);

  // Create the texture.
  m_original_width = width;
  m_original_height = height;

  // You must use power-of-two dimensions!!
  int w = 1; while (w < width) { w <<= 1; }
  int h = 1; while (h < height) { h <<= 1; }

  s_d3d_textures.push_back(NULL);
  m_texture_id = s_d3d_textures.size() - 1;

  IDirect3DTexture*  tex;
  HRESULT result = render_handler_d3d::m_pd3dDevice->CreateTexture(
    w, h, 1, 0,      // Usage
    render_handler_d3d::m_FormatA,  // Format
    D3DPOOL_MANAGED, &tex
#if DIRECT3D_VERSION >= 0x0900
    , NULL
#endif
    );

  if (result != S_OK)
  {
    gameswf::log_error("error: can't create texture\n");
    return;
  }
  s_d3d_textures.back() = tex;

  IDirect3DSurface*  surf = NULL;

  result = tex->GetSurfaceLevel(0, &surf);
  if (result != S_OK)
  {
    gameswf::log_error("error: can't get surface\n");
    return;
  }
  assert(surf);

  RECT	source_rect;
  source_rect.left    = 0;
  source_rect.top     = 0;
  source_rect.right   = width;
  source_rect.bottom  = height;
  result = D3DXLoadSurfaceFromMemory( surf, NULL, NULL, data,
    D3DFMT_A8, width, NULL, &source_rect, D3DX_DEFAULT, 0);

  if (result != S_OK)
    gameswf::log_error("error: can't load surface from memory, result = %d\n", result);

  surf->Release();

  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

  //  // Build mips.
  //  int level = 1;
  //  while (width > 1 || height > 1)
  //  {
  //    render_handler_d3d::make_next_miplevel(&width, &height, data);
  //    glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
  //    level++;
  //  }
}

IDirect3DDevice*  render_handler_d3d::m_pd3dDevice;
D3DFORMAT         render_handler_d3d::m_FormatRGB;
D3DFORMAT         render_handler_d3d::m_FormatRGBA;
D3DFORMAT         render_handler_d3d::m_FormatA;
D3DXMATRIX        render_handler_d3d::m_ModelViewMatrix;
D3DXMATRIX        render_handler_d3d::m_ProjMatrix;

gameswf::render_handler*  gameswf::create_render_handler_d3d(IDirect3DDevice* device)
{
  render_handler_d3d *hndlr = new render_handler_d3d( device );
  return hndlr;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:


