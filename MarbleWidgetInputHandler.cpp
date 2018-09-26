//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2014      Adam Dabrowski <adamdbrw@gmail.com>
//

#include "MarbleWidgetInputHandler.h"

#include <QRubberBand>
#include <QToolTip>
#include <QTimer>

#include "MarbleGlobal.h"
#include "MarbleDebug.h"
#include "MarbleWidget.h"
#include "AbstractDataPluginItem.h"
#include "MarbleWidgetPopupMenu.h"
#include "layers/PopupLayer.h"
#include "RenderPlugin.h"

namespace Marble
{

class MarbleWidgetInputHandlerPrivate
{
    class MarbleWidgetSelectionRubber : public AbstractSelectionRubber
    {
        public:
            MarbleWidgetSelectionRubber(MarbleWidget *widget)
                : m_rubberBand(QRubberBand::Rectangle, widget)
            {
                m_rubberBand.hide();
            }

            void show() override { m_rubberBand.show(); }
            void hide() override { m_rubberBand.hide(); }
            bool isVisible() const override { return m_rubberBand.isVisible(); }
            const QRect &geometry() const override { return m_rubberBand.geometry(); }
            void setGeometry(const QRect &geometry) override { m_rubberBand.setGeometry(geometry); }

        private:
            QRubberBand m_rubberBand;
    };

    public:
        MarbleWidgetInputHandlerPrivate(MarbleWidgetInputHandler *handler, MarbleWidget *widget)
            : m_inputHandler(handler)
            ,m_marbleWidget(widget)
            ,m_selectionRubber(widget)
        {         
            foreach(RenderPlugin *renderPlugin, widget->renderPlugins())
            {
                if(renderPlugin->isInitialized())
                {
                    installPluginEventFilter(renderPlugin);
                }
            }
            m_marbleWidget->grabGesture(Qt::PinchGesture);
        }

        void setCursor(const QCursor &cursor)
        {
            m_marbleWidget->setCursor(cursor);
        }

        void installPluginEventFilter(RenderPlugin *renderPlugin)
        {
            m_marbleWidget->installEventFilter(renderPlugin);
        }

        MarbleWidgetInputHandler *m_inputHandler;
        MarbleWidget *m_marbleWidget;
        MarbleWidgetSelectionRubber m_selectionRubber;
};


void MarbleWidgetInputHandler::setCursor(const QCursor &cursor)
{
    d->setCursor(cursor);
}

AbstractSelectionRubber *MarbleWidgetInputHandler::selectionRubber()
{
    return &d->m_selectionRubber;
}

void MarbleWidgetInputHandler::installPluginEventFilter(RenderPlugin *renderPlugin)
{
    d->installPluginEventFilter(renderPlugin);
}

MarbleWidgetInputHandler::MarbleWidgetInputHandler(MarbleAbstractPresenter *marblePresenter, MarbleWidget *widget)
    : MarbleDefaultInputHandler(marblePresenter)
    ,d(new MarbleWidgetInputHandlerPrivate(this, widget))
{
}

void MarbleWidgetInputHandler::restoreViewContext()
{
    MarbleInputHandler::restoreViewContext();

    setCursor(QCursor(Qt::OpenHandCursor));
}

//FIXME - these should be moved to superclass and popupMenu should be abstracted in MarbleAbstractPresenter
void MarbleWidgetInputHandler::showInfo(int x, int y)
{
    d->m_marbleWidget->popupMenu()->showInfo(x, y);
}

void MarbleWidgetInputHandler::clearPopup()
{
    d->m_marbleWidget->popupMenu()->clear();
}

void MarbleWidgetInputHandler::openItemToolTip()
{
    if (!lastToolTipItem().isNull())
    {
        QToolTip::showText(d->m_marbleWidget->mapToGlobal(toolTipPosition()),
                            lastToolTipItem()->toolTip(),
                            d->m_marbleWidget,
                            lastToolTipItem()->containsRect(toolTipPosition()).toRect());
    }
}

}

//#include "moc_MarbleWidgetInputHandler.cpp"
