//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012   Torsten Rahn      <tackat@kde.org>
// Copyright 2012   Mohammed Nafees   <nafees.technocool@gmail.com>
// Copyright 2012   Illya Kovalevskyy <illya.kovalevskyy@gmail.com>
//

#ifndef POPUPITEM_H
#define POPUPITEM_H

#include <QObject>
#include <QUrl>
#include <QLabel>

#include "graphicsview/BillboardGraphicsItem.h"

class QPainter;
class InfoFrame;

namespace Marble
{

/**
 * @brief The PopupItem Class
 *
 * This class represents graphics item for information bubble.
 * Mostly used by @see MapInfoDialog.
 *
 * It has nice API for QWebView and methods for styling it.
 *
 */

class PopupItem : public QObject, public BillboardGraphicsItem
{
    Q_OBJECT
public:
    explicit PopupItem( QObject* parent = 0 );
    ~PopupItem() override;

    /**
     * @brief Set content of the popup
     *
     * There is a small web browser inside. It can show custom HTML.
     * This method sets custom @p html for its window
     *
     * @param html custom html for popup
     */
    void setContent(const QString &html, const QPixmap &pixmap = QPixmap());

    /**
     * @brief Sets text color of the header
     *
     * Frame of the web browser is called bubble. Bubble has
     * a header - part of the bubble at the top. Usually
     * it contains the name of the page which can be set via
     * TITLE html tag in HTML document loaded.
     * This method sets text @p color of the header.
     *
     * @param color text color of the header
     */
    void setTextColor( const QColor &color );

    /**
     * @brief Sets background color of the bubble
     *
     * Frame of the web browser is called bubble. This method
     * sets background @p color of this bubble.
     *
     * @param color background color of the bubble
     */
    void setBackgroundColor( const QColor &color );

    QSize
    sizeHint() const;

private Q_SLOTS:
    /**
     * @brief Marks cache as dirty and tells the world its need for repainting.
     */
    void requestUpdate();

protected:
    void paint( QPainter *painter ) override;

Q_SIGNALS:
    void repaintNeeded();

private:
    QPixmap aPixmap;
    QString aText;
};

}

#endif
