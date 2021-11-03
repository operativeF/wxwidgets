/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/colour.h
// Purpose:     wxColour class implementation for wxQt
// Author:      Kolya Kosenko
// Created:     2010-05-12
// Copyright:   (c) 2010 Kolya Kosenko
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_COLOUR_H_
#define _WX_QT_COLOUR_H_

class QColor;

class wxColour : public wxColourBase
{
public:
    DEFINE_STD_WXCOLOUR_CONSTRUCTORS
    wxColour(const QColor& color);

    bool IsOk() const override { return m_valid; }

    ChannelType Red()   const override { return m_red;   }
    ChannelType Green() const override { return m_green; }
    ChannelType Blue()  const override { return m_blue;  }
    ChannelType Alpha() const override { return m_alpha; }

    bool operator==(const wxColour& color) const;
    bool operator!=(const wxColour& color) const;

    int GetPixel() const;

    QColor GetQColor() const;

protected:
    void Init();
    void InitRGBA(ChannelType r, ChannelType g, ChannelType b, ChannelType a) override;

private:
    ChannelType m_red, m_green, m_blue, m_alpha;
    bool m_valid;

    wxDECLARE_DYNAMIC_CLASS(wxColour);
};

#endif // _WX_QT_COLOUR_H_
