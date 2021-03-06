/*
 *      Copyright (C) 2011-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "system.h"

#include <EGL/egl.h>
#include "EGLNativeTypeRaspberryPI.h"
#include "utils/log.h"
#include "guilib/gui3d.h"
#include "linux/DllBCM.h"

#ifndef __VIDEOCORE4__
#define __VIDEOCORE4__
#endif

#define __VCCOREVER__ 0x04000000

#define IS_WIDESCREEN(m) ( m == 3 || m == 7 || m == 9 || \
    m == 11 || m == 13 || m == 15 || m == 18 || m == 22 || \
    m == 24 || m == 26 || m == 28 || m == 30 || m == 36 || \
    m == 38 || m == 43 || m == 45 || m == 49 || m == 51 || \
    m == 53 || m == 55 || m == 57 || m == 59)

#define MAKEFLAGS(group, mode, interlace) \
  ( ( (mode)<<24 ) | ( (group)<<16 ) | \
   ( (interlace) != 0 ? D3DPRESENTFLAG_INTERLACED : D3DPRESENTFLAG_PROGRESSIVE) | \
   ( ((group) == HDMI_RES_GROUP_CEA && IS_WIDESCREEN(mode) ) ? D3DPRESENTFLAG_WIDESCREEN : 0) )

#define GETFLAGS_GROUP(f)       ( (HDMI_RES_GROUP_T)( ((f) >> 16) & 0xff ))
#define GETFLAGS_MODE(f)        ( ( (f) >>24 ) & 0xff )

//#ifndef DEBUG_PRINT
//#define DEBUG_PRINT 1
//#endif

#if defined(DEBUG_PRINT)
# define DLOG(fmt, args...) printf(fmt, ##args)
#else
# define DLOG(fmt, args...)
#endif


CEGLNativeTypeRaspberryPI::CEGLNativeTypeRaspberryPI()
{
#if defined(TARGET_RASPBERRY_PI)
  m_DllBcmHost    = NULL;
  m_nativeWindow  = NULL;
#endif
}

CEGLNativeTypeRaspberryPI::~CEGLNativeTypeRaspberryPI()
{
#if defined(TARGET_RASPBERRY_PI)
  delete m_DllBcmHost;
  if(m_nativeWindow)
    free(m_nativeWindow);
#endif
} 

bool CEGLNativeTypeRaspberryPI::CheckCompatibility()
{
#if defined(TARGET_RASPBERRY_PI)
  DLOG("CEGLNativeTypeRaspberryPI::CheckCompatibility\n");
  return true;
#else
  return false;
#endif
}

void CEGLNativeTypeRaspberryPI::Initialize()
{
#if defined(TARGET_RASPBERRY_PI)
  m_DllBcmHost              = NULL;
  m_dispman_element         = DISPMANX_NO_HANDLE;
  m_dispman_element2        = DISPMANX_NO_HANDLE;
  m_dispman_display         = DISPMANX_NO_HANDLE;

  /* PLEX */
#if defined(__PLEX__)
  m_width                   = 1920;
  m_height                  = 1080;
#else
  m_width                   = 1280;
  m_height                  = 720;
#endif
  /* END PLEX */
  m_initDesktopRes          = true;

  m_DllBcmHost = new DllBcmHost;
  m_DllBcmHost->Load();
#endif
}

void CEGLNativeTypeRaspberryPI::Destroy()
{
#if defined(TARGET_RASPBERRY_PI)
  if(m_DllBcmHost && m_DllBcmHost->IsLoaded())
    m_DllBcmHost->Unload();
  delete m_DllBcmHost;
  m_DllBcmHost = NULL;
#endif
}

