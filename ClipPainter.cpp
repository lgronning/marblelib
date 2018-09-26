//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2009 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//


#include "ClipPainter.h"

#include <cmath>

#include "MarbleDebug.h"

using namespace Marble;

ClipPainter::ClipPainter(QPaintDevice * pd)
    : QPainter( pd )
{
}


ClipPainter::ClipPainter()
{
}


ClipPainter::~ClipPainter()
{
}


void ClipPainter::drawPolygon ( const QPolygonF & polygon,
                                Qt::FillRule fillRule,
                                const QPixmap &pointPixmap)
{
    QPainter::drawPolygon ( polygon, fillRule );

    foreach( const QPointF & point, polygon )
    {
        QPainter::drawPixmap(QPointF(point.x() - pointPixmap.width()/2, point.y() - pointPixmap.height()/2), pointPixmap);
    }
}

void ClipPainter::drawPolyline(const QPolygonF & polygon,
                               const QPixmap &pointPixmap)
{
    QPainter::drawPolyline( polygon );

    if(!pointPixmap.isNull())
    {
        foreach( const QPointF & point, polygon )
        {
            QPainter::drawPixmap(QPointF(point.x() - pointPixmap.width()/2, point.y() - pointPixmap.height()/2), pointPixmap);
        }
    }
}


