/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <IDocumentSettingAccess.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>
#include <tools/lineend.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf128197)
{
    load(mpTestDocumentPath, "128197_compat14.docx");
    xmlDocPtr pLayout14 = parseLayoutDump();
    sal_Int32 nHeight14 = getXPath(pLayout14, "//page[1]/body/txt[1]/infos/bounds", "height").toInt32();

    load(mpTestDocumentPath, "128197_compat15.docx");
    xmlDocPtr pLayout15 = parseLayoutDump();
    sal_Int32 nHeight15 = getXPath(pLayout15, "//page[1]/body/txt[1]/infos/bounds", "height").toInt32();

    // In compat mode=14 second line has size of the shape thus entire paragraph height is smaller
    // So nHeight14 < nHeight15
    CPPUNIT_ASSERT_LESS(nHeight15, nHeight14);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf78749, "tdf78749.docx")
{
    //Shape lost the background image before, now check if it still has...
    auto xShape = getShape(1);
    uno::Reference<beans::XPropertySet> xShpProps(xShape, uno::UNO_QUERY);
    OUString aPropertyVal;
    xShpProps->getPropertyValue("FillBitmapName") >>= aPropertyVal;
    CPPUNIT_ASSERT(!aPropertyVal.isEmpty());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128207, "tdf128207.docx")
{
    //There was the charts on each other, because their horizontal and vertical position was 0!
    xmlDocPtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPathContent(p_XmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor/wp:positionH/wp:posOffset", "4445");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf123873, "tdf123873.docx")
{
    //OLE Object were overlapped due to missing wrap import
    xmlDocPtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:drawing/wp:anchor/wp:wrapTopAndBottom");
}


DECLARE_OOXMLIMPORT_TEST(testTdf129888vml, "tdf129888vml.docx")
{
    //the line shape has anchor in the first cell however it has to
    //be positioned to an another cell. To reach this we must handle
    //the o:allowincell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888vml The line shape has bad place!",
                                 text::RelOrientation::PAGE_FRAME, nValue);
}

DECLARE_OOXMLIMPORT_TEST(testTdf129888dml, "tdf129888dml.docx")
{
    //the shape has anchor in the first cell however it has to
    //be positioned to the right side of the page. To reach this we must handle
    //the layoutInCell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888dml The shape has bad place!",
                                 text::RelOrientation::PAGE_FRAME, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569v, "tdf87569_vml.docx")
{
    //the original tdf87569 sample has vml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_vml: The Shape is not in the table!",
                                 text::RelOrientation::FRAME, nValue);
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(Test, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for ARABIC_ZERO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "decimalZero");
}

