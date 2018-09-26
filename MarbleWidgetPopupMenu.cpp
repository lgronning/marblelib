#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn      <tackat@kde.org>
// Copyright 2007      Inge Wallin       <ingwa@kde.org>
// Copyright 2012      Illya Kovalevskyy <illya.kovalevskyy@gmail.com>
// Copyright 2014      Gábor Péterffy    <peterffy95@gmail.com>
//

// Self
#include "MarbleWidgetPopupMenu.h"

// Marble
#include "AbstractDataPluginItem.h"
#include "AbstractFloatItem.h"
#include "MarbleDirs.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "geodata/data/GeoDataExtendedData.h"
#include "geodata/data/GeoDataFolder.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataSnippet.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataBalloonStyle.h"

#include "geodata/parser/GeoDataTypes.h"
#include "geodata/scene/GeoSceneDocument.h"
#include "geodata/scene/GeoSceneHead.h"
#include "MarbleClock.h"
#include "MarbleDebug.h"
#include "layers/PopupLayer.h"
#include "Planet.h"
#include "TemplateDocument.h"

#include "service/XmlHelper.h"

// Qt
#include <QApplication>
#include <QFile>
#include <QMimeData>
#include <QPointer>
#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>
#include <QImage>

namespace Marble {
/* TRANSLATOR Marble::MarbleWidgetPopupMenu */

class Q_DECL_HIDDEN MarbleWidgetPopupInfo::Private {
public:
    const MarbleModel    *const m_model;
    MarbleWidget   *const m_widget;

    QList<AbstractDataPluginItem *> m_itemList;

