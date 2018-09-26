//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//


#include "MarbleDirs.h"
#include "MarbleDebug.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QApplication>

#include <stdlib.h>

#include <QtCore/QStandardPaths>

#ifdef Q_OS_WIN
//for getting appdata path
//mingw-w64 Internet Explorer 5.01
#define _WIN32_IE 0x0501
#include <shlobj.h>
#endif

#ifdef Q_OS_DARWIN
//for getting app bundle path
#include <ApplicationServices/ApplicationServices.h>
#endif

using namespace Marble;

namespace
{
    QString runTimeMarbleDataPath = QLatin1String("");

    QString runTimeMarblePluginPath = QLatin1String("");
}

MarbleDirs::MarbleDirs()
    : d( nullptr )
{
}


QString MarbleDirs::path( const QString& relativePath )
{ 
    QString  localpath = localPath() + '/' + relativePath;	// local path
    QString  systempath  = systemPath() + '/' + relativePath;	// system path


    QString fullpath = systempath;
    if ( QFile::exists( localpath ) ) {
        fullpath = localpath;
    }
    return QDir( fullpath ).canonicalPath(); 
}


QString MarbleDirs::pluginPath( const QString& relativePath )
{ 
    QString  localpath = pluginLocalPath() + QDir::separator() + relativePath;    // local path
    QString  systempath  = pluginSystemPath() + QDir::separator() + relativePath; // system path


    QString fullpath = systempath;
    if ( QFile::exists( localpath ) ) {
        fullpath = localpath;
    }

    return QDir( fullpath ).canonicalPath(); 
}

QStringList MarbleDirs::entryList( const QString& relativePath, QDir::Filters filters )
{
    QStringList filesLocal = QDir( MarbleDirs::localPath() + '/' + relativePath ).entryList(filters);
    QStringList filesSystem = QDir( MarbleDirs::systemPath() + '/' + relativePath ).entryList(filters);
    QStringList allFiles( filesLocal );
    allFiles << filesSystem;

    // remove duplicate entries
    allFiles.sort();
    for ( int i = 1; i < allFiles.size(); ++i ) {
        if ( allFiles.at(i) == allFiles.at( i - 1 ) ) {
            allFiles.removeAt(i);
            --i;
        }
    }

    return allFiles;
}

QStringList MarbleDirs::pluginEntryList( const QString& relativePath, QDir::Filters filters )
{
    QStringList filesLocal = QDir( MarbleDirs::pluginLocalPath() + '/' + relativePath ).entryList(filters);
    QStringList filesSystem = QDir( MarbleDirs::pluginSystemPath() + '/' + relativePath ).entryList(filters);
    QStringList allFiles( filesLocal );
    allFiles << filesSystem;

    // remove duplicate entries
    allFiles.sort();
    for ( int i = 1; i < allFiles.size(); ++i ) {
        if ( allFiles.at(i) == allFiles.at( i - 1 ) ) {
            allFiles.removeAt(i);
            --i;
        }
    }

    return allFiles;
}

QString MarbleDirs::systemPath()
{

#ifdef Q_OS_DARWIN
    QString systempath;
    //
    // On OSX lets try to find any file first in the bundle
    // before branching out to home and sys dirs
    //
    CFURLRef myBundleRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef myMacPath = CFURLCopyFileSystemPath(myBundleRef, kCFURLPOSIXPathStyle);
    const char *mypPathPtr = CFStringGetCStringPtr(myMacPath,CFStringGetSystemEncoding());
    CFRelease(myBundleRef);
    QString myPath(mypPathPtr);
    CFRelease(myMacPath);
    //do some magick so that we can still find data dir if
    //marble was not built as a bundle
    if (myPath.contains(".app"))  //its a bundle!
    {
      systempath = myPath + "/Contents/Resources/marble/data";
    }

    qDebug() << "myPath" << myPath;

    if ( QFile::exists( systempath ) )
    {
        qDebug() << "systempath" << systempath;
        return systempath;
    }
#endif   // mac bundle

#ifdef Q_OS_ANDROID
    systempath = "assets:/data";
    return systempath;
#endif

// Should this happen before the Mac bundle already?
    if ( !runTimeMarbleDataPath.isEmpty() )
        return runTimeMarbleDataPath;

    return QCoreApplication::applicationDirPath() + QLatin1String( "/marble/data" );
}

QString MarbleDirs::pluginSystemPath()
{
#ifdef Q_OS_DARWIN
    //
    // On OSX lets try to find any file first in the bundle
    // before branching out to home and sys dirs
    //
    CFURLRef myBundleRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef myMacPath = CFURLCopyFileSystemPath(myBundleRef, kCFURLPOSIXPathStyle);
    const char *mypPathPtr = CFStringGetCStringPtr(myMacPath,CFStringGetSystemEncoding());
    CFRelease(myBundleRef);
    CFRelease(myMacPath);
    QString myPath(mypPathPtr);
    //do some magick so that we can still find data dir if
    //marble was not built as a bundle
    QString systempath;
    if (myPath.contains(".app"))  //its a bundle!
    {
      systempath = myPath + "/Contents/Resources/plugins";
    }

    if ( QFile::exists( systempath ) )
    {
      return systempath;
    }
#endif   // mac bundle

    // Should this happen before the Mac bundle already?
    if ( !runTimeMarblePluginPath.isEmpty() )
        return runTimeMarblePluginPath;

    return QCoreApplication::applicationDirPath() + "/marble/plugins";
}

QString MarbleDirs::localPath() 
{
#ifndef Q_OS_WIN
    return QDir::homePath() + "/.marblelib";
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/.marblelib";
#endif
}

QString MarbleDirs::pluginLocalPath() 
{
    return QCoreApplication::applicationDirPath() + "/marble";
}

QString MarbleDirs::marbleDataPath()
{
    return runTimeMarbleDataPath;
}

QString MarbleDirs::marblePluginPath()
{
    return runTimeMarblePluginPath;
}

void MarbleDirs::setMarbleDataPath( const QString& adaptedPath )
{
    if ( !QDir::root().exists( adaptedPath ) )
    {
        qWarning() << QStringLiteral( "Invalid MarbleDataPath \"%1\". Using \"%2\" instead." ).arg( adaptedPath ).arg( systemPath() );
        return;
    }

    runTimeMarbleDataPath = adaptedPath;
}

void MarbleDirs::setMarblePluginPath( const QString& adaptedPath )
{
    if ( !QDir::root().exists( adaptedPath ) )
    {
        qWarning() << QStringLiteral( "Invalid MarblePluginPath \"%1\". Using \"%2\" instead." ).arg( adaptedPath ).arg( pluginSystemPath() );
        return;
    }

    runTimeMarblePluginPath = adaptedPath;
}


void MarbleDirs::debug()
{
    mDebug() << "=== MarbleDirs: ===";
    mDebug() << "Local Path:" << localPath();
    mDebug() << "Plugin Local Path:" << pluginLocalPath();
    mDebug() << "";
    mDebug() << "Marble Data Path (Run Time) :" << runTimeMarbleDataPath; 
    mDebug() << "";
    mDebug() << "Marble Plugin Path (Run Time) :" << runTimeMarblePluginPath; 
    mDebug() << "";
    mDebug() << "System Path:" << systemPath();
    mDebug() << "Plugin System Path:" << pluginSystemPath();
    mDebug() << "===================";
}
