/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/layout.cpp
// Purpose:     Constraint layout system classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CONSTRAINTS

#include "wx/layout.h"
#include "wx/window.h"
#include "wx/utils.h"
#include "wx/intl.h"

import WX.Utils.Cast;

inline void wxGetAsIs(wxWindowBase* win, int* w, int* h)
{
#if 1
    // FIXME: Works for me (TM).
    // The old way.  Works for me.
    wxSize sz = win->GetSize();
    if (w)
        *w = sz.x;
    if (h)
        *h = sz.y;
#endif

#if 0
    // Vadim's change.  Breaks wxPython's LayoutAnchors
    win->GetBestSize(w, h);
#endif

#if 0
    // Proposed compromise.  Doesn't work.
    int sw, sh, bw, bh;
    win->GetSize(&sw, &sh);
    win->GetBestSize(&bw, &bh);
    if (w)
        *w = std::max(sw, bw);
    if (h)
        *h = std::max(sh, bh);
#endif
}

void wxIndividualLayoutConstraint::Set(wxRelationship rel, wxWindowBase *otherW, wxEdge otherE, int val, int marg)
{
    if (rel == wxSameAs)
    {
        // If Set is called by the user with wxSameAs then call SameAs to do
        // it since it will actually use wxPercent instead.
        SameAs(otherW, otherE, marg);
        return;
    }

    relationship = rel;
    otherWin = otherW;
    otherEdge = otherE;

    if ( rel == wxPercentOf )
    {
        percent = val;
    }
    else
    {
        value = val;
    }

    margin = marg;
}

void wxIndividualLayoutConstraint::LeftOf(wxWindowBase *sibling, int marg)
{
    Set(wxLeftOf, sibling, wxEdge::Left, 0, marg);
}

void wxIndividualLayoutConstraint::RightOf(wxWindowBase *sibling, int marg)
{
    Set(wxRightOf, sibling, wxEdge::Right, 0, marg);
}

void wxIndividualLayoutConstraint::Above(wxWindowBase *sibling, int marg)
{
    Set(wxAbove, sibling, wxEdge::Top, 0, marg);
}

void wxIndividualLayoutConstraint::Below(wxWindowBase *sibling, int marg)
{
    Set(wxBelow, sibling, wxEdge::Bottom, 0, marg);
}

//
// 'Same edge' alignment
//
void wxIndividualLayoutConstraint::SameAs(wxWindowBase *otherW, wxEdge edge, int marg)
{
    Set(wxPercentOf, otherW, edge, 100, marg);
}

// The edge is a percentage of the other window's edge
void wxIndividualLayoutConstraint::PercentOf(wxWindowBase *otherW, wxEdge wh, int per)
{
    Set(wxPercentOf, otherW, wh, per);
}

//
// Edge has absolute value
//
void wxIndividualLayoutConstraint::Absolute(int val)
{
    value = val;
    relationship = wxAbsolute;
}

// Reset constraint if it mentions otherWin
bool wxIndividualLayoutConstraint::ResetIfWin(wxWindowBase *otherW)
{
    if (otherW == otherWin)
    {
        myEdge = wxEdge::Top;
        relationship = wxAsIs;
        margin = 0;
        value = 0;
        percent = 0;
        otherEdge = wxEdge::Top;
        otherWin = nullptr;
        return true;
    }

    return false;
}

