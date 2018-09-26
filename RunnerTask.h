//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <nienhueser@kde.org>
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>

#ifndef MARBLE_RUNNERTASK_H
#define MARBLE_RUNNERTASK_H

#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataLatLonBox.h"

#include <QRunnable>
#include <QString>


namespace Marble
{

class ParsingRunner;
class MarbleModel;
class ParsingRunnerManager;



/** A RunnerTask that executes a file Parsing */
class ParsingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ParsingTask( ParsingRunner *runner, ParsingRunnerManager *manager, QString  fileName, DocumentRole role );

    /**
     * @reimp
     */
    void run() override;

Q_SIGNALS:
    void parsed(GeoDataDocument* document, const QString &error);
    void finished();

private:
    ParsingRunner *const m_runner;
    QString m_fileName;
    DocumentRole m_role;
    ParsingRunnerManager* m_manager;
};

}

#endif
