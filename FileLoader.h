//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//
#ifndef MARBLE_FILELOADER_H
#define MARBLE_FILELOADER_H

#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataStyle.h"
#include "ParsingRunnerManager.h"

#include <QThread>
#include <QString>

namespace Marble
{
class GeoDataContainer;
class PluginManager;
class FileLoader;

class FileLoaderPrivate
{
public:
    FileLoaderPrivate( FileLoader* parent, const PluginManager *pluginManager, bool recenter,
                       QString  file, QString  property, GeoDataStyle::Ptr style, DocumentRole role );

    FileLoaderPrivate( FileLoader* parent, const PluginManager *pluginManager,
                       QString  contents, QString  file, DocumentRole role );

    ~FileLoaderPrivate();

    void createFilterProperties( GeoDataContainer *container );
    static int cityPopIdx( qint64 population );
    static int spacePopIdx( qint64 population );
    static int areaPopIdx( qreal area );

    void documentParsed( GeoDataDocument *doc, const QString& error);

    FileLoader *q;
    ParsingRunnerManager m_runner;
    bool m_recenter;
    QString m_filepath;
    QString m_contents;
    QString m_property;
    GeoDataStyle::Ptr m_style;
    DocumentRole m_documentRole;
    GeoDataStyleMap* m_styleMap;
    GeoDataDocument *m_document;
    QString m_error;
};

class FileLoader : public QThread
{
    Q_OBJECT
    public:
        FileLoader(QObject* parent, const PluginManager *pluginManager, bool recenter,
                    const QString& file, const QString& property, const GeoDataStyle::Ptr &style, DocumentRole role );
        FileLoader( QObject* parent, const PluginManager *pluginManager,
                    const QString& contents, const QString& name, DocumentRole role );
        ~FileLoader() override;

        void run() override;
        bool recenter() const;
        QString path() const;
        GeoDataDocument *document();
        QString error() const;

    Q_SIGNALS:
        void loaderFinished( FileLoader* );
        void newGeoDataDocumentAdded( GeoDataDocument* );

private:
        Q_PRIVATE_SLOT ( d, void documentParsed( GeoDataDocument *, QString) )

        friend class FileLoaderPrivate;

        FileLoaderPrivate *d;

};

} // namespace Marble

#endif
