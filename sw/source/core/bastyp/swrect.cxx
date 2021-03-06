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

#include <swrect.hxx>

#include <libxml/xmlwriter.h>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#endif

SwRect::SwRect( const tools::Rectangle &rRect ) :
    m_Point( rRect.Left(), rRect.Top() )
{
    m_Size.setWidth( rRect.IsWidthEmpty() ? 0 : rRect.Right()  - rRect.Left() + 1);
    m_Size.setHeight(rRect.IsHeightEmpty() ? 0 : rRect.Bottom() - rRect.Top()  + 1);
}

Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );
}

SwRect& SwRect::Intersection( const SwRect& rRect )
{
    // any similarity between me and given element?
    if ( IsOver( rRect ) )
    {
        // get smaller right and lower, and greater left and upper edge
        if ( Left() < rRect.Left() )
            Left( rRect.Left() );
        if ( Top() < rRect.Top() )
            Top( rRect.Top() );
        long n = rRect.Right();
        if ( Right() > n )
            Right( n );
        n = rRect.Bottom();
        if ( Bottom() > n )
            Bottom( n );
    }
    else
        // Def.: if intersection is empty, set only SSize to 0
        SSize(0, 0);

    return *this;
}

SwRect& SwRect::Intersection_( const SwRect& rRect )
{
    // get smaller right and lower, and greater left and upper edge
    if ( Left() < rRect.Left() )
        Left( rRect.Left() );
    if ( Top() < rRect.Top() )
        Top( rRect.Top() );
    long n = rRect.Right();
    if ( Right() > n )
        Right( n );
    n = rRect.Bottom();
    if ( Bottom() > n )
        Bottom( n );

    return *this;
}

// mouse moving of table borders
bool SwRect::IsNear( const Point& rPoint, long nTolerance ) const
{
    bool bIsNearby = (((Left()   - nTolerance) <= rPoint.X()) &&
                      ((Top()    - nTolerance) <= rPoint.Y()) &&
                      ((Right()  + nTolerance) >= rPoint.X()) &&
                      ((Bottom() + nTolerance) >= rPoint.Y()));
    return IsInside(rPoint) || bIsNearby;
}

bool SwRect::IsOver( const SwRect& rRect ) const
{
    return (Top()   <= rRect.Bottom()) &&
           (Left()  <= rRect.Right())  &&
           (Right() >= rRect.Left())   &&
           (Bottom()>= rRect.Top());
}

void SwRect::Justify()
{
    if ( m_Size.getHeight() < 0 )
    {
        m_Point.setY(m_Point.getY() + m_Size.getHeight() + 1);
        m_Size.setHeight(-m_Size.getHeight());
    }
    if ( m_Size.getWidth() < 0 )
    {
        m_Point.setX(m_Point.getX() + m_Size.getWidth() + 1);
        m_Size.setWidth(-m_Size.getWidth());
    }
}

// Similar to the inline methods, but we need the function pointers
void SwRect::Width_( const long nNew ) { m_Size.setWidth(nNew); }
void SwRect::Height_( const long nNew ) { m_Size.setHeight(nNew); }
void SwRect::Left_( const long nLeft ){ m_Size.AdjustWidth(m_Point.getX() - nLeft ); m_Point.setX(nLeft); }
void SwRect::Right_( const long nRight ){ m_Size.setWidth(nRight - m_Point.getX()); }
void SwRect::Top_( const long nTop ){ m_Size.AdjustHeight(m_Point.getY() - nTop ); m_Point.setY(nTop); }
void SwRect::Bottom_( const long nBottom ){ m_Size.setHeight(nBottom - m_Point.getY()); }

long SwRect::Width_() const{ return m_Size.getWidth(); }
long SwRect::Height_() const{ return m_Size.getHeight(); }
long SwRect::Left_() const{ return m_Point.getX(); }
long SwRect::Right_() const{ return m_Point.getX() + m_Size.getWidth(); }
long SwRect::Top_() const{ return m_Point.getY(); }
long SwRect::Bottom_() const{ return m_Point.getY() + m_Size.getHeight(); }

