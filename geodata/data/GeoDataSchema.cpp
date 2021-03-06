//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Abhinav Gangwar <abhgang@gmail.com>
//

// Qt
#include <QDataStream>

// Marble
#include "geodata/data/GeoDataSchema.h"
#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataSchemaPrivate
{
  public:
    QHash<QString, GeoDataSimpleField> m_simpleField;
    QString m_name;
};

GeoDataSchema::GeoDataSchema()
    : d( new GeoDataSchemaPrivate )
{
}

GeoDataSchema::GeoDataSchema( const QHash<QString, GeoDataSimpleField>& simplefields )
    : d( new GeoDataSchemaPrivate )
{
    d->m_simpleField = simplefields;
}

GeoDataSchema::GeoDataSchema( const GeoDataSchema& other )
    : GeoDataObject( other ),
      d( new GeoDataSchemaPrivate( *other.d ) )
{
}

GeoDataSchema &GeoDataSchema::operator=(const GeoDataSchema &other)
{
    GeoDataObject::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataSchema::operator==(const GeoDataSchema& other) const
{
    return equals(other) &&
           d->m_name == other.d->m_name &&
           d->m_simpleField == other.d->m_simpleField;
}

bool GeoDataSchema::operator!=(const GeoDataSchema& other) const
{
    return !this->operator==( other );
}

GeoDataSchema::~GeoDataSchema()
{
    delete d;
}

QString GeoDataSchema::schemaName() const
{
    return d->m_name;
}

void GeoDataSchema::setSchemaName( const QString& name )
{
    d->m_name = name;
}

GeoDataSimpleField& GeoDataSchema::simpleField( const QString& name ) const
{
    return d->m_simpleField[ name ];
}

void GeoDataSchema::addSimpleField( const GeoDataSimpleField &value )
{
    d->m_simpleField.insert( value.name(), value );
}

QList<GeoDataSimpleField> GeoDataSchema::simpleFields() const
{
    return d->m_simpleField.values();
}

const char* GeoDataSchema::nodeType() const
{
    return GeoDataTypes::GeoDataSchemaType;
}

}