CPPUNIT_TEST_FIXTURE(Test, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for SYMBOL_CHICAGO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "chicago");
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569d, "tdf87569_drawingml.docx")
{
    //if the original tdf87569 sample is upgraded it will have drawingml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_drawingml: The Shape is not in the table!",
                                 text::RelOrientation::FRAME, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf130610, "tdf130610_bold_in_2_styles.ott")
{
    // check character properties
    {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles("CharacterStyles")->getByName("WollMuxRoemischeZiffer"),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
    }

    // check paragraph properties
    {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles("ParagraphStyles")->getByName("WollMuxVerfuegungspunkt"),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
    }

    // check inline text properties
    {
        xmlDocPtr pXmlDoc =parseExport("word/document.xml");
        if (pXmlDoc)
        {
            assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:b");
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf78352, "tdf78352.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Ensure that width of first tab is close to zero (previous value was ~1000 twips)
    int nWidth = parseDump("/root/page/body/txt[1]/Text[@nType='PortionType::TabLeft']", "nWidth").toInt32();
    CPPUNIT_ASSERT_LESS(150, nWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf120315, "tdf120315.docx")
{
    // tdf#120315 cells of the second column weren't vertically merged
    // because their horizontal positions are different a little bit
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    CPPUNIT_ASSERT_EQUAL(getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                             .Position,
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), "TableColumnSeparators")[2]
                             .Position);
}

DECLARE_OOXMLEXPORT_TEST(testTdf108350_noFontdefaults, "tdf108350_noFontdefaults.docx")
{
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("NoParent"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xStyleProps, "CharFontName"));
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Font size", 10.f, getProperty<float>(xStyleProps, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testPageContentBottom, "page-content-bottom.docx")
{
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10 (PAGE_PRINT_AREA_BOTTOM)
    // - Actual  : 0 (FRAME)
    // i.e. the bottom-of-body relation was lost.
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038, "tdf125038.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone:...
    // - Actual  : result1result2phone:...
    // i.e. the result if the inner MERGEFIELD fields ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: \t1234567890"), aActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf124986, "tdf124986.docx")
{
    // Load a document with SET fields, where the SET fields contain leading/trailing quotation marks and spaces.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("demo"), aValue);
        }
    }
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038b, "tdf125038b.docx")
{
    // Load a document with an IF field, where the IF field command contains a paragraph break.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphAccess->createEnumeration();
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone: 1234
    // - Actual  :
    // i.e. the first paragraph was empty and the second paragraph had the content.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: 1234"), xParagraph->getString());
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    xParagraphs->nextElement();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !xParagraphs->hasMoreElements()
    // i.e. the document had 3 paragraphs, while only 2 was expected.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038c, "tdf125038c.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: email: test@test.test
    // - Actual  : email:
    // I.e. the result of the MERGEFIELD field inside an IF field was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("email: test@test.test"), aActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf83309, "tdf83309.docx")
{
    // Important: bug case had 4 pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // First paragraph does not have tab before
    // (same applies to all paragraphs in doc, but lets assume they are
    // behave same way)
    OUString sNodeType = parseDump("/root/page[1]/body/txt[1]/Text[1]", "nType");
    CPPUNIT_ASSERT_EQUAL(OUString("PortionType::Text"), sNodeType);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121661, "tdf121661.docx")
{
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:hyphenationZone", "val", "851");
}

DECLARE_OOXMLEXPORT_TEST(testTdf121658, "tdf121658.docx")
{
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:doNotHyphenateCaps");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testTableStyleConfNested)
{
    // Create the doc model.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "table-style-conf-nested.docx";
    loadURL(aURL, nullptr);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top", "val", "nil");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testZeroLineSpacing)
{
    // Create the doc model.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue("ParaLineSpacing", uno::makeAny(aSpacing));

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "lineRule", "atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "line", "0");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserField)
{
    // Create an in-memory empty document with a user field.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::makeAny(OUString("foo")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::makeAny(OUString("bar")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText", " DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t", "bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "name", "foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "val", "bar");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testSemiTransparentText)
{
    // Create an in-memory empty document.
    loadURL("private:factory/swriter", nullptr);

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    OString aXPath
        = "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha";
    double fValue = getXPath(pXmlDoc, aXPath, "val").toDouble();
    sal_Int16 nActual = basegfx::fround(fValue / oox::drawingml::PER_PERCENT);

    // Without the accompanying fix in place, this test would have failed, as the w14:textFill
    // element was missing.
    CPPUNIT_ASSERT_EQUAL(nTransparence, nActual);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf132766, "tdf132766.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Ensure that for list=1 and level=0 we wrote correct bullet char and correct font
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:lvlText",
                "val", u"\uF0B7");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "ascii", "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "hAnsi", "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "cs", "Symbol");
}

DECLARE_OOXMLEXPORT_TEST(testTdf124367, "tdf124367.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // it was 2761 at the first import, and 2760 at the second import, due to incorrect rounding
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2762),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(2), "TableColumnSeparators")[0]
                             .Position);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf95189, "tdf95189.docx")
{
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128820, "tdf128820.fodt")
{
    // Import of exported DOCX failed because of wrong namespase used for wsp element
    // Now test the exported XML, in case we stop failing opening invalid files
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // The parent wpg:wgp element has three children: wpg:cNvGrpSpPr, wpg:grpSpPr, and wpg:wsp
    // (if we start legitimately exporting additional children, this needs to be adjusted to check
    // all those, to make sure we don't export wrong elements).
    assertXPathChildren(pXml,
                        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                        "wp:inline/a:graphic/a:graphicData/wpg:wgp",
                        3);
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:cNvGrpSpPr");
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:grpSpPr");
    // This one was pic:wsp instead of wps:wsp
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128889, "tdf128889.fodt")
{
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // There was an w:r (with w:br) as an invalid child of first paragraph's w:pPr
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:pPr/w:r", 0);
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r", 2);
    // Check that the break is in proper - last - position
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf132754, "tdf132754.docx")
{
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.0."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.2."), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf129353, "tdf129353.docx")
{
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    getParagraph(1, "(Verne, 1870)");
    getParagraph(2, "Bibliography");
    getParagraph(4, "Christie, A. (1922). The Secret Adversary. ");
    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(8)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Check that all the pre-rendered entries are correct, including trailing spaces
    CPPUNIT_ASSERT_EQUAL(OUString("\n" // starting with an empty paragraph
                                  "Christie, A. (1922). The Secret Adversary. \n"
                                  "\n"
                                  "Verne, J. G. (1870). Twenty Thousand Leagues Under the Sea. \n"
                                  ""), // ending with an empty paragraph
                         aIndexString);
}

DECLARE_OOXMLEXPORT_TEST(testTdf120394, "tdf120394.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(CHAR_ZWSP), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(CHAR_ZWSP), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.2.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testHyphenationAuto, "hyphenation.odt")
{
    // Explicitly set hyphenation=auto on document level
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);
    assertXPath(pXmlSettings, "/w:settings/w:autoHyphenation", "val", "true");

    // Second paragraph has explicitly enabled hyphenation
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:p[2]/w:pPr/w:suppressAutoHyphens", "val", "false");

    // Default paragraph style explicitly disables hyphens
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    CPPUNIT_ASSERT(pXmlStyles);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/w:suppressAutoHyphens", "val", "true");
}

DECLARE_OOXMLEXPORT_TEST(testContSectBreakHeaderFooter, "cont-sect-break-header-footer.docx")
{
    // Load a document with a continuous section break on page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"),
                         parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 1"),
                         parseDump("/root/page[1]/footer/txt/text()"));
    // Make sure the header stays like this; if we naively just update the page style name of the
    // first para on page 2, then this would be 'Header, section 2', which is incorrect.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 2"),
                         parseDump("/root/page[2]/footer/txt/text()"));
    // This is inherited from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Header, section 2"),
                         parseDump("/root/page[3]/header/txt/text()"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - xpath should match exactly 1 node
    // i.e. the footer had no text (inherited from page 2), while the correct behavior is to provide
    // the own footer text.
    CPPUNIT_ASSERT_EQUAL(OUString("Footer, section 3"),
                         parseDump("/root/page[3]/footer/txt/text()"));

    // Without the export fix in place, the import-export-import test would have failed with:
    // - Expected: Header, section 2
    // - Actual  : First page header, section 2
    // i.e. both the header and the footer on page 3 was wrong.

    // Additional problem: top margin on page 3 was wrong.
    if (mbExported)
    {
        xmlDocPtr pXml = parseExport("word/document.xml");
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2200
        // - Actual  : 2574
        // i.e. the top margin on page 3 was too large and now matches the value from the input
        // document.
        assertXPath(pXml, "/w:document/w:body/w:sectPr/w:pgMar", "top", "2200");
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