void SwRect::AddWidth( const long nAdd ) { m_Size.AdjustWidth(nAdd ); }
void SwRect::AddHeight( const long nAdd ) { m_Size.AdjustHeight(nAdd ); }
void SwRect::SubLeft( const long nSub ){ m_Size.AdjustWidth(nSub ); m_Point.setX(m_Point.getX() - nSub); }
void SwRect::AddRight( const long nAdd ){ m_Size.AdjustWidth(nAdd ); }
void SwRect::SubTop( const long nSub ){ m_Size.AdjustHeight(nSub ); m_Point.setY(m_Point.getY() - nSub); }
void SwRect::AddBottom( const long nAdd ){ m_Size.AdjustHeight(nAdd ); }
void SwRect::SetPosX( const long nNew ){ m_Point.setX(nNew); }
void SwRect::SetPosY( const long nNew ){ m_Point.setY(nNew); }

Size  SwRect::Size_() const { return SSize(); }
Size  SwRect::SwappedSize() const { return Size( m_Size.getHeight(), m_Size.getWidth() ); }

Point SwRect::TopLeft() const { return Pos(); }
Point SwRect::TopRight() const { return Point( m_Point.getX() + m_Size.getWidth(), m_Point.getY() ); }
Point SwRect::BottomLeft() const { return Point( m_Point.getX(), m_Point.getY() + m_Size.getHeight() ); }
Point SwRect::BottomRight() const
    { return Point( m_Point.getX() + m_Size.getWidth(), m_Point.getY() + m_Size.getHeight() ); }

long SwRect::GetLeftDistance( long nLimit ) const { return m_Point.getX() - nLimit; }
long SwRect::GetBottomDistance( long nLim ) const { return nLim - m_Point.getY() - m_Size.getHeight();}
long SwRect::GetTopDistance( long nLimit ) const { return m_Point.getY() - nLimit; }
long SwRect::GetRightDistance( long nLim ) const { return nLim - m_Point.getX() - m_Size.getWidth(); }

bool SwRect::OverStepLeft( long nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }
bool SwRect::OverStepBottom( long nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
bool SwRect::OverStepTop( long nLimit ) const
    { return nLimit > m_Point.getY() && m_Point.getY() + m_Size.getHeight() > nLimit; }
bool SwRect::OverStepRight( long nLimit ) const
    { return nLimit > m_Point.getX() && m_Point.getX() + m_Size.getWidth() > nLimit; }

void SwRect::SetLeftAndWidth( long nLeft, long nNew )
{
    m_Point.setX(nLeft);
    m_Size.setWidth(nNew);
}
void SwRect::SetTopAndHeight( long nTop, long nNew )
{
    m_Point.setY(nTop);
    m_Size.setHeight(nNew);
}
void SwRect::SetRightAndWidth( long nRight, long nNew )
{
    m_Point.setX(nRight - nNew);
    m_Size.setWidth(nNew);
}
void SwRect::SetBottomAndHeight( long nBottom, long nNew )
{
    m_Point.setY(nBottom - nNew);
    m_Size.setHeight(nNew);
}
void SwRect::SetUpperLeftCorner(  const Point& rNew )
    { m_Point = rNew; }
void SwRect::SetUpperRightCorner(  const Point& rNew )
    { m_Point = Point(rNew.X() - m_Size.getWidth(), rNew.Y()); }
void SwRect::SetLowerLeftCorner(  const Point& rNew )
    { m_Point = Point(rNew.X(), rNew.Y() - m_Size.getHeight()); }

void SwRect::dumpAsXmlAttributes(xmlTextWriterPtr writer) const
{
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("left"), "%li", Left());
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("top"), "%li", Top());
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("width"), "%li", Width());
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("height"), "%li", Height());
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("bottom"), "%li", Bottom());
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("right"), "%li", Right());
}

#ifdef DBG_UTIL
SvStream& WriteSwRect(SvStream &rStream, const SwRect &rRect)
{
    rStream.WriteChar('[').WriteInt32(rRect.Top()).
            WriteChar('/').WriteInt32(rRect.Left()).
            WriteChar(',').WriteInt32(rRect.Width()).
            WriteChar('x').WriteInt32(rRect.Height()).
            WriteCharPtr("] ");
    return rStream;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