bool CEGLNativeTypeRaspberryPI::CreateNativeDisplay()
{
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeRaspberryPI::CreateNativeWindow()
{
#if defined(TARGET_RASPBERRY_PI)
  if(!m_nativeWindow)
    m_nativeWindow = (EGLNativeWindowType) calloc(1,sizeof( EGL_DISPMANX_WINDOW_T));
  DLOG("CEGLNativeTypeRaspberryPI::CEGLNativeTypeRaspberryPI\n");
  return true;
#else
  return false;
#endif
}

bool CEGLNativeTypeRaspberryPI::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;
  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeRaspberryPI::GetNativeWindow(XBNativeDisplayType **nativeWindow) const
{
  DLOG("CEGLNativeTypeRaspberryPI::GetNativeWindow\n");
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;
  return true;
}  

bool CEGLNativeTypeRaspberryPI::DestroyNativeDisplay()
{
  DLOG("CEGLNativeTypeRaspberryPI::DestroyNativeDisplay\n");
  return true;
}

bool CEGLNativeTypeRaspberryPI::DestroyNativeWindow()
{
#if defined(TARGET_RASPBERRY_PI)
  DestroyDispmaxWindow();
  free(m_nativeWindow);
  m_nativeWindow = NULL;
  DLOG("CEGLNativeTypeRaspberryPI::DestroyNativeWindow\n");
  return true;
#else
  return false;
#endif  
}

bool CEGLNativeTypeRaspberryPI::GetNativeResolution(RESOLUTION_INFO *res) const
{
#if defined(TARGET_RASPBERRY_PI)
  *res = m_desktopRes;

  DLOG("CEGLNativeTypeRaspberryPI::GetNativeResolution %s\n", res->strMode.c_str());
  return true;
#else
  return false;
#endif
}

#if defined(TARGET_RASPBERRY_PI)
int CEGLNativeTypeRaspberryPI::FindMatchingResolution(const RESOLUTION_INFO &res, const std::vector<RESOLUTION_INFO> &resolutions)
{
  for (int i = 0; i < (int)resolutions.size(); i++)
  {
    if(resolutions[i].iScreenWidth == res.iScreenWidth && resolutions[i].iScreenHeight == res.iScreenHeight && resolutions[i].fRefreshRate == res.fRefreshRate)
    {
       return i;
    }
  }
  return -1;
}
#endif

#if defined(TARGET_RASPBERRY_PI)
int CEGLNativeTypeRaspberryPI::AddUniqueResolution(const RESOLUTION_INFO &res, std::vector<RESOLUTION_INFO> &resolutions)
{
  int i = FindMatchingResolution(res, resolutions);
  if (i>=0)
  {  // don't replace a progressive resolution with an interlaced one of same resolution
     if (!(res.dwFlags & D3DPRESENTFLAG_INTERLACED))
       resolutions[i] = res;
  }
  else
  {
     resolutions.push_back(res);
  }
  return i;
}
#endif

bool CEGLNativeTypeRaspberryPI::SetNativeResolution(const RESOLUTION_INFO &res)
{
#if defined(TARGET_RASPBERRY_PI)
  if(!m_DllBcmHost || !m_nativeWindow)
    return false;

  DestroyDispmaxWindow();

  if(!m_fixedMode && GETFLAGS_GROUP(res.dwFlags) && GETFLAGS_MODE(res.dwFlags))
  {
    sem_init(&m_tv_synced, 0, 0);
    m_DllBcmHost->vc_tv_register_callback(CallbackTvServiceCallback, this);

    if (res.dwFlags & (D3DPRESENTFLAG_MODE3DSBS|D3DPRESENTFLAG_MODE3DTB))
    {
      /* inform TV of any 3D settings. Note this property just applies to next hdmi mode change, so no need to call for 2D modes */
      HDMI_PROPERTY_PARAM_T property;
      property.property = HDMI_PROPERTY_3D_STRUCTURE;
      if (res.dwFlags & D3DPRESENTFLAG_MODE3DSBS)
        property.param1 = HDMI_3D_FORMAT_SBS_HALF;
      else if (res.dwFlags & D3DPRESENTFLAG_MODE3DTB)
        property.param1 = HDMI_3D_FORMAT_TB_HALF;
      else
        property.param1 = HDMI_3D_FORMAT_NONE;
      property.param2 = 0;
      vc_tv_hdmi_set_property(&property);
    }
    int success = m_DllBcmHost->vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_HDMI, GETFLAGS_GROUP(res.dwFlags), GETFLAGS_MODE(res.dwFlags));

    if (success == 0)
    {
      CLog::Log(LOGDEBUG, "EGL set HDMI mode (%d,%d)=%d%s%s\n",
                          GETFLAGS_GROUP(res.dwFlags), GETFLAGS_MODE(res.dwFlags), success,
                          (res.dwFlags & D3DPRESENTFLAG_MODE3DSBS) ? " SBS":"",
                          (res.dwFlags & D3DPRESENTFLAG_MODE3DTB) ? " TB":"");

      sem_wait(&m_tv_synced);
    }
    else
    {
      CLog::Log(LOGERROR, "EGL failed to set HDMI mode (%d,%d)=%d%s%s\n",
                          GETFLAGS_GROUP(res.dwFlags), GETFLAGS_MODE(res.dwFlags), success,
                          (res.dwFlags & D3DPRESENTFLAG_MODE3DSBS) ? " SBS":"",
                          (res.dwFlags & D3DPRESENTFLAG_MODE3DTB) ? " TB":"");
    }
    m_DllBcmHost->vc_tv_unregister_callback(CallbackTvServiceCallback);
    sem_destroy(&m_tv_synced);

    m_desktopRes = res;
  }

  m_dispman_display = m_DllBcmHost->vc_dispmanx_display_open(0);

  m_width   = res.iWidth;
  m_height  = res.iHeight;

  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;

  dst_rect.x      = 0;
  dst_rect.y      = 0;
  dst_rect.width  = res.iScreenWidth;
  dst_rect.height = res.iScreenHeight;

  src_rect.x      = 0;
  src_rect.y      = 0;
  src_rect.width  = m_width << 16;
  src_rect.height = m_height << 16;

  VC_DISPMANX_ALPHA_T alpha;
  memset(&alpha, 0x0, sizeof(VC_DISPMANX_ALPHA_T));
  alpha.flags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE;

  DISPMANX_CLAMP_T clamp;
  memset(&clamp, 0x0, sizeof(DISPMANX_CLAMP_T));

  DISPMANX_TRANSFORM_T transform = DISPMANX_NO_ROTATE;
  DISPMANX_UPDATE_HANDLE_T dispman_update = m_DllBcmHost->vc_dispmanx_update_start(0);

  CLog::Log(LOGDEBUG, "EGL set resolution %dx%d -> %dx%d @ %.2f fps (%d,%d) flags:%x aspect:%.2f\n",
      m_width, m_height, dst_rect.width, dst_rect.height, res.fRefreshRate, GETFLAGS_GROUP(res.dwFlags), GETFLAGS_MODE(res.dwFlags), (int)res.dwFlags, res.fPixelRatio);

  // The trick for SBS is that we stick two dispman elements together 
  // and the PI firmware knows that we are in SBS/TAB mode and it renders the gui in SBS/TAB
  if(res.dwFlags & (D3DPRESENTFLAG_MODE3DSBS|D3DPRESENTFLAG_MODE3DTB))
  {
    if (res.dwFlags & D3DPRESENTFLAG_MODE3DTB)
    {
      // bottom half
      dst_rect.y = res.iScreenHeight;
    }
    else
    {
      // right side
      dst_rect.x = res.iScreenWidth;
    }

    m_dispman_element2 = m_DllBcmHost->vc_dispmanx_element_add(dispman_update,
      m_dispman_display,
      1,                              // layer
      &dst_rect,
      (DISPMANX_RESOURCE_HANDLE_T)0,  // src
      &src_rect,
      DISPMANX_PROTECTION_NONE,
      &alpha,                         // alpha
      &clamp,                         // clamp
      transform);                     // transform
      assert(m_dispman_element2 != DISPMANX_NO_HANDLE);
      assert(m_dispman_element2 != (unsigned)DISPMANX_INVALID);

    if (res.dwFlags & D3DPRESENTFLAG_MODE3DTB)
    {
      // top half - fall through
      dst_rect.y = 0;
    }
    else
    {
      // left side - fall through
      dst_rect.x = 0;
    }
  }

  m_dispman_element = m_DllBcmHost->vc_dispmanx_element_add(dispman_update,
    m_dispman_display,
    1,                              // layer
    &dst_rect,
    (DISPMANX_RESOURCE_HANDLE_T)0,  // src
    &src_rect,
    DISPMANX_PROTECTION_NONE,
    &alpha,                         //alphe
    &clamp,                         //clamp
    transform);                     // transform

  assert(m_dispman_element != DISPMANX_NO_HANDLE);
  assert(m_dispman_element != (unsigned)DISPMANX_INVALID);

  memset(m_nativeWindow, 0, sizeof(EGL_DISPMANX_WINDOW_T));

  EGL_DISPMANX_WINDOW_T *nativeWindow = (EGL_DISPMANX_WINDOW_T *)m_nativeWindow;

  nativeWindow->element = m_dispman_element;
  nativeWindow->width   = m_width;
  nativeWindow->height  = m_height;

  m_DllBcmHost->vc_dispmanx_display_set_background(dispman_update, m_dispman_display, 0x00, 0x00, 0x00);
  m_DllBcmHost->vc_dispmanx_update_submit_sync(dispman_update);

  DLOG("CEGLNativeTypeRaspberryPI::SetNativeResolution\n");

  return true;
#else
  return false;
#endif
}

