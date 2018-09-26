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

//
// MarblePlacemarkModel exposes the model for Placemarks
//

#ifndef MARBLE_MARBLEPLACEMARKMODEL_H
#define MARBLE_MARBLEPLACEMARKMODEL_H


#include <QAbstractListModel>

namespace Marble
{

class MarblePlacemarkModel
{
    public:
    /**
     * The roles of the place marks.
     */
    enum Roles
    {
      GeoTypeRole = Qt::UserRole + 1,  ///< The geo type (e.g. city or mountain)
      DescriptionRole,                 ///< The description
      CoordinateRole,                  ///< The GeoDataCoordinates coordinate
      PopulationRole,                  ///< The population
      AreaRole,                        ///< The area size
      CountryCodeRole,                 ///< The country code
      StateRole,                       ///< The state
      VisualCategoryRole,              ///< The category
      StyleRole,                       ///< The style
      PopularityIndexRole,             ///< The popularity index
      PopularityRole,                  ///< The popularity
      ObjectPointerRole,               ///< The pointer to a specific object
      GmtRole,                         ///< The Greenwich Mean Time
      DstRole,                         ///< The Daylight Saving Time
      GeometryRole,                    ///< The GeoDataGeometry geometry
      LongitudeRole,                   ///< The longitude in degree (for use in QML)
      LatitudeRole,                    ///< The latitude in degree (for use in QML)
      IconPathRole                     ///< Path to the image, if known
    };
};

}

#endif
