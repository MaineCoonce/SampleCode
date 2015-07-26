
/************************************************************************
*    FILE NAME:       collisionmanifold.h
*
*    DESCRIPTION:     Class to handle the collision interactions between
*                     two sprites.
************************************************************************/

#ifndef __collision_manifold_h__
#define __collision_manifold_h__

// Game lib dependencies
#include <common/point.h>
#include <common/worldpoint.h>

// Forward declarations
class CSpriteGroup2D;
class CEdge;

class CCollisionManifold
{
public:

    // Constructor
    CCollisionManifold();

    // Push the two sprites away from each other if they're intersecting
    void PositionalCorrection();

    // Get the incident edge
    void FindIncidentEdge();

    // Apply the impulse to the incident and reference sprites
    void ApplyImpulse();

public:

    // The incident and reference sprites who we're checking collision on
    CSpriteGroup2D * pRefSprite;
    CSpriteGroup2D * pIncSprite;

    // The amount the incident sprite has penetrated the reference edge
    float penetration;

    // Normal to the direction of the collision
    CPoint normal;

    // Edges
    CEdge * pRefEdge;
    CEdge * pIncEdge;

    // The points of contact bewteen the two sprites
    CWorldPoint contactPoint[2];
    int contactCount;

};

#endif  // __collision_manifold_h__
