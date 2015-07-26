/************************************************************************
*    FILE NAME:       collisionresfunc2d.h
*
*    DESCRIPTION:     Standalone 2D collision functions used for 
*					  collision resolution.
************************************************************************/  

#ifndef __collision_res_func_2d_h__
#define __collision_res_func_2d_h__

// Game lib dependencies
#include <common/worldpoint.h>
#include <common/collisionmanifold.h>

// Forward declarations
class CSpriteGroup2D;

namespace NCollisionResFunc2D
{
    // Get the collision data between two sprites
    CCollisionManifold GetCollisionManifold( CSpriteGroup2D * pSpriteA, CSpriteGroup2D * pSpriteB );

    // Resolve the collision between two sprites
    bool ResolveCollision( CCollisionManifold & colMan, CSpriteGroup2D * pSpriteA, CSpriteGroup2D * pSpriteB );

    // Clip the passed in edge against the passed in side 
    int Clip( CPoint & sidePlaneNormal, CWorldValue & side, CWorldPoint * pIncVert );

    // Apply an impulse from a specific point
    void ApplyPointImpulse( CWorldPoint & point, float radius, float inverseRadius, float force, 
                            CSpriteGroup2D * pSprite, bool diminishingForce = true );
}

#endif  // __collision_res_func_2d_h__

