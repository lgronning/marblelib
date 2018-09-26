//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#ifndef MARBLE_MARBLEWIDGETPOPUPMENU_H
#define MARBLE_MARBLEWIDGETPOPUPMENU_H

#include "marble_export.h"

#include <QObject>
#include <QPoint>

class QAction;

namespace Marble
{

class MarbleWidget;
class MarbleModel;
class GeoDataFeature;
/**
 * The MarbleWidgetPopupMenu handles context menus.
 */
class MARBLE_EXPORT MarbleWidgetPopupInfo  : public QObject
{
    Q_OBJECT

 public:
    MarbleWidgetPopupInfo( MarbleWidget*, const MarbleModel* );

    ~MarbleWidgetPopupInfo() override;

public Q_SLOTS:
    void  showInfo( int, int );
    void  clear();

private:
    void slotInfoDialog(const GeoDataFeature* feature, const QPoint &curpos);

    Q_DISABLE_COPY( MarbleWidgetPopupInfo )
    class Private;
    Private* const d;
};

}

#endif
