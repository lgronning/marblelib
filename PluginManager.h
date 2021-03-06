//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2008 Torsten Rahn <tackat@kde.org>
// Copyright 2009      Jens-Michael Hoffmann <jensmh@gmx.de>
//

#ifndef MARBLE_PLUGINMANAGER_H
#define MARBLE_PLUGINMANAGER_H

#include <QObject>
#include <QList>
#include "marble_export.h"

namespace Marble
{

class RenderPlugin;
class PositionProviderPlugin;
class AbstractFloatItem;
class PluginManagerPrivate;
class ParseRunnerPlugin;

/**
 * @short The class that handles Marble's plugins.
 *
 * Ownership policy for plugins:
 *
 * On every invocation of createNetworkPlugins and
 * createFloatItems the PluginManager creates new objects and transfers
 * ownership to the calling site. In order to create
 * the objects, the PluginManager internally has a list of the plugins
 * which are owned by the PluginManager and destroyed by it.
 *
 */

class MARBLE_EXPORT PluginManager : public QObject
{
    Q_OBJECT

 public:
    explicit PluginManager( QObject* parent = 0 );

    ~PluginManager() override;

    /**
     * @brief Returns all available RenderPlugins.
     *
     * Ownership of the items remains in PluginManager.
     * In order to use the RenderPlugins, first create new instances using
     * RenderPlugin::newInstance().
     */
    QList<const RenderPlugin *> renderPlugins() const;

    /**
     * @brief Add a RenderPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addRenderPlugin( const RenderPlugin *plugin );

    /**
     * @brief Returns all available PositionProviderPlugins.
     *
     * Ownership of the items remains in PluginManager.
     * In order to use the PositionProviderPlugins, first create new instances using
     * PositionProviderPlugin::newInstance().
     */
    QList<const PositionProviderPlugin *> positionProviderPlugins() const;

    /**
     * @brief Add a PositionProviderPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addPositionProviderPlugin( const PositionProviderPlugin *plugin );

    /**
     * Returns all parse runner plugins.
     * @note: The runner plugins are owned by the PluginManager, do not delete them.
     */
    QList<const ParseRunnerPlugin *> parsingRunnerPlugins() const;

    /**
     * @brief Add a ParseRunnerPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addParseRunnerPlugin( const ParseRunnerPlugin *plugin );

Q_SIGNALS:
    void renderPluginsChanged();

    void positionProviderPluginsChanged();

    void parseRunnerPluginsChanged();

 private:
    Q_DISABLE_COPY( PluginManager )

#ifdef Q_OS_ANDROID
    void installPluginsFromAssets() const;
#endif

    PluginManagerPrivate  * const d;
};

}

#endif
