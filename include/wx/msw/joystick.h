/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/joystick.h
// Purpose:     wxJoystick class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_JOYSTICK_H_
#define _WX_JOYSTICK_H_

#include "wx/event.h"

#include <chrono>

import <string>;

using namespace std::chrono_literals;

class wxJoystickThread;

class wxJoystick: public wxObject
{
public:
  explicit wxJoystick(int joystick = wxJOYSTICK1);
  ~wxJoystick();

  wxPoint GetPosition() const;
  int GetPosition(unsigned axis) const;
  bool GetButtonState(unsigned button) const;
  int GetZPosition() const;
  int GetButtonState() const;
  int GetPOVPosition() const;
  int GetPOVCTSPosition() const;
  int GetRudderPosition() const;
  int GetUPosition() const;
  int GetVPosition() const;
  int GetMovementThreshold() const;
  void SetMovementThreshold(int threshold) ;

  static int GetNumberJoysticks();

  bool IsOk() const; // Checks that the joystick is functioning
  int GetManufacturerId() const ;
  int GetProductId() const ;
  std::string GetProductName() const ;
  int GetXMin() const;
  int GetYMin() const;
  int GetZMin() const;
  int GetXMax() const;
  int GetYMax() const;
  int GetZMax() const;
  int GetNumberButtons() const;
  int GetNumberAxes() const;
  int GetMaxButtons() const;
  int GetMaxAxes() const;
  int GetPollingMin() const;
  int GetPollingMax() const;
  int GetRudderMin() const;
  int GetRudderMax() const;
  int GetUMin() const;
  int GetUMax() const;
  int GetVMin() const;
  int GetVMax() const;

  bool HasRudder() const;
  bool HasZ() const;
  bool HasU() const;
  bool HasV() const;
  bool HasPOV() const;
  bool HasPOV4Dir() const;
  bool HasPOVCTS() const;

  // pollingFreq = 0 means that movement events are sent when above the threshold.
  // If pollingFreq > 0, events are received every this many milliseconds.
  bool SetCapture(wxWindow* win, std::chrono::milliseconds pollingFreq = 0ms);
  bool ReleaseCapture();

protected:
  int                   m_joystick;
  wxJoystickThread*     m_thread{nullptr};
};

#endif // _WX_JOYSTICK_H_
