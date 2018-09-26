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

#include "geodata/handlers/dgml/DgmlElementDictionary.h"

namespace Marble
{

// One static list of all tag names, to avoid string comparisons all-over-the-place
namespace dgml
{

const char* const dgmlTag_nameSpace20 = "http://edu.kde.org/marble/dgml/2.0";

const char* const dgmlTag_Available = "available";
const char* const dgmlTag_Blending = "blending";
const char* const dgmlTag_Brush = "brush";
const char* const dgmlTag_Color = "color";
const char* const dgmlTag_CustomPlugin = "customplugin";
const char* const dgmlTag_Dem = "dem";
const char* const dgmlTag_Description = "description";
const char* const dgmlTag_Dgml = "dgml";
const char* const dgmlTag_Discrete = "discrete";
const char* const dgmlTag_Document = "document";
const char* const dgmlTag_DownloadPolicy = "downloadPolicy";
const char* const dgmlTag_DownloadUrl = "downloadUrl";
const char* const dgmlTag_Filter = "filter";
const char* const dgmlTag_Geodata = "geodata";
const char* const dgmlTag_Group = "group";
const char* const dgmlTag_Head = "head";
const char* const dgmlTag_Heading = "heading";
const char* const dgmlTag_Icon = "icon";
const char* const dgmlTag_InstallMap = "installmap";
const char* const dgmlTag_Item = "item";
const char* const dgmlTag_Layer = "layer";
const char* const dgmlTag_Legend = "legend";
const char* const dgmlTag_License = "license";
const char* const dgmlTag_Map = "map";
const char* const dgmlTag_Maximum = "maximum";
const char* const dgmlTag_Minimum = "minimum";
const char* const dgmlTag_Name = "name";
const char* const dgmlTag_Palette = "palette";
const char* const dgmlTag_Pen = "pen";
const char* const dgmlTag_Projection = "projection";
const char* const dgmlTag_Property = "property";
const char* const dgmlTag_Section = "section";
const char* const dgmlTag_Settings = "settings";
const char* const dgmlTag_SourceDir = "sourcedir";
const char* const dgmlTag_SourceFile = "sourcefile";
const char* const dgmlTag_StorageLayout = "storageLayout";
const char* const dgmlTag_Target = "target";
const char* const dgmlTag_Text = "text";
const char* const dgmlTag_Texture = "texture";
const char* const dgmlTag_Theme = "theme";
const char* const dgmlTag_TileSize = "tileSize";
const char* const dgmlTag_Value = "value";
const char* const dgmlTag_Vector = "vector";
const char* const dgmlTag_Vectortile = "vectortile";
const char* const dgmlTag_Visible = "visible";
const char* const dgmlTag_Zoom = "zoom";
}
}
