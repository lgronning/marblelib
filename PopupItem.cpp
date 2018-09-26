#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012   Torsten Rahn      <tackat@kde.org>
// Copyright 2013   Mohammed Nafees   <nafees.technocool@gmail.com>
// Copyright 2012   Dennis Nienhüser  <nienhueser@kde.org>
// Copyright 2012   Illya Kovalevskyy <illya.kovalevskyy@gmail.com>
//

#include "PopupItem.h"
#include "MarbleWidget.h"
#include "ColorMap.h"
#include "ColorMapInterval.h"

#include <QtWebKitWidgets/QWebView>
#include <QtWebKit/QWebHistory>

#include <QPointer>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QMouseEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QKeyEvent>
#include <QClipboard>
#include <QFrame>

#include <qdrawutil.h>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QPixmap>
#include <QtWidgets/QHBoxLayout>

namespace Marble
{

class ColorMapWidget : public QWidget
{
    ColorMapWidget(Marble::ColorMapPtr colorMap, const Marble::ColorMapInterval &interval, Qt::Orientation orientation, QWidget *parent = nullptr)
        :   QWidget(parent),
            aColorMap(colorMap),
            aInterval(interval),
            aOrientation(orientation)
    {}

    QSize
    sizeHint() const override
    {
        QSize size = QWidget::sizeHint();

        return size.expandedTo(QSize(10,10));
    }

protected:
    void
    paintEvent(QPaintEvent *event) override;

private:
    void
    drawColorBar( QPainter *painter);

    Marble::ColorMapPtr aColorMap;
    Marble::ColorMapInterval aInterval;
    Qt::Orientation aOrientation;
};


void
ColorMapWidget::paintEvent(QPaintEvent *event)
{
    QRect rect = this->rect();

    QVector<QRgb> colorTable;
    if ( aColorMap->format() == Marble::ColorMap::Indexed )
        colorTable = aColorMap->colorTable( aInterval );

    QColor c;

    /*
      We paint to a pixmap first to have something scalable for printing
      ( f.e. in a Pdf document )
     */

    QPixmap pixmap( rect.size() );
    pixmap.fill( Qt::transparent );

    QPainter pmPainter( &pixmap );
    pmPainter.translate( -rect.x(), -rect.y() );

    double valueStep = (aInterval.end-aInterval.begin)/rect.width();
    if ( aOrientation == Qt::Horizontal )
    {
        for ( int x = rect.left(); x <= rect.right(); x++ )
        {
            const double value = (x-rect.left())*valueStep;;

            if ( aColorMap->format() == Marble::ColorMap::RGB )
                c.setRgba( aColorMap->rgb( aInterval, value ) );
            else
                c = colorTable[aColorMap->colorIndex( aInterval, value )];

            pmPainter.setPen( c );
            pmPainter.drawLine( x, rect.top(), x, rect.bottom() );
        }
    }
    else // Vertical
    {
        for ( int y = rect.top(); y <= rect.bottom(); y++ )
        {
            const double value = (rect.width() - qAbs(rect.top()-y))*valueStep;

            if ( aColorMap->format() == Marble::ColorMap::RGB )
                c.setRgba( aColorMap->rgb( aInterval, value ) );
            else
                c = colorTable[aColorMap->colorIndex( aInterval, value )];

            pmPainter.setPen( c );
            pmPainter.drawLine( rect.left(), y, rect.right(), y );
        }
    }
    pmPainter.end();

    QPainter painter(this);
    painter.drawPixmap( rect, pixmap );
}

class InfoFrame : public QFrame
{
public:
    InfoFrame(QWidget *parent = 0);

    InfoFrame(const QString &text, const QPixmap &pixmap, QWidget *parent = 0);

private:
    void
    init(const QString &text, const QPixmap &pixmap);

    QLabel *textLabel;
    QLabel *pixmapLabel;

    QPixmap aPixmap;
    QString aText;
};


InfoFrame::InfoFrame(QWidget *parent)
    :   QFrame(parent)
{
    init(QString(), QPixmap());
}

InfoFrame::InfoFrame(const QString &text, const QPixmap &pixmap, QWidget *parent)
    :   QFrame(parent)
{
    init(text, pixmap);
}

void
InfoFrame::init(const QString &text, const QPixmap &pixmap)
{
    setFrameShape(QFrame::NoFrame);
    setContentsMargins(1,1,1,1);

    QPalette palette = this->palette();
    palette.setColor(QPalette::All, QPalette::Background, QColor(210, 225, 242, 180));
    palette.setColor(QPalette::All, QPalette::WindowText, Qt::black);
    setPalette(palette);
    setBackgroundRole(QPalette::Background);
    setAutoFillBackground(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    QVBoxLayout *pixmapLayout = new QVBoxLayout();
    pixmapLayout->setSpacing(0);
    pixmapLayout->setMargin(0);
    layout->addLayout(pixmapLayout);

    pixmapLabel = new QLabel(this);
    pixmapLabel->setFrameShape(QFrame::NoFrame);
    pixmapLabel->setPixmap(pixmap);
    pixmapLabel->setScaledContents(true);

    pixmapLayout->addWidget(pixmapLabel);
    pixmapLayout->addStretch(1);

    textLabel = new QLabel(text, this);
    textLabel->setFrameShape(QFrame::NoFrame);
    layout->addWidget(textLabel);

    QFont f = textLabel->font();

    f.setPointSize(7);
    textLabel->setFont(f);

    textLabel->setContentsMargins(2, 2, 2, 2);

}


PopupItem::PopupItem( QObject* parent ) :
    QObject( parent ),
    BillboardGraphicsItem()
{
    setCacheMode( ItemCoordinateCache );
    setVisible( false );
}

PopupItem::~PopupItem()
{
}

void PopupItem::setContent( const QString &html, const QPixmap &pixmap)
{
    aPixmap = pixmap;
    aText = html;

    requestUpdate();
}

QSize
PopupItem::sizeHint() const
{
    InfoFrame infoFrame(aText, aPixmap);

    QSize size = infoFrame.sizeHint();
    size = size.boundedTo(QSize(600, 450));
    qDebug() << "PopupItem::sizeHint" << size << aText;
    return size;
}

void PopupItem::paint( QPainter *painter )
{
    InfoFrame infoFrame(aText, aPixmap);
    infoFrame.setFixedSize(size().toSize());

    qDebug() << "PopupItem::sizeHint" << size().toSize();

    infoFrame.render(painter, QPoint( 0, 0 ));
}

void PopupItem::requestUpdate()
{
    update();
    emit repaintNeeded();
}

}

//#include "moc_PopupItem.cpp"
