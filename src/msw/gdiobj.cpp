/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/gdiobj.cpp
// Purpose:     wxGDIObject class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/gdiobj.h"
#include "wx/utils.h"

#define M_GDIDATA static_cast<wxGDIRefData*>(m_refData)

/*
void wxGDIObject::IncrementResourceUsage()
{
    if ( !M_GDIDATA )
        return;

//  wxDebugMsg("Object %ld about to be incremented: %d\n", (long)this, m_usageCount);
  M_GDIDATA->m_usageCount ++;
};

void wxGDIObject::DecrementResourceUsage()
{
    if ( !M_GDIDATA )
        return;

  M_GDIDATA->m_usageCount --;
  if (wxTheApp)
    wxTheApp->SetPendingCleanup(true);
//  wxDebugMsg("Object %ld decremented: %d\n", (long)this, M_GDIDATA->m_usageCount);
  if (M_GDIDATA->m_usageCount < 0)
  {
    char buf[80];
    sprintf(buf, "Object %ld usage count is %d\n", (long)this, M_GDIDATA->m_usageCount);
    wxDebugMsg(buf);
  }
//  assert(M_GDIDATA->m_usageCount >= 0);
};

*/
