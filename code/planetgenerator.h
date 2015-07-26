
/************************************************************************
*    FILE NAME:       planetgenerator.h
*
*    DESCRIPTION:     Generator class to generate the planets in the
*                     background.
************************************************************************/

#ifndef __planet_generator_h__
#define __planet_generator_h__

// Physical dependencies
#include "generator.h"

// Game lib dependencies
#include <common/pointint.h>
#include <common/defs.h>
#include <3d/worldcamera.h>

class CPlanetGenerator : public CGenerator
{
public:

    CPlanetGenerator();
    virtual ~CPlanetGenerator();

    // Initialize the world generator
    virtual void Init( CPointInt & focus, uint wSeed );

    // Handle the world generation for space
    virtual void HandleGeneration( const CPointInt & focus, const CPointInt & newFocus, bool forceGenerate = false );

    // Clear the generator
    void Clear();

private:

    // Generate the background
    void Generate( const CPointInt & pos, const CPointInt & newFocus, bool forceGenerate = false );

    // Generate each individual part of the background
    void GeneratePlanet( std::vector<CSector2D *>::iterator & sectorIter );

    // Find the sectors out of our focus range and moves them to the unused sector vector
    virtual void OrganizeSectors( const CPointInt & point );

    // Get the position of the color sector
    CPointInt GetColorSectorPos( std::vector<CSector2D *>::iterator & sectorIter );

private:

    // The center of the background
    CPointInt center;

    // Generator just for the color
    BaseRandGenType colorGenerator;

    // Boost random number generators
    RandIntGen GetRandPlanetPos;
    RandIntGen GetRandPlanetDepth;
    RandFloatGen GetRandPlanetScale;
    RandIntGen GetRandPlanetCount;
    RandFloatGen GetRandPlanetColor;
    RandIntGen GetRandHueShift;

};

#endif  // __planet_generator_h__
