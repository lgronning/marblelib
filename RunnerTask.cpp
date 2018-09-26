//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <nienhueser@kde.org>
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>
// Copyright 2012,2013 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

#include "RunnerTask.h"

#include "MarbleDebug.h"
#include "ParsingRunner.h"
#include "ParsingRunnerManager.h"

#include <QTimer>

namespace Marble
{

ParsingTask::ParsingTask( ParsingRunner *runner, ParsingRunnerManager *manager, QString  fileName, DocumentRole role ) :
    QObject(),
    m_runner( runner ),
    m_fileName(std::move( fileName )),
    m_role( role ),
    m_manager(manager)
{
    connect(this, SIGNAL(parsed(GeoDataDocument*,QString)), m_manager, SLOT(addParsingResult(GeoDataDocument*,QString)));
}

void ParsingTask::run()
{
    QString error;
    GeoDataDocument* document = m_runner->parseFile( m_fileName, m_role, error );
    emit parsed(document, error);
    m_runner->deleteLater();
    emit finished();
}

}

//#include "moc_RunnerTask.cpp"