#if defined(TARGET_RASPBERRY_PI)
static float get_display_aspect_ratio(HDMI_ASPECT_T aspect)
{
  float display_aspect;
  switch (aspect) {
    case HDMI_ASPECT_4_3:   display_aspect = 4.0/3.0;   break;
    case HDMI_ASPECT_14_9:  display_aspect = 14.0/9.0;  break;
    case HDMI_ASPECT_16_9:  display_aspect = 16.0/9.0;  break;
    case HDMI_ASPECT_5_4:   display_aspect = 5.0/4.0;   break;
    case HDMI_ASPECT_16_10: display_aspect = 16.0/10.0; break;
    case HDMI_ASPECT_15_9:  display_aspect = 15.0/9.0;  break;
    case HDMI_ASPECT_64_27: display_aspect = 64.0/27.0; break;
    default:                display_aspect = 16.0/9.0;  break;
  }
  return display_aspect;
}

static float get_display_aspect_ratio(SDTV_ASPECT_T aspect)
{
  float display_aspect;
  switch (aspect) {
    case SDTV_ASPECT_4_3:  display_aspect = 4.0/3.0;  break;
    case SDTV_ASPECT_14_9: display_aspect = 14.0/9.0; break;
    case SDTV_ASPECT_16_9: display_aspect = 16.0/9.0; break;
    default:               display_aspect = 4.0/3.0;  break;
  }
  return display_aspect;
}
#endif

