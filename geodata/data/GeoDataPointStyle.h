#ifndef GEODATAPOINTSTYLE_H
#define GEODATAPOINTSTYLE_H


#include <QPixmap>

#include "geodata/data/GeoDataColorStyle.h"

#include "geodata/geodata_export.h"

namespace Marble
{

class GeoDataPointStylePrivate;
class RemoteIconLoader;

// Limits for the user in case of scaling the icon too much
static const QSize s_maximumPointSize = QSize( 20, 20 );
static const QSize s_minimumPointSize = QSize( 4, 4);

class GEODATA_EXPORT GeoDataPointStyle : public GeoDataColorStyle
{
  public:
    GeoDataPointStyle();
    GeoDataPointStyle( const GeoDataPointStyle& other );
    explicit GeoDataPointStyle( const QString& iconPath);
    ~GeoDataPointStyle() override;

    GeoDataPointStyle& operator=( const GeoDataPointStyle& other );

    bool operator==( const GeoDataPointStyle &other ) const;
    bool operator!=( const GeoDataPointStyle &other ) const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType() const override;

    void setIcon( const QPixmap& icon );
    QPixmap icon() const;

    /**
     * @brief Returns a scaled version of label icon
     * @return  the scaled icon
     */
    QPixmap scaledIcon() const;

    /**
     * @brief setIconPath Set the path to load the icon from. Any existing icon is invalidated.
     * @param filename Path to the icon to load. Can also be a virtual file system like qrc:/
     */
    void setIconPath( const QString& filename );
    QString iconPath() const;

    void setScale( const double &scale );
    double scale() const;

    RemoteIconLoader *remoteIconLoader() const;

  private:
    GeoDataPointStylePrivate * const d;
};

}


#endif // GEODATAPOINTSTYLE_H
