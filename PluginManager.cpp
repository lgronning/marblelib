//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
//


// Own
#include "PluginManager.h"

// Qt
#include <QList>
#include <QPluginLoader>
#include <QTime>

// Local dir
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "RenderPlugin.h"
#include "PositionProviderPlugin.h"
#include "AbstractFloatItem.h"
#include "ParseRunnerPlugin.h"

namespace Marble
{

class PluginManagerPrivate
{
 public:
    PluginManagerPrivate()
            : m_pluginsLoaded(false)
    {
    }

    ~PluginManagerPrivate();

    void loadPlugins();

    bool m_pluginsLoaded;
    QList<const RenderPlugin *> m_renderPluginTemplates;
    QList<const PositionProviderPlugin *> m_positionProviderPluginTemplates;
    QList<const ParseRunnerPlugin *> m_parsingRunnerPlugins;

#ifdef Q_OS_ANDROID
    QStringList m_pluginPaths;
#endif
};

PluginManagerPrivate::~PluginManagerPrivate()
{
    // nothing to do
}

PluginManager::PluginManager( QObject *parent ) : QObject( parent ),
    d( new PluginManagerPrivate() )
{
    //Checking assets:/plugins for uninstalled plugins
#ifdef Q_OS_ANDROID
        installPluginsFromAssets();
#endif
}

PluginManager::~PluginManager()
{
    delete d;
}

QList<const RenderPlugin *> PluginManager::renderPlugins() const
{
    d->loadPlugins();
    return d->m_renderPluginTemplates;
}

void PluginManager::addRenderPlugin( const RenderPlugin *plugin )
{
    d->loadPlugins();
    d->m_renderPluginTemplates << plugin;
    emit renderPluginsChanged();
}

QList<const PositionProviderPlugin *> PluginManager::positionProviderPlugins() const
{
    d->loadPlugins();
    return d->m_positionProviderPluginTemplates;
}

void PluginManager::addPositionProviderPlugin( const PositionProviderPlugin *plugin )
{
    d->loadPlugins();
    d->m_positionProviderPluginTemplates << plugin;
    emit positionProviderPluginsChanged();
}


QList<const ParseRunnerPlugin *> PluginManager::parsingRunnerPlugins() const
{
    d->loadPlugins();
    return d->m_parsingRunnerPlugins;
}

void PluginManager::addParseRunnerPlugin( const ParseRunnerPlugin *plugin )
{
    d->loadPlugins();
    d->m_parsingRunnerPlugins << plugin;
    emit parseRunnerPluginsChanged();
}

/** Append obj to the given plugins list if it inherits both T and U */
template<class T, class U>
bool appendPlugin( QObject * obj, QList<T*> &plugins )
{
    if ( qobject_cast<T*>( obj ) && qobject_cast<U*>( obj ) ) {
        Q_ASSERT( obj->metaObject()->superClass() ); // all our plugins have a super class
        T* plugin = qobject_cast<T*>( obj );
        Q_ASSERT( plugin ); // checked above
        plugins << plugin;
        return true;
    }

    return false;
}

/** Append obj to the given plugins list if it inherits both T and U */
template<class T, class U>
bool appendPlugin( QObject * obj, QList<const T*> &plugins )
{
    if ( qobject_cast<T*>( obj ) && qobject_cast<U*>( obj ) ) {
        Q_ASSERT( obj->metaObject()->superClass() ); // all our plugins have a super class
        T* plugin = qobject_cast<T*>( obj );
        Q_ASSERT( plugin ); // checked above
        plugins << plugin;
        return true;
    }

    return false;
}

void PluginManagerPrivate::loadPlugins()
{
    if (m_pluginsLoaded)
    {
        return;
    }

    QTime t;
    t.start();
    mDebug() << "Starting to load Plugins.";

    Q_ASSERT( m_renderPluginTemplates.isEmpty() );
    Q_ASSERT( m_positionProviderPluginTemplates.isEmpty() );
    Q_ASSERT( m_parsingRunnerPlugins.isEmpty() );

    foreach(QObject *obj, QPluginLoader::staticInstances ())
    {

        bool isPlugin = appendPlugin<RenderPlugin, RenderPluginInterface>
                   ( obj, m_renderPluginTemplates );
        isPlugin = isPlugin || appendPlugin<PositionProviderPlugin, PositionProviderPluginInterface>
                   ( obj, m_positionProviderPluginTemplates );
        isPlugin = isPlugin || appendPlugin<ParseRunnerPlugin, ParseRunnerPlugin>
                   ( obj, m_parsingRunnerPlugins ); // intentionally T==U

        qDebug() << obj->objectName();
    }

    m_pluginsLoaded = true;

    mDebug() << Q_FUNC_INFO << "Time elapsed:" << t.elapsed() << "ms";
}

#ifdef Q_OS_ANDROID
    void PluginManager::installPluginsFromAssets() const
    {
        d->m_pluginPaths.clear();
        QStringList copyList = MarbleDirs::pluginEntryList(QString());
        QDir pluginHome(MarbleDirs::localPath());
        pluginHome.mkpath(MarbleDirs::pluginLocalPath());
        pluginHome.setCurrent(MarbleDirs::pluginLocalPath());
        foreach (const QString & file, copyList) {
            QString const target = MarbleDirs::pluginLocalPath() + '/' + file;
            if (QFileInfo(MarbleDirs::pluginSystemPath() + '/' + file).isDir()) {
                pluginHome.mkpath(target);
            }
            else {
                QFile temporaryFile(MarbleDirs::pluginSystemPath() + '/' + file);
                temporaryFile.copy(target);
                QFileInfo targetFile(target);
                d->m_pluginPaths << targetFile.canonicalFilePath();
            }
        }
    }
#endif

}

//#include "moc_PluginManager.cpp"