// Try to satisfy constraint
bool wxIndividualLayoutConstraint::SatisfyConstraint(wxLayoutConstraints *constraints, wxWindowBase *win)
{
    if (relationship == wxAbsolute)
    {
        done = true;
        return true;
    }

    switch (myEdge)
    {
        case wxEdge::Left:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                    // We can know this edge if: otherWin is win's
                    // parent, or otherWin has a satisfied constraint,
                    // or otherWin has no constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the left-hand edge position if we know
                    // the right-hand edge and we know the width; OR if
                    // we know the centre and the width.
                    if (constraints->right.GetDone() && constraints->width.GetDone())
                    {
                        value = (constraints->right.GetValue() - constraints->width.GetValue() + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->centreX.GetValue() - (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    value = win->GetPosition().x;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::Right:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                    // We can know this edge if: otherWin is win's
                    // parent, or otherWin has a satisfied constraint,
                    // or otherWin has no constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the right-hand edge position if we know the
                    // left-hand edge and we know the width, OR if we know the
                    // centre edge and the width.
                    if (constraints->left.GetDone() && constraints->width.GetDone())
                    {
                        value = (constraints->left.GetValue() + constraints->width.GetValue() - margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->centreX.GetValue() + (constraints->width.GetValue()/2) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int w, h;
                    wxGetAsIs(win, &w, &h);
                    int x_pos = win->GetPosition().x;
                    value = x_pos + w;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::Top:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                    // We can know this edge if: otherWin is win's
                    // parent, or otherWin has a satisfied constraint,
                    // or otherWin has no constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the top edge position if we know the bottom edge
                    // and we know the height; OR if we know the centre edge and
                    // the height.
                    if (constraints->bottom.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->bottom.GetValue() - constraints->height.GetValue() + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreY.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->centreY.GetValue() - (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    value = win->GetPosition().y;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::Bottom:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                    // We can know this edge if: otherWin is win's parent,
                    // or otherWin has a satisfied constraint, or
                    // otherWin has no constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the bottom edge position if we know the top edge
                    // and we know the height; OR if we know the centre edge and
                    // the height.
                    if (constraints->top.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->top.GetValue() + constraints->height.GetValue() - margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreY.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->centreY.GetValue() + (constraints->height.GetValue()/2) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int w, h;
                    wxGetAsIs(win, &w, &h);
                    int y_pos = win->GetPosition().y;
                    value = h + y_pos;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::CenterX:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                    // We can know this edge if: otherWin is win's parent, or
                    // otherWin has a satisfied constraint, or otherWin has no
                    // constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the centre position if we know
                    // the left-hand edge and we know the width, OR
                    // the right-hand edge and the width
                    if (constraints->left.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->left.GetValue() + (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->right.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->left.GetValue() - (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::CenterY:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                    // We can know this edge if: otherWin is win's parent,
                    // or otherWin has a satisfied constraint, or otherWin
                    // has no constraint.
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100 + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                    // We know the centre position if we know
                    // the top edge and we know the height, OR
                    // the bottom edge and the height.
                    if (constraints->bottom.GetDone() && constraints->height.GetDone())
                    {
                        value = (int)(constraints->bottom.GetValue() - (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->top.GetDone() && constraints->height.GetDone())
                    {
                        value = (int)(constraints->top.GetValue() + (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::Width:
        {
            switch (relationship)
            {
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    if (win)
                    {
                        int h;
                        wxGetAsIs(win, &value, &h);
                        done = true;
                        return true;
                    }
                    else return false;
                }
                case wxUnconstrained:
                {
                    // We know the width if we know the left edge and the right edge, OR
                    // if we know the left edge and the centre, OR
                    // if we know the right edge and the centre
                    if (constraints->left.GetDone() && constraints->right.GetDone())
                    {
                        value = constraints->right.GetValue() - constraints->left.GetValue();
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->left.GetDone())
                    {
                        value = (int)(2*(constraints->centreX.GetValue() - constraints->left.GetValue()));
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->right.GetDone())
                    {
                        value = (int)(2*(constraints->right.GetValue() - constraints->centreX.GetValue()));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxEdge::Height:
        {
            switch (relationship)
            {
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (edgePos * percent) / 100;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    if (win)
                    {
                        int w;
                        wxGetAsIs(win, &w, &value);
                        done = true;
                        return true;
                    }
                    else return false;
                }
                case wxUnconstrained:
                {
                    // We know the height if we know the top edge and the bottom edge, OR
                    // if we know the top edge and the centre, OR
                    // if we know the bottom edge and the centre
                    if (constraints->top.GetDone() && constraints->bottom.GetDone())
                    {
                        value = constraints->bottom.GetValue() - constraints->top.GetValue();
                        done = true;
                        return true;
                    }
                    else if (constraints->top.GetDone() && constraints->centreY.GetDone())
                    {
                        value = (int)(2*(constraints->centreY.GetValue() - constraints->top.GetValue()));
                        done = true;
                        return true;
                    }
                    else if (constraints->bottom.GetDone() && constraints->centreY.GetDone())
                    {
                        value = (int)(2*(constraints->bottom.GetValue() - constraints->centreY.GetValue()));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

// Get the value of this edge or dimension, or if this is not determinable, -1.
int wxIndividualLayoutConstraint::GetEdge(wxEdge which,
                                          wxWindowBase *thisWin,
                                          wxWindowBase *other) const
{
    // If the edge or dimension belongs to the parent, then we know the
    // dimension is obtainable immediately. E.g. a wxExpandSizer may contain a
    // button (but the button's true parent is a panel, not the sizer)
    if (other->GetChildren().Find((wxWindow*)thisWin))
    {
        switch (which)
        {
            case wxEdge::Left:
                {
                    return 0;
                }
            case wxEdge::Top:
                {
                    return 0;
                }
            case wxEdge::Right:
                {
                    return other->GetClientSizeConstraint().x;
                }
            case wxEdge::Bottom:
                {
                    return other->GetClientSizeConstraint().y;
                }
            case wxEdge::Width:
                {
                    return other->GetClientSizeConstraint().x;
                }
            case wxEdge::Height:
                {
                    return other->GetClientSizeConstraint().y;
                }
            case wxEdge::CenterX:
            case wxEdge::CenterY:
                {
                    wxSize sz_constraint = other->GetClientSizeConstraint();
                    // FIXME: rounding?
                    if (which == wxEdge::CenterX)
                        return wx::narrow_cast<int>(sz_constraint.x / 2);
                    else
                        return wx::narrow_cast<int>(sz_constraint.y / 2);
                }
            default:
                return -1;
        }
    }
    switch (which)
    {
        case wxEdge::Left:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->left.GetDone())
                        return constr->left.GetValue();
                    else
                        return -1;
                }
                else
                {
                    return other->GetPosition().x;
                }
            }
        case wxEdge::Top:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->top.GetDone())
                        return constr->top.GetValue();
                    else
                        return -1;
                }
                else
                {
                    return other->GetPosition().y;
                }
            }
        case wxEdge::Right:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->right.GetDone())
                        return constr->right.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x_pos = other->GetPosition().x;
                    int w = other->GetSize().x;
                    return x_pos + w;
                }
            }
        case wxEdge::Bottom:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->bottom.GetDone())
                        return constr->bottom.GetValue();
                    else
                        return -1;
                }
                else
                {
                    auto y_pos = other->GetPosition().y;
                    auto h_size = other->GetSize().y;
                    return y_pos + h_size;
                }
            }
        case wxEdge::Width:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->width.GetDone())
                        return constr->width.GetValue();
                    else
                        return -1;
                }
                else
                {
                    return other->GetSize().x;
                }
            }
        case wxEdge::Height:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->height.GetDone())
                        return constr->height.GetValue();
                    else
                        return -1;
                }
                else
                {
                    return other->GetSize().y;
                }
            }
        case wxEdge::CenterX:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->centreX.GetDone())
                        return constr->centreX.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x_pos = other->GetPosition().x;
                    int w = other->GetSize().x;
                    return x_pos + (w/2);
                }
            }
        case wxEdge::CenterY:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                // If no constraints, it means the window is not dependent
                // on anything, and therefore we know its value immediately
                if (constr)
                {
                    if (constr->centreY.GetDone())
                        return constr->centreY.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int y_pos = other->GetPosition().y;
                    int h = other->GetSize().y;
                    return y_pos + (h/2);
                }
            }
        default:
            break;
    }
    return -1;
}

wxLayoutConstraints::wxLayoutConstraints()
{
    left.SetEdge(wxEdge::Left);
    top.SetEdge(wxEdge::Top);
    right.SetEdge(wxEdge::Right);
    bottom.SetEdge(wxEdge::Bottom);
    centreX.SetEdge(wxEdge::CenterX);
    centreY.SetEdge(wxEdge::CenterY);
    width.SetEdge(wxEdge::Width);
    height.SetEdge(wxEdge::Height);
}

bool wxLayoutConstraints::SatisfyConstraints(wxWindowBase *win, int *nChanges)
{
    int noChanges = 0;

    bool done = width.GetDone();
    bool newDone = (done ? true : width.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = height.GetDone();
    newDone = (done ? true : height.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = left.GetDone();
    newDone = (done ? true : left.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = top.GetDone();
    newDone = (done ? true : top.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = right.GetDone();
    newDone = (done ? true : right.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = bottom.GetDone();
    newDone = (done ? true : bottom.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = centreX.GetDone();
    newDone = (done ? true : centreX.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = centreY.GetDone();
    newDone = (done ? true : centreY.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    *nChanges = noChanges;

    return AreSatisfied();
}

#endif // wxUSE_CONSTRAINTS
