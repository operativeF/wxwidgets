///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/affinetransform.cpp
// Purpose:     Unit test for transformations implemented for wxAffineMatrix2D
// Author:      Catalin Raceanu
// Created:     2011-04-14
// Copyright:   (c) 2011 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/graphics.h"
#include "wx/dcmemory.h"

import Utils.Geometry;

#if wxUSE_GRAPHICS_CONTEXT
#include "wx/dcgraph.h"
#endif // wxUSE_GRAPHICS_CONTEXT

#include "testimage.h"

// ----------------------------------------------------------------------------
// Affine transform test class
// ----------------------------------------------------------------------------

class AffineTransformTestCase : public CppUnit::TestCase
{
public:
    AffineTransformTestCase() {}

private:
    CPPUNIT_TEST_SUITE( AffineTransformTestCase );
        CPPUNIT_TEST( InvertMatrix );
        CPPUNIT_TEST( Concat );
    CPPUNIT_TEST_SUITE_END();

    void InvertMatrix();
    void Concat();

    AffineTransformTestCase(const AffineTransformTestCase&) = delete;
	AffineTransformTestCase& operator=(const AffineTransformTestCase&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( AffineTransformTestCase );

// also include in its own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( AffineTransformTestCase, "AffineTransformTestCase" );

void AffineTransformTestCase::InvertMatrix()
{
    wxAffineMatrix2D matrix1;
    matrix1.Set(wxMatrix2D(2, 1, 1, 1), wxPoint2DDouble(1, 1));

    wxAffineMatrix2D matrix2(matrix1);

    matrix2.Invert();

    wxMatrix2D m;
    wxPoint2DDouble p;
    matrix2.Get(&m, &p);
    CHECK_EQ( 1, (int)m.m_11 );
    CHECK_EQ( -1, (int)m.m_12 );
    CHECK_EQ( -1, (int)m.m_21 );
    CHECK_EQ( 2, (int)m.m_22 );
    CHECK_EQ( 0, (int)p.m_x );
    CHECK_EQ( -1, (int)p.m_y );

    matrix2.Concat(matrix1);
    CHECK( matrix2.IsIdentity() );
}

void AffineTransformTestCase::Concat()
{
    wxAffineMatrix2D m1;
    m1.Set(wxMatrix2D(0.9, 0.4, -0.4, 0.9), wxPoint2DDouble(0.0, 0.0));
    wxAffineMatrix2D m2;
    m2.Set(wxMatrix2D(1.0, 0.0, 0.0, 1.0), wxPoint2DDouble(3.0, 5.0));
    m1.Concat(m2);

    wxMatrix2D m;
    wxPoint2DDouble p;
    m1.Get(&m, &p);

    const double delta = 0.01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.9, m.m_11, delta );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.4, m.m_12, delta );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -0.4, m.m_21, delta );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.9, m.m_22, delta );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.7, p.m_x, delta );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.7, p.m_y, delta );
}

#if wxUSE_DC_TRANSFORM_MATRIX
// -------------------------------
//  Transform matrix test classes
// -------------------------------

// ====================
// wxDC / wxGCDC tests
// ====================

class TransformMatrixTestCaseDCBase : public CppUnit::TestCase
{
public:
    TransformMatrixTestCaseDCBase()
    {
        m_dc = NULL;
        wxImage::AddHandler(new wxJPEGHandler);
        m_imgOrig.LoadFile(wxS("horse.jpg"));
        CHECK( m_imgOrig.IsOk() );
    }

    virtual ~TransformMatrixTestCaseDCBase()
    {
    }

    void setUp() override
    {
        m_bmpOrig = wxBitmap(m_imgOrig);
        m_bmpUsingMatrix.Create(m_bmpOrig.GetSize(), m_bmpOrig.GetDepth());
    }

protected:
    virtual void FlushDC() = 0;

    void VMirrorAndTranslate();
    void Rotate90Clockwise();
#if wxUSE_GRAPHICS_CONTEXT
    void CompareToGraphicsContext();
#endif // wxUSE_GRAPHICS_CONTEXT

protected:
    wxImage m_imgOrig;
    wxBitmap m_bmpOrig;

    wxBitmap m_bmpUsingMatrix;
    wxDC* m_dc;

    TransformMatrixTestCaseDCBase(const TransformMatrixTestCaseDCBase&) = delete;
	TransformMatrixTestCaseDCBase& operator=(const TransformMatrixTestCaseDCBase&) = delete;
};

// ===========
// wxDC tests
// ===========

class TransformMatrixTestCaseDC : public TransformMatrixTestCaseDCBase
{
public:
    TransformMatrixTestCaseDC()
    {
        m_dc = &m_mdc;
    }

