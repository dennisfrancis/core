/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pdfdecomposer.hxx"

#include <vector>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/pdfread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/graphic/XPdfDecomposer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace css;

namespace
{
/// Class to convert the PDF data into a XPrimitive2D (containing only a bitmap).
class XPdfDecomposer
    : public ::cppu::WeakAggImplHelper2<graphic::XPdfDecomposer, lang::XServiceInfo>
{
public:
    explicit XPdfDecomposer(uno::Reference<uno::XComponentContext> const& context);
    XPdfDecomposer(const XPdfDecomposer&) = delete;
    XPdfDecomposer& operator=(const XPdfDecomposer&) = delete;

    // XPdfDecomposer
    uno::Sequence<uno::Reference<graphic::XPrimitive2D>> SAL_CALL
    getDecomposition(const uno::Sequence<sal_Int8>& xPdfData,
                     const uno::Sequence<beans::PropertyValue>& xDecompositionParameters) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString&) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

XPdfDecomposer::XPdfDecomposer(uno::Reference<uno::XComponentContext> const&) {}

uno::Sequence<uno::Reference<graphic::XPrimitive2D>> SAL_CALL XPdfDecomposer::getDecomposition(
    const uno::Sequence<sal_Int8>& xPdfData, const uno::Sequence<beans::PropertyValue>& xParameters)
{
    sal_Int32 nPageIndex = -1;

    for (sal_Int32 index = 0; index < xParameters.getLength(); index++)
    {
        const beans::PropertyValue& rProperty = xParameters[index];

        if (rProperty.Name == "PageIndex")
        {
            rProperty.Value >>= nPageIndex;
        }
    }

    if (nPageIndex < 0)
        nPageIndex = 0;

    std::vector<Bitmap> aBitmaps;
    vcl::RenderPDFBitmaps(xPdfData.getConstArray(), xPdfData.getLength(), aBitmaps, nPageIndex, 1);

    BitmapEx aReplacement(aBitmaps[0]);

    // short form for scale and translate transformation
    const Size aDPI(
        Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)));
    const Size aBitmapSize(aReplacement.GetSizePixel());
    const basegfx::B2DHomMatrix aBitmapTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
        aBitmapSize.getWidth() * aDPI.getWidth(), aBitmapSize.getHeight() * aDPI.getHeight(), 0,
        0));

    // create primitive
    uno::Sequence<uno::Reference<graphic::XPrimitive2D>> aSequence(1);
    aSequence[0] = new drawinglayer::primitive2d::BitmapPrimitive2D(aReplacement, aBitmapTransform);

    return aSequence;
}

OUString SAL_CALL XPdfDecomposer::getImplementationName()
{
    return PDFDecomposer_getImplementationName();
}

sal_Bool SAL_CALL XPdfDecomposer::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL XPdfDecomposer::getSupportedServiceNames()
{
    return PDFDecomposer_getSupportedServiceNames();
}
}

OUString PDFDecomposer_getImplementationName() { return "com.sun.star.comp.PDF.PDFDecomposer"; }

uno::Sequence<OUString> PDFDecomposer_getSupportedServiceNames()
{
    return uno::Sequence<OUString>{ "com.sun.star.graphic.PdfTools" };
}

uno::Reference<uno::XInterface>
PDFDecomposer_createInstance(const uno::Reference<lang::XMultiServiceFactory>& rSMgr)
{
    return static_cast<cppu::OWeakObject*>(
        new XPdfDecomposer(comphelper::getComponentContext(rSMgr)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
