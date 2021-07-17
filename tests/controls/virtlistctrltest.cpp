///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/virtlistctrltest.cpp
// Purpose:     wxListCtrl unit tests for virtual mode
// Author:      Vadim Zeitlin
// Created:     2010-11-13
// Copyright:   (c) 2010 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_LISTCTRL


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/listctrl.h"
#include "testableframe.h"
#include "wx/uiaction.h"

TEST_CASE("Virtual list control test.")
{
    // Define a class overriding OnGetItemText() which must be overridden for
    // any virtual list control.
    class VirtListCtrl : public wxListCtrl
    {
    public:
        VirtListCtrl()
            : wxListCtrl(wxTheApp->GetTopWindow(), wxID_ANY,
                wxPoint(0, 0), wxSize(400, 200),
                wxLC_REPORT | wxLC_VIRTUAL)
        {
        }

    protected:
        wxString OnGetItemText(long item, long column) const override
        {
            return wxString::Format("Row %ld, col %ld", item, column);
        }
    };

    auto m_list = new VirtListCtrl;

    SUBCASE("UpdateSelection")
    {
        m_list->SetItemCount(10);
        CHECK_EQ( 0, m_list->GetSelectedItemCount() );

        m_list->SetItemState(7, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        CHECK_EQ( 1, m_list->GetSelectedItemCount() );

        m_list->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        CHECK_EQ( 2, m_list->GetSelectedItemCount() );

        // The item 7 is now invalid and so shouldn't be counted as selected any
        // more.
        m_list->SetItemCount(5);
        CHECK_EQ( 1, m_list->GetSelectedItemCount() );
    }

    SUBCASE("DeselectedEvent")
    {
    #if wxUSE_UIACTIONSIMULATOR
        m_list->AppendColumn("Col0");
        m_list->SetItemCount(1);
        wxListCtrl* const list = m_list;

        EventCounter selected(list, wxEVT_LIST_ITEM_SELECTED);
        EventCounter deselected(list, wxEVT_LIST_ITEM_DESELECTED);

        wxUIActionSimulator sim;

        wxRect pos;
        list->GetItemRect(0, pos);

        //We move in slightly so we are not on the edge
        wxPoint point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick();
        wxYield();

        // We want a point within the listctrl but below any items
        point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 50);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, selected.GetCount());
        CHECK_EQ(1, deselected.GetCount());
    #endif
    }

    delete m_list;
    m_list = nullptr;
}

#endif // wxUSE_LISTCTRL