    virtual ~TransformMatrixTestCaseDC()
    {
    }

    void setUp() override
    {
        TransformMatrixTestCaseDCBase::setUp();
        m_mdc.SelectObject(m_bmpUsingMatrix);
    }

    void tearDown() override
    {
        m_mdc.SelectObject(wxNullBitmap);
        TransformMatrixTestCaseDCBase::tearDown();
    }

protected:
    void FlushDC() override {}

private:
    CPPUNIT_TEST_SUITE( TransformMatrixTestCaseDC );
        CPPUNIT_TEST( VMirrorAndTranslate );
        CPPUNIT_TEST( Rotate90Clockwise );
#if wxUSE_GRAPHICS_CONTEXT
        CPPUNIT_TEST( CompareToGraphicsContext );
#endif // wxUSE_GRAPHICS_CONTEXT
    CPPUNIT_TEST_SUITE_END();

protected:
    wxMemoryDC m_mdc;

    TransformMatrixTestCaseDC(const TransformMatrixTestCaseDC&) = delete;
	TransformMatrixTestCaseDC& operator=(const TransformMatrixTestCaseDC&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( TransformMatrixTestCaseDC );

// also include in it's own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformMatrixTestCaseDC, "TransformMatrixTestCaseDC" );

#if wxUSE_GRAPHICS_CONTEXT
// =============
// wxGCDC tests
// =============

class TransformMatrixTestCaseGCDC : public TransformMatrixTestCaseDC
{
public:
    TransformMatrixTestCaseGCDC() {}

    virtual ~TransformMatrixTestCaseGCDC() {}

    void setUp() override
    {
        TransformMatrixTestCaseDC::setUp();

        m_gcdc = new wxGCDC(m_mdc);
        m_dc = m_gcdc;

        wxGraphicsContext* ctx = m_gcdc->GetGraphicsContext();
        ctx->SetAntialiasMode(wxANTIALIAS_NONE);
    }

    void tearDown() override
    {
        delete m_gcdc;
        TransformMatrixTestCaseDC::tearDown();
    }

protected:
    void FlushDC() override
    {
        m_gcdc->GetGraphicsContext()->Flush();
    }

private:
    CPPUNIT_TEST_SUITE( TransformMatrixTestCaseGCDC );
        CPPUNIT_TEST( VMirrorAndTranslate );
        CPPUNIT_TEST( Rotate90Clockwise );
    CPPUNIT_TEST_SUITE_END();

protected:
    wxGCDC* m_gcdc;

    TransformMatrixTestCaseGCDC(const TransformMatrixTestCaseGCDC&) = delete;
	TransformMatrixTestCaseGCDC& operator=(const TransformMatrixTestCaseGCDC&) = delete;
};

// For MSW we have individual test cases for each graphics renderer
// so we don't need to test wxGCDC with default renderer.
#ifndef __WXMSW__
// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( TransformMatrixTestCaseGCDC );

// also include in it's own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformMatrixTestCaseGCDC, "TransformMatrixTestCaseGCDC" );
#endif // !__WXMSW__

#ifdef __WXMSW__
// Direct2D are available only under MSW.

#if wxUSE_GRAPHICS_DIRECT2D
class TransformMatrixTestCaseGCDCDirect2D : public TransformMatrixTestCaseGCDC
{
public:
    TransformMatrixTestCaseGCDCDirect2D() {}

    virtual ~TransformMatrixTestCaseGCDCDirect2D() {}

    void setUp() override
    {
        TransformMatrixTestCaseGCDC::setUp();

        wxGraphicsRenderer* rend = wxGraphicsRenderer::GetDirect2DRenderer();
        wxGraphicsContext* ctx = rend->CreateContext(m_mdc);
        m_gcdc->SetGraphicsContext(ctx);
    }

    void FlushDC() override
    {
        // Apparently, flushing native Direct2D renderer
        // is not enough to update underlying DC (bitmap)
        // and therefore we have to destroy the renderer
        // to do so.
        TransformMatrixTestCaseGCDC::FlushDC();
        m_gcdc->SetGraphicsContext(NULL);
    }

private:
    CPPUNIT_TEST_SUITE( TransformMatrixTestCaseGCDCDirect2D );
        CPPUNIT_TEST( VMirrorAndTranslate );
        CPPUNIT_TEST( Rotate90Clockwise );
    CPPUNIT_TEST_SUITE_END();

protected:

    TransformMatrixTestCaseGCDCDirect2D(const TransformMatrixTestCaseGCDCDirect2D&) = delete;
	TransformMatrixTestCaseGCDCDirect2D& operator=(const TransformMatrixTestCaseGCDCDirect2D&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( TransformMatrixTestCaseGCDCDirect2D );

// also include in it's own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformMatrixTestCaseGCDCDirect2D, "TransformMatrixTestCaseGCDCDirect2D" );

#endif // wxUSE_GRAPHICS_DIRECT2D

#endif // __WXMSW__

#if wxUSE_CAIRO
class TransformMatrixTestCaseGCDCCairo : public TransformMatrixTestCaseGCDC
{
public:
    TransformMatrixTestCaseGCDCCairo() {}

    virtual ~TransformMatrixTestCaseGCDCCairo() {}

    void setUp() override
    {
        TransformMatrixTestCaseGCDC::setUp();

        wxGraphicsRenderer* rend = wxGraphicsRenderer::GetCairoRenderer();
        wxGraphicsContext* ctx = rend->CreateContext(m_mdc);
        m_gcdc->SetGraphicsContext(ctx);
    }

private:
    CPPUNIT_TEST_SUITE( TransformMatrixTestCaseGCDCCairo );
        CPPUNIT_TEST( VMirrorAndTranslate );
        CPPUNIT_TEST( Rotate90Clockwise );
    CPPUNIT_TEST_SUITE_END();

protected:

    TransformMatrixTestCaseGCDCCairo(const TransformMatrixTestCaseGCDCCairo&) = delete;
	TransformMatrixTestCaseGCDCCairo& operator=(const TransformMatrixTestCaseGCDCCairo&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( TransformMatrixTestCaseGCDCCairo );

// also include in it's own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TransformMatrixTestCaseGCDCCairo, "TransformMatrixTestCaseGCDCCairo" );

#endif // wxUSE_CAIRO

#endif // wxUSE_GRAPHICS_CONTEXT

void TransformMatrixTestCaseDCBase::VMirrorAndTranslate()
{
    // build the mirrored image using the transformation matrix
    if ( !m_dc->CanUseTransformMatrix() )
        return;

    wxAffineMatrix2D matrix;
    matrix.Mirror(wxVERTICAL);
    // For wxDC pixel center is at (0, 0) so row 0 of the bitmap is the axis
    // of mirroring and it is left intact by the transformation. In this case
    // mirrored bitmap needs to be shifted by dim-1 pixels.
    // For wxGCDC pixel center of underlying wxGraphicsContext is at (0.5, 0.5)
    // so the axis of mirroring is above row 0 of the bitmap and this row
    // is affected by the transformation. In this case mirrored bitmap
    // needs to be shifthed by dim pixels.
    int ty;
#if wxUSE_GRAPHICS_CONTEXT
    if ( m_dc->GetGraphicsContext() )
        ty = m_bmpOrig.GetHeight();
    else
#endif // wxUSE_GRAPHICS_CONTEXT
        ty = m_bmpOrig.GetHeight() - 1;
    matrix.Translate(0, -ty);
    m_dc->SetTransformMatrix(matrix);
    m_dc->DrawBitmap(m_bmpOrig, 0, 0);
    FlushDC();

    CHECK_THAT( m_bmpUsingMatrix.ConvertToImage(),
                RGBSameAs(m_imgOrig.Mirror(false)) );
}

void TransformMatrixTestCaseDCBase::Rotate90Clockwise()
{
    // build the rotated image using the transformation matrix
    if ( !m_dc->CanUseTransformMatrix() )
        return;

    wxAffineMatrix2D matrix;
    matrix.Rotate(0.5 * M_PI);
    matrix.Translate(0, -m_bmpOrig.GetHeight());
    m_dc->SetTransformMatrix(matrix);
    m_dc->DrawBitmap(m_bmpOrig, 0, 0);
    FlushDC();

    CHECK_THAT( m_bmpUsingMatrix.ConvertToImage(),
                RGBSameAs(m_imgOrig.Rotate90(true)) );
}

#if wxUSE_GRAPHICS_CONTEXT
void TransformMatrixTestCaseDCBase::CompareToGraphicsContext()
{
    wxPoint2DDouble pointA1(1.0, 3.0), pointA2(60.0, 50.0),
                    pointG1(1.0, 3.0), pointG2(60.0, 50.0);

    // Create affine matrix and transform it
    wxAffineMatrix2D matrixA1, matrixA2;
    matrixA2.Rotate(M_PI / 3);
    matrixA1.Translate(-m_bmpOrig.GetWidth()/2, -m_bmpOrig.GetHeight()/2);
    matrixA1.Rotate(-M_PI *2/ 6);
    matrixA1.Translate(m_bmpOrig.GetWidth()/2, m_bmpOrig.GetHeight()/2);
    matrixA1.Mirror(wxHORIZONTAL);
    matrixA1.Concat(matrixA2);
    matrixA1.Mirror(wxVERTICAL);
    matrixA1.Translate(m_bmpOrig.GetWidth()/2, -m_bmpOrig.GetHeight()/2);
    matrixA1.Scale(0.9, 0.9);
    matrixA1.Invert();

    // Create image using first matrix
    wxBitmap bmpUsingMatrixA1(m_bmpOrig.GetSize(), m_bmpOrig.GetDepth());

    // Build the transformed image using the transformation matrix
    {
        wxMemoryDC dc(bmpUsingMatrixA1);

        if ( !dc.CanUseTransformMatrix() )
            return;

        // Draw the bitmap
        dc.SetTransformMatrix(matrixA1);
        dc.DrawBitmap(m_bmpOrig, 0, 0);

        // Draw a line
        matrixA1.TransformPoint(&pointA1.m_x, &pointA1.m_y);
        matrixA1.TransformDistance(&pointA2.m_x, &pointA2.m_y);

        dc.DrawLine(std::lround(pointA1.m_x), std::lround(pointA1.m_y),
            std::lround(pointA1.m_x + pointA2.m_x), std::lround(pointA1.m_x + pointA2.m_y));
    }


    // Create graphics matrix and transform it
    wxGraphicsRenderer* r;
    if ( m_dc->GetGraphicsContext() )
    {
        r = m_dc->GetGraphicsContext()->GetRenderer();
    }
    else
    {
        r = wxGraphicsRenderer::GetDefaultRenderer();
    }

    wxBitmap bmp(10, 10);
    wxMemoryDC mDc(bmp);
    wxGraphicsContext* gDc = r->CreateContext(mDc);
    wxGraphicsMatrix matrixG1 = gDc->CreateMatrix();
    wxGraphicsMatrix matrixG2 = gDc->CreateMatrix();
    matrixG2.Rotate(M_PI / 3);
    matrixG1.Translate(-m_bmpOrig.GetWidth()/2, -m_bmpOrig.GetHeight()/2);
    matrixG1.Rotate(-M_PI*2 / 6);
    matrixG1.Translate(m_bmpOrig.GetWidth()/2, m_bmpOrig.GetHeight()/2);
    matrixG1.Scale(-1, 1);
    matrixG1.Concat(matrixG2);
    matrixG1.Scale(1, -1);
    matrixG1.Translate(m_bmpOrig.GetWidth()/2, -m_bmpOrig.GetHeight()/2);
    matrixG1.Scale(0.9, 0.9);
    matrixG1.Invert();
    // Create affine matrix from the graphics matrix
    wxMatrix2D mat2D;
    wxPoint2DDouble tr;
    matrixG1.Get(&mat2D.m_11, &mat2D.m_12, &mat2D.m_21, &mat2D.m_22, &tr.m_x, &tr.m_y);
    wxAffineMatrix2D matrixAG;
    matrixAG.Set(mat2D, tr);

    delete gDc;

    // Create image using last matrix
    wxBitmap bmpUsingMatrixAG(m_bmpOrig.GetHeight(), m_bmpOrig.GetWidth());

    // Build the transformed image using the transformation matrix
    {
        wxMemoryDC dc(bmpUsingMatrixAG);

        if ( !dc.CanUseTransformMatrix() )
            return;

        // Draw the bitmap
        dc.SetTransformMatrix(matrixAG);
        dc.DrawBitmap(m_bmpOrig, 0, 0);

        // Draw a line
        matrixG1.TransformPoint(&pointG1.m_x, &pointG1.m_y);
        matrixG1.TransformDistance(&pointG2.m_x, &pointG2.m_y);

        dc.DrawLine(std::lround(pointG1.m_x), std::lround(pointG1.m_y),
            std::lround(pointG1.m_x + pointG2.m_x), std::lround(pointG1.m_x + pointG2.m_y));
    }


    CHECK_THAT( bmpUsingMatrixA1.ConvertToImage(),
                RGBSameAs(bmpUsingMatrixAG.ConvertToImage()) );

    // Save the images to check that something _is_ inside the visible area.
    //bmpUsingMatrixA1.SaveFile("matrixA1.jpg", wxBitmapType::JPEG);
    //bmpUsingMatrixAG.SaveFile("matrixAG.jpg", wxBitmapType::JPEG);
}
#endif // wxUSE_GRAPHICS_CONTEXT

#endif // wxUSE_DC_TRANSFORM_MATRIX