bool CEGLNativeTypeRaspberryPI::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
#if defined(TARGET_RASPBERRY_PI)
  resolutions.clear();

  if(!m_DllBcmHost)
    return false;

  m_fixedMode               = false;

  /* read initial desktop resolution before probe resolutions.
   * probing will replace the desktop resolution when it finds the same one.
   * we raplace it because probing will generate more detailed 
   * resolution flags we don't get with vc_tv_get_state.
   */

  if(m_initDesktopRes)
  {
    TV_DISPLAY_STATE_T tv_state;

    // get current display settings state
    memset(&tv_state, 0, sizeof(TV_DISPLAY_STATE_T));
    m_DllBcmHost->vc_tv_get_display_state(&tv_state);

    if ((tv_state.state & ( VC_HDMI_HDMI | VC_HDMI_DVI )) != 0) // hdtv
    {
      m_desktopRes.iScreen      = 0;
      m_desktopRes.bFullScreen  = true;
      m_desktopRes.iWidth       = tv_state.display.hdmi.width;
      m_desktopRes.iHeight      = tv_state.display.hdmi.height;
      m_desktopRes.iScreenWidth = tv_state.display.hdmi.width;
      m_desktopRes.iScreenHeight= tv_state.display.hdmi.height;
      m_desktopRes.dwFlags      = MAKEFLAGS(tv_state.display.hdmi.group, tv_state.display.hdmi.mode, tv_state.display.hdmi.scan_mode);
      // Also add 3D flags
      if (tv_state.display.hdmi.format_3d == HDMI_3D_FORMAT_SBS_HALF)
      {
        m_desktopRes.dwFlags |= D3DPRESENTFLAG_MODE3DSBS;
        m_desktopRes.iWidth       >>= 1;
        m_desktopRes.iScreenWidth >>= 1;
      }
      else if (tv_state.display.hdmi.format_3d == HDMI_3D_FORMAT_TB_HALF)
      {
        m_desktopRes.dwFlags |= D3DPRESENTFLAG_MODE3DTB;
        m_desktopRes.iHeight       >>= 1;
        m_desktopRes.iScreenHeight >>= 1;
      }
      m_desktopRes.fRefreshRate = (float)tv_state.display.hdmi.frame_rate;
      m_desktopRes.fPixelRatio  = get_display_aspect_ratio((HDMI_ASPECT_T)tv_state.display.hdmi.display_options.aspect) / ((float)m_desktopRes.iScreenWidth / (float)m_desktopRes.iScreenHeight);
    }
    else // sdtv
    {
      m_desktopRes.iScreen      = 0;
      m_desktopRes.bFullScreen  = true;
      m_desktopRes.iWidth       = tv_state.display.sdtv.width;
      m_desktopRes.iHeight      = tv_state.display.sdtv.height;
      m_desktopRes.iScreenWidth = tv_state.display.sdtv.width;
      m_desktopRes.iScreenHeight= tv_state.display.sdtv.height;
      m_desktopRes.dwFlags      = D3DPRESENTFLAG_INTERLACED;
      m_desktopRes.fRefreshRate = (float)tv_state.display.sdtv.frame_rate;
      m_desktopRes.fPixelRatio  = get_display_aspect_ratio((SDTV_ASPECT_T)tv_state.display.sdtv.display_options.aspect) / ((float)m_desktopRes.iScreenWidth / (float)m_desktopRes.iScreenHeight);
    }

    m_desktopRes.strMode.Format("%dx%d", m_desktopRes.iScreenWidth, m_desktopRes.iScreenHeight);

    if((int)m_desktopRes.fRefreshRate > 1)
    {
      m_desktopRes.strMode.Format("%dx%d @ %.2f%s - Full Screen", m_desktopRes.iScreenWidth, m_desktopRes.iScreenHeight, m_desktopRes.fRefreshRate,
        m_desktopRes.dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
    }
    m_initDesktopRes = false;

    int gui_width  = m_desktopRes.iWidth;
    int gui_height = m_desktopRes.iHeight;

    ClampToGUIDisplayLimits(gui_width, gui_height);

    m_desktopRes.iWidth = gui_width;
    m_desktopRes.iHeight = gui_height;

    m_desktopRes.iSubtitles   = (int)(0.965 * m_desktopRes.iHeight);

    CLog::Log(LOGDEBUG, "EGL initial desktop resolution %s (%.2f)\n", m_desktopRes.strMode.c_str(), m_desktopRes.fPixelRatio);
  }

  GetSupportedModes(HDMI_RES_GROUP_CEA, resolutions);
  GetSupportedModes(HDMI_RES_GROUP_DMT, resolutions);

  if(resolutions.size() == 0)
  {
    RESOLUTION_INFO res;
    CLog::Log(LOGDEBUG, "EGL probe resolution %s:%x\n", m_desktopRes.strMode.c_str(), m_desktopRes.dwFlags);

    AddUniqueResolution(m_desktopRes, resolutions);
  }

  if(resolutions.size() < 2)
    m_fixedMode = true;

  DLOG("CEGLNativeTypeRaspberryPI::ProbeResolutions\n");
  return true;
#else
  return false;
#endif
}

