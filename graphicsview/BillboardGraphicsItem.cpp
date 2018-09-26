//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2012      Mohammed Nafees <nafees.technocool@gmail.com>
//

// self
#include "BillboardGraphicsItem.h"
#include "MarbleGraphicsItem_p.h"

// Marble
#include "ViewportParams.h"

namespace Marble
{

class Q_DECL_HIDDEN BillboardGraphicsItem::Private : public MarbleGraphicsItemPrivate
{
 public:
    Private( BillboardGraphicsItem *parent ) :
        MarbleGraphicsItemPrivate( parent )
    {
    }

    QList<QPointF> positions() const override
    {
        return QList<QPointF>() << m_itemPosition;
    }

    QList<QPointF> absolutePositions() const override
    {
        return positions();
    }

    Qt::Alignment m_alignment;

    void
    setProjection( const ViewportParams *viewport ) override
    {
        // handle vertical alignment
        int topY = m_pos.y();

        // handle horizontal alignment
        int leftX = m_pos.x() + 20;

        if(topY+m_size.height() > viewport->height())
        {
            topY = viewport->height()-m_size.height();
        }

        if(leftX + m_size.width() > viewport->width())
        {
            leftX = m_pos.x() - 20 - m_size.width();
        }

        m_itemPosition = QPointF( leftX, topY ) ;
    }

    QPoint m_pos;
    QPointF m_itemPosition;
};

BillboardGraphicsItem::BillboardGraphicsItem()
    : MarbleGraphicsItem( new Private( this ) )
{
}

QPoint BillboardGraphicsItem::position() const
{
    return p()->m_pos;
}

void BillboardGraphicsItem::setPosition( const QPoint &pos )
{
    p()->m_pos = pos;
}

QList<QPointF> BillboardGraphicsItem::positions() const
{
    return p()->positions();
}

QList<QRectF>
BillboardGraphicsItem::boundingRects() const
{
    QList<QRectF> rects;
    QSizeF const size = p()->m_size;
    foreach(const QPointF &point, positions()) {
        rects << QRectF(point, size);
    }

    return rects;
}

QRectF
BillboardGraphicsItem::containsRect( const QPointF &point ) const
{
    foreach( const QRectF &rect, boundingRects() )
    {
        if( rect.contains( point ) )
            return rect;
    }

    return QRectF();
}

BillboardGraphicsItem::Private *BillboardGraphicsItem::p()
{
    return static_cast<Private *>( d );
}

const BillboardGraphicsItem::Private *BillboardGraphicsItem::p() const
{
    return static_cast<Private *>( d );
}

} // Marble namespace
