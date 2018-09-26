// Copyright 2010 Jens-Michael Hoffmann <jmho@c-xx.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "BlendingFactory.h"

#include <QDebug>

#include "blendings/SunLightBlending.h"
#include "BlendingAlgorithms.h"

namespace Marble
{

void BlendingFactory::setLevelZeroLayout( int levelZeroColumns, int levelZeroRows )
{
    m_sunLightBlending->setLevelZeroLayout( levelZeroColumns, levelZeroRows );
}

Blending const * BlendingFactory::findBlending( QString const & name ) const
{
    if ( name.isEmpty() )
        return nullptr;

    Blending const * const result = m_blendings.value( name, nullptr );
    if ( !result )
        qWarning() << "Unknown blending" << name << "specified in .dgml file. Please fix the .dgml file.";
    return result;
}

BlendingFactory::BlendingFactory( const SunLocator *sunLocator )
    : m_sunLightBlending( new SunLightBlending( sunLocator ) )
{
    m_blendings.insert( QStringLiteral("OverpaintBlending"), new OverpaintBlending );

    // Neutral blendings
    m_blendings.insert( QStringLiteral("AllanonBlending"), new AllanonBlending );
    m_blendings.insert( QStringLiteral("ArcusTangentBlending"), new ArcusTangentBlending );
    m_blendings.insert( QStringLiteral("GeometricMeanBlending"), new GeometricMeanBlending );
    m_blendings.insert( QStringLiteral("LinearLightBlending"), new LinearLightBlending );
    //m_blendings.insert( "NoiseBlending", new NoiseBlending );
    m_blendings.insert( QStringLiteral("OverlayBlending"), new OverlayBlending );
    //m_blendings.insert( "ParallelBlending", new ParallelBlending );
    //m_blendings.insert( "TextureBlending", new TextureBlending );

    // Darkening blendings
    m_blendings.insert( QStringLiteral("AlphaBlending"), new OverpaintBlending ); // for backwards compatibility
    m_blendings.insert( QStringLiteral("ColorBurnBlending"), new ColorBurnBlending );
    m_blendings.insert( QStringLiteral("DarkBlending"), new DarkBlending );
    m_blendings.insert( QStringLiteral("DarkenBlending"), new DarkenBlending );
    m_blendings.insert( QStringLiteral("DivideBlending"), new DivideBlending );
    m_blendings.insert( QStringLiteral("GammaDarkBlending"), new GammaDarkBlending );
    m_blendings.insert( QStringLiteral("LinearBurnBlending"), new LinearBurnBlending );
    m_blendings.insert( QStringLiteral("MultiplyBlending"), new MultiplyBlending );
    m_blendings.insert( QStringLiteral("SubtractiveBlending"), new SubtractiveBlending );

    // Lightening blendings
    m_blendings.insert( QStringLiteral("AdditiveBlending"), new AdditiveBlending );
    m_blendings.insert( QStringLiteral("ColorDodgeBlending"), new ColorDodgeBlending );
    m_blendings.insert( QStringLiteral("GammaLightBlending"), new GammaLightBlending );
    m_blendings.insert( QStringLiteral("HardLightBlending"), new HardLightBlending );
    m_blendings.insert( QStringLiteral("LightBlending"), new LightBlending );
    m_blendings.insert( QStringLiteral("LightenBlending"), new LightenBlending );
    m_blendings.insert( QStringLiteral("PinLightBlending"), new PinLightBlending );
    m_blendings.insert( QStringLiteral("ScreenBlending"), new ScreenBlending );
    m_blendings.insert( QStringLiteral("SoftLightBlending"), new SoftLightBlending );
    m_blendings.insert( QStringLiteral("VividLightBlending"), new VividLightBlending );

    // Inverter blendings
    //m_blendings.insert( "AdditiveSubtractiveBlending", new AdditiveSubtractiveBlending );
    m_blendings.insert( QStringLiteral("BleachBlending"), new BleachBlending );
    m_blendings.insert( QStringLiteral("DifferenceBlending"), new DifferenceBlending );
    m_blendings.insert( QStringLiteral("EquivalenceBlending"), new EquivalenceBlending );
    m_blendings.insert( QStringLiteral("HalfDifferenceBlending"), new HalfDifferenceBlending );

    // Special purpose blendings
    m_blendings.insert( QStringLiteral("CloudsBlending"), new CloudsBlending );
    m_blendings.insert( QStringLiteral("SunLightBlending"), m_sunLightBlending );
    m_blendings.insert( QStringLiteral("GrayscaleBlending"), new GrayscaleBlending );
}

BlendingFactory::~BlendingFactory()
{
    m_blendings.remove( QStringLiteral("SunLightBlending") );
    delete m_sunLightBlending;
    qDeleteAll( m_blendings );
}

}