bool CEGLNativeTypeRaspberryPI::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  DLOG("CEGLNativeTypeRaspberryPI::GetPreferredResolution\n");
  return false;
}

bool CEGLNativeTypeRaspberryPI::ShowWindow(bool show)
{
  DLOG("CEGLNativeTypeRaspberryPI::ShowWindow\n");
  return false;
}

#if defined(TARGET_RASPBERRY_PI)
void CEGLNativeTypeRaspberryPI::DestroyDispmaxWindow()
{
  if(!m_DllBcmHost)
    return;

  DISPMANX_UPDATE_HANDLE_T dispman_update = m_DllBcmHost->vc_dispmanx_update_start(0);

  if (m_dispman_element != DISPMANX_NO_HANDLE)
  {
    m_DllBcmHost->vc_dispmanx_element_remove(dispman_update, m_dispman_element);
    m_dispman_element = DISPMANX_NO_HANDLE;
  }
  if (m_dispman_element2 != DISPMANX_NO_HANDLE)
  {
    m_DllBcmHost->vc_dispmanx_element_remove(dispman_update, m_dispman_element2);
    m_dispman_element2 = DISPMANX_NO_HANDLE;
  }
  m_DllBcmHost->vc_dispmanx_update_submit_sync(dispman_update);

  if (m_dispman_display != DISPMANX_NO_HANDLE)
  {
    m_DllBcmHost->vc_dispmanx_display_close(m_dispman_display);
    m_dispman_display = DISPMANX_NO_HANDLE;
  }
  DLOG("CEGLNativeTypeRaspberryPI::DestroyDispmaxWindow\n");
}

