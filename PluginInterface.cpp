//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//
#include "PluginInterface.h"

namespace Marble
{

PluginInterface::~PluginInterface()
= default;

QString PluginInterface::aboutDataText() const
{
    return QString();
}

}
