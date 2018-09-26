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

#include "DgmlPropertyTagHandler.h"

#include "MarbleDebug.h"

#include "geodata/handlers/dgml/DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "geodata/parser/GeoParser.h"
#include "geodata/scene/GeoSceneSettings.h"
#include "geodata/scene/GeoSceneGroup.h"
#include "geodata/scene/GeoSceneProperty.h"

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Property)

GeoNode* DgmlPropertyTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(dgmlTag_Property));

    QString name = parser.attribute(dgmlAttr_name).trimmed();

    GeoSceneProperty* property = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();
    if (parentItem.represents(dgmlTag_Settings)) {
        property = new GeoSceneProperty( name );
        parentItem.nodeAs<GeoSceneSettings>()->addProperty( property );
    }
    if (parentItem.represents(dgmlTag_Group)) {
        property = new GeoSceneProperty( name );
        parentItem.nodeAs<GeoSceneGroup>()->addProperty( property);
    }

    return property;
}

}
}
