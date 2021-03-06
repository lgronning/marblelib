/*
    Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "DgmlVectorTagHandler.h"

#include "MarbleDebug.h"

#include "geodata/handlers/dgml/DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "geodata/handlers/dgml/DgmlAuxillaryDictionary.h"
#include "geodata/parser/GeoParser.h"
#include "geodata/scene/GeoSceneLayer.h"
#include "geodata/scene/GeoSceneGeodata.h"

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Vector)

GeoNode* DgmlVectorTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(dgmlTag_Vector));

    QString name      = parser.attribute(dgmlAttr_name).trimmed();
    QString feature   = parser.attribute(dgmlAttr_feature).trimmed();

    GeoSceneGeodata *vector = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    // Check parent type and make sure that the dataSet type 
    // matches the backend of the parent layer
    if ( parentItem.represents(dgmlTag_Layer)
        && parentItem.nodeAs<GeoSceneLayer>()->backend() == dgmlValue_vector ) {

        vector = new GeoSceneGeodata( name );
        vector->setProperty( feature );
        vector->setColorize( feature );
        parentItem.nodeAs<GeoSceneLayer>()->addDataset( vector );
    }

    return vector;
}

}
}
