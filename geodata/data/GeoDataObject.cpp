//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
// Copyright 2008      Jens-Michael Hoffmann <jensmh@gmx.de>
//

#include "geodata/data/GeoDataObject.h"

#include <QtGlobal>
#include <QDataStream>
#include <QFileInfo>
#include <QUrl>

#include "geodata/data/GeoDataDocument.h"

#include "geodata/parser/GeoDataTypes.h"


namespace Marble
{

class GeoDataObjectPrivate
{
  public:
    GeoDataObjectPrivate()
        : m_id(),
          m_targetId(),
          m_parent(nullptr)
    {
    }

    QString  m_id;
    QString  m_targetId;
    GeoDataObject *m_parent;
};

GeoDataObject::GeoDataObject()
    : GeoNode(),
      d( new GeoDataObjectPrivate() )
{
}

GeoDataObject::GeoDataObject( GeoDataObject const & other )
    : GeoNode(),
      d( new GeoDataObjectPrivate( *other.d ) )
{
}

GeoDataObject & GeoDataObject::operator=( const GeoDataObject & rhs )
{
    *d = *rhs.d;
    return *this;
}

GeoDataObject::~GeoDataObject()
{
    delete d;
}

GeoDataObject *GeoDataObject::parent() const
{
    return d->m_parent;
}

void GeoDataObject::setParent(GeoDataObject *parent)
{
    d->m_parent = parent;
}

QString GeoDataObject::id() const
{
    return d->m_id;
}

void GeoDataObject::setId( const QString& value )
{
    d->m_id = value;
}

QString GeoDataObject::targetId() const
{
    return d->m_targetId;
}

void GeoDataObject::setTargetId( const QString& value )
{
    d->m_targetId = value;
}

QString GeoDataObject::resolvePath( const QString &relativePath ) const
{
    QUrl const url( relativePath );
    QFileInfo const fileInfo( url.path() );
    if ( url.isRelative() && fileInfo.isRelative() ) {
        GeoDataDocument const * document = dynamic_cast<GeoDataDocument const*>( this );
        if ( document ) {
            QString const baseUri = document->baseUri();
            QFileInfo const documentRoot = baseUri.isEmpty() ? document->fileName() : baseUri;
            QFileInfo const absoluteImage( documentRoot.absolutePath() + '/' + url.path() );
            return absoluteImage.absoluteFilePath();
        } else if ( d->m_parent ) {
            return d->m_parent->resolvePath( relativePath );
        }
    }

    return relativePath;
}


bool GeoDataObject::equals(const GeoDataObject &other) const
{
    return d->m_id == other.d->m_id && d->m_targetId == other.d->m_targetId;
}

}
