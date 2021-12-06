/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/accel.h
// Purpose:     wxAcceleratorTable class
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_ACCEL_H_
#define _WX_GENERIC_ACCEL_H_

class wxKeyEvent;

// ----------------------------------------------------------------------------
// wxAcceleratorTable
// ----------------------------------------------------------------------------

class wxAcceleratorTable : public wxObject
{
public:
    wxAcceleratorTable() = default;
    wxAcceleratorTable(int n, const wxAcceleratorEntry entries[]);
    ~wxAcceleratorTable();

    bool IsOk() const;

    void Add(const wxAcceleratorEntry& entry);
    void Remove(const wxAcceleratorEntry& entry);

    // implementation
    // --------------

    wxMenuItem *GetMenuItem(const wxKeyEvent& event) const;
    int GetCommand(const wxKeyEvent& event) const;

    const wxAcceleratorEntry *GetEntry(const wxKeyEvent& event) const;

protected:
    // ref counting code
    wxObjectRefData *CreateRefData() const override;
    wxObjectRefData *CloneRefData(const wxObjectRefData *data) const override;
};

#endif // _WX_GENERIC_ACCEL_H_