void CEGLNativeTypeRaspberryPI::GetSupportedModes(HDMI_RES_GROUP_T group, std::vector<RESOLUTION_INFO> &resolutions)
{
  if(!m_DllBcmHost)
    return;

  //Supported HDMI CEA/DMT resolutions, preferred resolution will be returned
  int32_t num_modes = 0;
  HDMI_RES_GROUP_T prefer_group;
  uint32_t prefer_mode;
  int i;
  TV_SUPPORTED_MODE_NEW_T *supported_modes = NULL;
  // query the number of modes first
  int max_supported_modes = m_DllBcmHost->vc_tv_hdmi_get_supported_modes_new(group, NULL, 0, &prefer_group, &prefer_mode);

  if (max_supported_modes > 0)
    supported_modes = new TV_SUPPORTED_MODE_NEW_T[max_supported_modes];

  if (supported_modes)
  {
    num_modes = m_DllBcmHost->vc_tv_hdmi_get_supported_modes_new(group,
        supported_modes, max_supported_modes, &prefer_group, &prefer_mode);

    CLog::Log(LOGDEBUG, "EGL get supported modes (%d) = %d, prefer_group=%x, prefer_mode=%x\n",
        group, num_modes, prefer_group, prefer_mode);
  }

  if (num_modes > 0 && prefer_group != HDMI_RES_GROUP_INVALID)
  {
    TV_SUPPORTED_MODE_NEW_T *tv = supported_modes;
    for (i=0; i < num_modes; i++, tv++)
    {
      RESOLUTION_INFO res;

      res.iScreen       = 0;
      res.bFullScreen   = true;
      res.dwFlags       = MAKEFLAGS(group, tv->code, tv->scan_mode);
      res.fRefreshRate  = (float)tv->frame_rate;
      res.iWidth        = tv->width;
      res.iHeight       = tv->height;
      res.iScreenWidth  = tv->width;
      res.iScreenHeight = tv->height;
      res.fPixelRatio   = get_display_aspect_ratio((HDMI_ASPECT_T)tv->aspect_ratio) / ((float)res.iScreenWidth / (float)res.iScreenHeight);

      int gui_width  = res.iWidth;
      int gui_height = res.iHeight;

      ClampToGUIDisplayLimits(gui_width, gui_height);

      res.iWidth = gui_width;
      res.iHeight = gui_height;

      res.strMode.Format("%dx%d @ %.2f%s - Full Screen", res.iScreenWidth, res.iScreenHeight, res.fRefreshRate,
        res.dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");

      CLog::Log(LOGDEBUG, "EGL mode %d: %s (%.2f) %s%s:%x\n", i, res.strMode.c_str(), res.fPixelRatio,
          tv->native ? "N" : "", tv->scan_mode ? "I" : "", tv->code);

      res.iSubtitles    = (int)(0.965 * res.iHeight);

      AddUniqueResolution(res, resolutions);

      // Also add 3D versions of modes
      if (tv->struct_3d_mask & HDMI_3D_STRUCT_SIDE_BY_SIDE_HALF_HORIZONTAL)
      {
        RESOLUTION_INFO res2 = res;
        res2.dwFlags |= D3DPRESENTFLAG_MODE3DSBS;
        res2.iWidth       >>= 1;
        res2.iScreenWidth >>= 1;
        res2.fPixelRatio    = get_display_aspect_ratio((HDMI_ASPECT_T)tv->aspect_ratio) / ((float)res2.iScreenWidth / (float)res2.iScreenHeight);
        res2.strMode.Format("%dx%d", res2.iScreenWidth, res2.iScreenHeight);
        res2.strMode.Format("%dx%d @ %.2f%s - Full Screen", res2.iScreenWidth, res2.iScreenHeight, res2.fRefreshRate,
            res2.dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
        CLog::Log(LOGDEBUG, "EGL mode %d: %s (%.2f) SBS\n", i, res2.strMode.c_str(), res2.fPixelRatio);

        res2.iSubtitles    = (int)(0.965 * res2.iHeight);

        AddUniqueResolution(res2, resolutions);
      }
      if (tv->struct_3d_mask & HDMI_3D_STRUCT_TOP_AND_BOTTOM)
      {
        RESOLUTION_INFO res2 = res;
        res2.dwFlags |= D3DPRESENTFLAG_MODE3DTB;
        res2.iHeight       >>= 1;
        res2.iScreenHeight >>= 1;
        res2.fPixelRatio    = get_display_aspect_ratio((HDMI_ASPECT_T)tv->aspect_ratio) / ((float)res2.iScreenWidth / (float)res2.iScreenHeight);
        res2.strMode.Format("%dx%d", res2.iScreenWidth, res2.iScreenHeight);
        res2.strMode.Format("%dx%d @ %.2f%s - Full Screen", res2.iScreenWidth, res2.iScreenHeight, res2.fRefreshRate,
            res2.dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
        CLog::Log(LOGDEBUG, "EGL mode %d: %s (%.2f) TAB\n", i, res2.strMode.c_str(), res2.fPixelRatio);

        res2.iSubtitles    = (int)(0.965 * res2.iHeight);

        AddUniqueResolution(res2, resolutions);
      }
    }
  }
  if (supported_modes)
    delete [] supported_modes;
}

void CEGLNativeTypeRaspberryPI::TvServiceCallback(uint32_t reason, uint32_t param1, uint32_t param2)
{
  CLog::Log(LOGDEBUG, "EGL tv_service_callback (%d,%d,%d)\n", reason, param1, param2);
  switch(reason)
  {
  case VC_HDMI_UNPLUGGED:
    break;
  case VC_HDMI_STANDBY:
    break;
  case VC_SDTV_NTSC:
  case VC_SDTV_PAL:
  case VC_HDMI_HDMI:
  case VC_HDMI_DVI:
    //Signal we are ready now
    sem_post(&m_tv_synced);
    break;
  default:
     break;
  }
}

void CEGLNativeTypeRaspberryPI::CallbackTvServiceCallback(void *userdata, uint32_t reason, uint32_t param1, uint32_t param2)
{
   CEGLNativeTypeRaspberryPI *callback = static_cast<CEGLNativeTypeRaspberryPI*>(userdata);
   callback->TvServiceCallback(reason, param1, param2);
}

bool CEGLNativeTypeRaspberryPI::ClampToGUIDisplayLimits(int &width, int &height)
{
   /* PLEX */
#if defined(__PLEX__)
  const int max_width = 1920, max_height = 1080;
#else
  const int max_width = 1280, max_height = 720;  
#endif
   /* END PLEX */
  float ar = (float)width/(float)height;
  // bigger than maximum, so need to clamp
  if (width > max_width || height > max_height) {
    // wider than max, so clamp width first
    if (ar > (float)max_width/(float)max_height)
    {
      width = max_width;
      height = (float)max_width / ar + 0.5f;
    // taller than max, so clamp height first
    } else {
      height = max_height;
      width = (float)max_height * ar + 0.5f;
    }
    return true;
  }

  return false;
}

#endif

