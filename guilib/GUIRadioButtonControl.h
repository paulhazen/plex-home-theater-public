/*!
\file GUIRadioButtonControl.h
\brief 
*/

#pragma once
#include "GUIButtonControl.h"

/*!
 \ingroup controls
 \brief 
 */
class CGUIRadioButtonControl :
      public CGUIButtonControl
{
public:
  CGUIRadioButtonControl(DWORD dwParentID, DWORD dwControlId,
                         int iPosX, int iPosY, DWORD dwWidth, DWORD dwHeight,
                         const CStdString& strTextureFocus, const CStdString& strTextureNoFocus,
                         const CLabelInfo& labelInfo,
                         const CStdString& strRadioFocus, const CStdString& strRadioNoFocus);

  virtual ~CGUIRadioButtonControl(void);
  virtual void Render();
  virtual bool OnAction(const CAction &action) ;
  virtual bool OnMessage(CGUIMessage& message);
  virtual void PreAllocResources();
  virtual void AllocResources();
  virtual void FreeResources();
  virtual void DynamicResourceAlloc(bool bOnOff);
  virtual void SetPosition(int iPosX, int iPosY);
  virtual void SetWidth(int iWidth);
  virtual void SetHeight(int iHeight);
  const CStdString& GetTextureRadioFocusName() const { return m_imgRadioFocus.GetFileName(); };
  const CStdString& GetTextureRadioNoFocusName() const { return m_imgRadioNoFocus.GetFileName(); };
  virtual CStdString GetDescription() const;
  void SetRadioDimensions(int posX, int posY, DWORD width, DWORD height);
  void GetRadioDimensions(int &posX, int &posY, DWORD &width, DWORD &height);
  int GetToggleSelect() const { return m_toggleSelect; };
  void SetToggleSelect(int toggleSelect) { m_toggleSelect = toggleSelect; };

protected:
  virtual void Update();
  CGUIImage m_imgRadioFocus;
  CGUIImage m_imgRadioNoFocus;
  int m_radioPosX;
  int m_radioPosY;
  int m_toggleSelect;
};
