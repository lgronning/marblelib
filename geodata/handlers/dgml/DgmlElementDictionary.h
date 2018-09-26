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

#ifndef MARBLE_DGML_ELEMENTDICTIONARY_H
#define MARBLE_DGML_ELEMENTDICTIONARY_H

#include "geodata/geodata_export.h"

namespace Marble
{

// Lists all known DGML 2.0 tags
// http://edu.kde.org/marble/apis/dgml/documentation/dgmltags.html)
namespace dgml
{
    extern  const char* const dgmlTag_nameSpace20;

    extern  const char* const dgmlTag_Available;
    extern  const char* const dgmlTag_Blending;
    extern  const char* const dgmlTag_Brush;
    extern  const char* const dgmlTag_Color;
    extern  const char* const dgmlTag_CustomPlugin;
    extern  const char* const dgmlTag_Dem;
    extern  const char* const dgmlTag_Description;
    extern  const char* const dgmlTag_Discrete;
    extern  const char* const dgmlTag_Dgml;
    extern  const char* const dgmlTag_Document;
    extern  const char* const dgmlTag_DownloadPolicy;
    extern  const char* const dgmlTag_DownloadUrl;
    extern  const char* const dgmlTag_Filter;
    extern  const char* const dgmlTag_Geodata;
    extern  const char* const dgmlTag_Group;
    extern  const char* const dgmlTag_Head;
    extern  const char* const dgmlTag_Heading;
    extern  const char* const dgmlTag_Icon;
    extern  const char* const dgmlTag_InstallMap;
    extern  const char* const dgmlTag_Item;
    extern  const char* const dgmlTag_Layer;
    extern  const char* const dgmlTag_Legend;
    extern  const char* const dgmlTag_License;
    extern  const char* const dgmlTag_Map;
    extern  const char* const dgmlTag_Maximum;
    extern  const char* const dgmlTag_Minimum;
    extern  const char* const dgmlTag_Name;
    extern  const char* const dgmlTag_Palette;
    extern  const char* const dgmlTag_Pen;
    extern  const char* const dgmlTag_Projection;
    extern  const char* const dgmlTag_Property;
    extern  const char* const dgmlTag_Section;
    extern  const char* const dgmlTag_Settings;
    extern  const char* const dgmlTag_SourceDir;
    extern  const char* const dgmlTag_SourceFile;
    extern  const char* const dgmlTag_StorageLayout;
    extern  const char* const dgmlTag_Target;
    extern  const char* const dgmlTag_Text;
    extern  const char* const dgmlTag_Texture;
    extern  const char* const dgmlTag_TileSize;
    extern  const char* const dgmlTag_Theme;
    extern  const char* const dgmlTag_Value;
    extern  const char* const dgmlTag_Vector;
    extern  const char* const dgmlTag_Vectortile;
    extern  const char* const dgmlTag_Visible;
    extern  const char* const dgmlTag_Zoom;
}

// Helper macro
#define DGML_DEFINE_TAG_HANDLER(Name) GEODATA_DEFINE_TAG_HANDLER(dgml, Dgml, Name, dgmlTag_nameSpace20)

}

#endif
