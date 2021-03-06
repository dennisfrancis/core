/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/sidebar/ControlFactory.hxx>
#include "FontworkPropertyPanel.hxx"
#include <svx/svxids.hrc>
#include <sfx2/objsh.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svxitems.hrc>
#include <vcl/toolbox.hxx>
#include <svtools/toolbarmenu.hxx>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

namespace svx
{
namespace sidebar
{
FontworkPropertyPanel::FontworkPropertyPanel(vcl::Window* pParent,
                                             const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "FontworkPropertyPanel", "svx/ui/sidebarfontwork.ui", rxFrame)
    , m_xToolbox(get<sfx2::sidebar::SidebarToolBox>("fontwork-toolbox"))
{
    if (comphelper::LibreOfficeKit::isActive())
        m_xToolbox->HideItem(m_xToolbox->GetItemId(".uno:ExtrusionToggle"));
}

void FontworkPropertyPanel::dispose()
{
    m_xToolbox.disposeAndClear();
    PanelLayout::dispose();
}

FontworkPropertyPanel::~FontworkPropertyPanel() { disposeOnce(); }

VclPtr<vcl::Window>
FontworkPropertyPanel::Create(vcl::Window* pParent,
                              const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to FontworkPropertyPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to FontworkPropertyPanel::Create",
                                             nullptr, 1);

    return VclPtr<FontworkPropertyPanel>::Create(pParent, rxFrame);
}
}
} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