    QAction *m_infoDialogAction;

public:
    Private( MarbleWidget *widget, const MarbleModel *model, MarbleWidgetPopupInfo* parent );
    /**
      * Returns the geo coordinates of the mouse pointer at the last right button menu.
      * You must not pass 0 as coordinates parameter. The result indicates whether the
      * coordinates are valid, which will be true if the right button menu was opened at least once.
      */
    static QString filterEmptyShortDescription( const QString &description );
    void setupDialogSatellite( const GeoDataPlacemark *placemark );
    static void setupDialogCity( PopupLayer *popup, const GeoDataPlacemark *placemark );
    static void setupDialogNation( PopupLayer *popup, const GeoDataPlacemark *placemark );
    static void setupDialogGeoPlaces( PopupLayer *popup, const GeoDataPlacemark *placemark );
    static void setupDialogSkyPlaces( PopupLayer *popup, const GeoDataPlacemark *placemark );
    static void setupDialogPlotPlaces( PopupLayer *popup, const GeoDataPlacemark *placemark );
    static void setupDialogPhotoOverlay( PopupLayer *popup, const GeoDataPoint *overlay);
};

MarbleWidgetPopupInfo::Private::Private( MarbleWidget *widget, const MarbleModel *model, MarbleWidgetPopupInfo* parent ) :
    m_model(model),
    m_widget(widget)
{
}

QString MarbleWidgetPopupInfo::Private::filterEmptyShortDescription(const QString &description)
{
    if(description.isEmpty())
        return tr("No description available.");
    return description;
}

void MarbleWidgetPopupInfo::Private::setupDialogSatellite( const GeoDataPlacemark *placemark )
{
    PopupLayer *const popup = m_widget->popupLayer();
    const GeoDataCoordinates location = placemark->coordinate(m_widget->model()->clockDateTime());
    const QString description = placemark->description();
    TemplateDocument doc(description);
    doc[QStringLiteral("altitude")] = QString::number(location.altitude(), 'f', 2);
    doc[QStringLiteral("latitude")] = location.latToString();
    doc[QStringLiteral("longitude")] = location.lonToString();
    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupInfo::Private::setupDialogCity( PopupLayer *popup, const GeoDataPlacemark *placemark )
{
    const GeoDataCoordinates location = placemark->coordinate();
    QFile descriptionFile(QStringLiteral(":/marble/webpopup/city.html"));
    if (!descriptionFile.open(QIODevice::ReadOnly)) {
        return;
    }

    const QString description = descriptionFile.readAll();
    TemplateDocument doc(description);

    doc[QStringLiteral("name")] = placemark->name();
    QString  roleString;
    const QString role = placemark->role();
    if(role==QLatin1String("PPLC")) {
        roleString = tr("National Capital");
    } else if(role==QLatin1String("PPL")) {
        roleString = tr("City");
    } else if(role==QLatin1String("PPLA")) {
        roleString = tr("State Capital");
    } else if(role==QLatin1String("PPLA2")) {
        roleString = tr("County Capital");
    } else if(role==QLatin1String("PPLA3") || role==QLatin1String("PPLA4") ) {
        roleString = tr("Capital");
    } else if(role==QLatin1String("PPLF") || role==QLatin1String("PPLG") || role==QLatin1String("PPLL") || role==QLatin1String("PPLQ") ||
              role==QLatin1String("PPLR") || role==QLatin1String("PPLS") || role==QLatin1String("PPLW") ) {
        roleString = tr("Village");
    }

    doc[QStringLiteral("category")] = roleString;
    doc[QStringLiteral("shortDescription")] = filterEmptyShortDescription(placemark->description());
    doc[QStringLiteral("latitude")] = location.latToString();
    doc[QStringLiteral("longitude")] = location.lonToString();
    doc[QStringLiteral("elevation")] =  QString::number(location.altitude(), 'f', 2);
    doc[QStringLiteral("population")] = QString::number(placemark->population());
    doc[QStringLiteral("country")] = placemark->countryCode();
    doc[QStringLiteral("state")] = placemark->state();

    QString dst = QStringLiteral( "%1" ).arg( ( placemark->extendedData().value(QStringLiteral("gmt")).value().toInt() +
                                         placemark->extendedData().value(QStringLiteral("dst")).value().toInt() ) /
                                       ( double ) 100, 0, 'f', 1 );
    // There is an issue about UTC.
    // It's possible to variants (e.g.):
    // +1.0 and -1.0, but dst does not have + an the start
    if(dst.startsWith('-')) {
        doc[QStringLiteral("timezone")] = dst;
    } else {
        doc[QStringLiteral("timezone")] = '+'+dst;
    }

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupInfo::Private::setupDialogNation( PopupLayer *popup, const GeoDataPlacemark *index)
{
    const GeoDataCoordinates location = index->coordinate();

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/nation.html"));
    if (!descriptionFile.open(QIODevice::ReadOnly)) {
        return;
    }

    const QString description = descriptionFile.readAll();
    TemplateDocument doc(description);

    doc[QStringLiteral("name")] = index->name();
    doc[QStringLiteral("shortDescription")] = filterEmptyShortDescription(index->description());
    doc[QStringLiteral("latitude")] = location.latToString();
    doc[QStringLiteral("longitude")] = location.lonToString();
    doc[QStringLiteral("elevation")] = QString::number(location.altitude(), 'f', 2);
    doc[QStringLiteral("population")] = QString::number(index->population());
    doc[QStringLiteral("area")] = QString::number(index->area(), 'f', 2);

    const QString flagPath = MarbleDirs::path(QStringLiteral("flags/flag_%1.svg").arg(index->countryCode().toLower()) );
    doc[QStringLiteral("flag")] = flagPath;

    popup->setContent(doc.finalText());
}

static const QString geoplaceString = QString::fromLatin1("<html>"
                                 "<head>"
                                 "<body>"
                                 "<h1>%name%</h1>"
                                 "<table>"
                                 "<tr>"
                                 "  <td colspan=\"2\">Coordinates</td>"
                                 "</tr>"
                                 "<tr>"
                                 "  <td>Longitude</a></td><td>%longitude%</td>"
                                 "</tr>"
                                 "<tr>"
                                 "  <td>Latitude</a></td><td>%latitude%</td>"
                                 "</tr>"
                                 "</table>"
                                 ""
                                 "<h3>Short description:</h3>"
                                 "<p>"
                                 "%shortDescription%"
                                 "</p>"
                                 "</body>"
                                 "</html>");

void
MarbleWidgetPopupInfo::Private::setupDialogGeoPlaces( PopupLayer *popup, const GeoDataPlacemark *index)
{
    const GeoDataCoordinates location = index->coordinate();


    TemplateDocument doc(geoplaceString);

    qDebug() << "geoplacesString" << geoplaceString;

    doc[QStringLiteral("name")] = index->name();
    doc[QStringLiteral("latitude")] = location.latToString();
    doc[QStringLiteral("longitude")] = location.lonToString();
    doc[QStringLiteral("shortDescription")] = filterEmptyShortDescription(index->description());

//    QFile file("temp.html");
//    file.open(QIODevice::WriteOnly);
//    file.write(doc.finalText().toUtf8());

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupInfo::Private::setupDialogSkyPlaces( PopupLayer *popup, const GeoDataPlacemark *index)
{
    const GeoDataCoordinates location = index->coordinate();

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/skyplace.html"));
    if (!descriptionFile.open(QIODevice::ReadOnly)) {
        return;
    }

    const QString description = descriptionFile.readAll();
    TemplateDocument doc(description);

    doc[QStringLiteral("name")] = index->name();
    doc[QStringLiteral("latitude")] = GeoDataCoordinates::latToString(
                            location.latitude(), GeoDataCoordinates::Astro, GeoDataCoordinates::Radian, -1, 'f');
    doc[QStringLiteral("longitude")] = GeoDataCoordinates::lonToString(
                            location.longitude(), GeoDataCoordinates::Astro, GeoDataCoordinates::Radian, -1, 'f');
    doc[QStringLiteral("shortDescription")] = filterEmptyShortDescription(index->description());

    popup->setContent(doc.finalText());
}


void MarbleWidgetPopupInfo::Private::setupDialogPlotPlaces( PopupLayer *popup, const GeoDataPlacemark *placemark)
{
    popup->setPlacemark(placemark);
}


static const QString photoOverlayDescription = QString("<html>"
                                                       "<head>"
                                                       "<body>"
                                                       "<h1>%name%</h1>"
                                                       "<table>"
                                                       "<tr>"
                                                       "  <td colspan=\"2\">Coordinates</td>"
                                                       "</tr>"
                                                       "<tr>"
                                                       "  <td>Longitude</a></td><td>%longitude%</td>"
                                                       "</tr>"
                                                       "<tr>"
                                                       "  <td>Latitude</a></td><td>%latitude%</td>"
                                                       "</tr>"
                                                       "</table>"
                                                       " "
                                                       "<h3>Short description:</h3>"
                                                       "<p>"
                                                       "%shortDescription%"
                                                       "</p>"
                                                       "</body>"
                                                       "</html>");

void MarbleWidgetPopupInfo::Private::setupDialogPhotoOverlay(PopupLayer *popup, const GeoDataPoint *index )
{
//    const GeoDataCoordinates location = index->coordinates();

//    const QString description = photoOverlayDescription;
//    TemplateDocument doc(description);
//    doc[QStringLiteral("name")] = index->name();
//    doc[QStringLiteral("latitude")] = location.latToString();
//    doc[QStringLiteral("longitude")] = location.lonToString();
//    doc[QStringLiteral("shortDescription")] = filterEmptyShortDescription(index->description());

//    popup->setContent(doc.finalText(), index->icon().isNull() ? QPixmap() : QPixmap::fromImage(index->icon().scaled(qMin(index->icon().width(), 100), qMin(index->icon().height(), 50), Qt::KeepAspectRatio, Qt::SmoothTransformation)) );
}

MarbleWidgetPopupInfo::MarbleWidgetPopupInfo(MarbleWidget *widget,
                                         const MarbleModel *model)
    : QObject(widget),
      d( new Private( widget, model, this ) )
{
    // nothing to do
}

MarbleWidgetPopupInfo::~MarbleWidgetPopupInfo()
{
    delete d;
}

void MarbleWidgetPopupInfo::showInfo( int xpos, int ypos )
{
    const QPoint curpos = QPoint( xpos, ypos );
    const GeoDataFeature* feature = d->m_widget->whichFeatureAt( curpos );

    if(feature == nullptr)
    {
        d->m_widget->popupLayer()->setVisible(false);
        return;
    }

    slotInfoDialog(feature, curpos);
}

void MarbleWidgetPopupInfo::clear()
{
    d->m_widget->popupLayer()->setVisible(false);
}

void MarbleWidgetPopupInfo::slotInfoDialog(const GeoDataFeature* feature, const QPoint &curpos)
{
    qDebug() << "MarbleWidgetPopupMenu::slotInfoDialog";

    const GeoDataPlacemark *placemark = dynamic_cast<const GeoDataPlacemark*>(feature);
    PopupLayer* popup = d->m_widget->popupLayer();
    bool isSatellite = false;
    bool isCity = false;
    bool isNation = false;

    if ( placemark ) {
        isSatellite = (placemark->visualCategory() == GeoDataFeature::Satellite);
        isCity = (placemark->visualCategory() >= GeoDataFeature::SmallCity &&
                  placemark->visualCategory() <= GeoDataFeature::LargeNationCapital);
        isNation = (placemark->visualCategory() == GeoDataFeature::Nation);
    }

    bool isSky = false;

    if ( d->m_widget->model()->mapTheme() ) {
        isSky = d->m_widget->model()->mapTheme()->head()->target() == QLatin1String("sky");
    }

    qDebug() << "MarbleWidgetPopupMenu::slotInfoDialog type" << isSatellite << isCity << isNation << isSky;
    if (placemark && isSatellite)
    {
        d->setupDialogSatellite( placemark );
    }
    else if (placemark && isCity)
    {
        Private::setupDialogCity( popup, placemark );
    }
    else if (placemark && isNation)
    {
        Private::setupDialogNation( popup, placemark );
    }
    else if (placemark && isSky)
    {
        Private::setupDialogSkyPlaces( popup, placemark );
    }
    else if ( placemark && placemark->extendedData().value("plot").valueRef().toBool() )
    {
        Private::setupDialogPlotPlaces( popup, placemark );
    }
    else if ( placemark && placemark->role().isEmpty() )
    {
        popup->setContent( placemark->description() );
    }
    else if ( placemark )
    {
        Private::setupDialogGeoPlaces( popup, placemark );
    }

    if ( placemark )
    {
        qDebug() << "MarbleWidgetPopupMenu::slotInfoDialog displayMode" << placemark->style()->balloonStyle().displayMode();
        if ( placemark->style() == nullptr )
        {
            return;
        }

        if ( placemark->style()->balloonStyle().displayMode() == GeoDataBalloonStyle::Hide ) {
            popup->setVisible(false);
            return;
        }

        QString content = placemark->style()->balloonStyle().text();
        if (content.length() > 0) {
            content = content.replace(QLatin1String("$[name]"), placemark->name(), Qt::CaseInsensitive);
            content = content.replace(QLatin1String("$[description]"), placemark->description(), Qt::CaseInsensitive);
            content = content.replace(QLatin1String("$[address]"), placemark->address(), Qt::CaseInsensitive);
            // @TODO: implement the line calculation, so that snippet().maxLines actually has effect.
            content = content.replace(QLatin1String("$[snippet]"), placemark->snippet().text(), Qt::CaseInsensitive);
            content = content.replace(QLatin1String("$[id]"), placemark->id(), Qt::CaseInsensitive);
            popup->setContent(content);
        }
    }

    popup->setPosition(curpos);
    popup->popup();
}
}

//#include "moc_MarbleWidgetPopupMenu.cpp"
