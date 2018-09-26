//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATADOCUMENTPRIVATE_H
#define MARBLE_GEODATADOCUMENTPRIVATE_H

#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataStyleMap.h"
#include "geodata/data/GeoDataSchema.h"
#include "geodata/data/GeoDataContainer_p.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataDocumentPrivate : public GeoDataContainerPrivate
{
  public:
    GeoDataDocumentPrivate()
    :  m_documentRole( UnknownDocument )
    {
    }
    
    GeoDataFeaturePrivate* copy() override
    { 
        GeoDataDocumentPrivate* copy = new GeoDataDocumentPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataDocumentType;
    }

    EnumFeatureId featureId() const override
    {
        return GeoDataDocumentId;
    }

    QMap<QString, GeoDataStyle::Ptr> m_styleHash;
    QMap<QString, GeoDataStyleMap> m_styleMapHash;
    QMap<QString, GeoDataSchema> m_schemaHash;
    QString m_filename;
    QString m_baseUri;
    QString m_property;
    DocumentRole m_documentRole;
};

} // namespace Marble

#endif
