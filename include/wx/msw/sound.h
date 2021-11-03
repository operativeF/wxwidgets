/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/sound.h
// Purpose:     wxSound class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SOUND_H_
#define _WX_SOUND_H_

#if wxUSE_SOUND

#include <string>

class wxSound : public wxSoundBase
{
public:
    wxSound() = default;
    wxSound(const std::string& fileName, bool isResource = false);
    wxSound(size_t size, const void* data);
    ~wxSound();

    wxSound& operator=(wxSound&&) = delete;

    // Create from resource or file
    [[maybe_unused]] bool Create(const std::string& fileName, bool isResource = false);

    // Create from data
    [[maybe_unused]] bool Create(size_t size, const void* data);

    bool IsOk() const { return m_data != nullptr; }

    static void Stop();

protected:
    bool CheckCreatedOk();
    void Free();

    bool DoPlay(unsigned flags) const override;

private:
    // data of this object
    struct wxSoundData *m_data{nullptr};
};

#endif // wxUSE_SOUND

#endif // _WX_SOUND_H_

