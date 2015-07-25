/************************************************************************
*    FILE NAME:       collisionfunc2d.cpp
*
*    DESCRIPTION:     2D collision functions
************************************************************************/  

// Physical component dependency
#include <utilities/collisionresfunc2d.h>

// Game lib dependencies
#include <2d/spritegroup2d.h>
#include <2d/collisionsprite2d.h>
#include <utilities/exceptionhandling.h>
#include <utilities/collisionfunc2d.h>
#include <utilities/mathfunc.h>
#include <common/collisionvertex.h>
#include <common/collisionbody.h>
#include <misc/settings.h>

// Required namespace(s)
//using namespace std;

// Turn off the data type conversion warning (ie. int to float, float to int etc.)
#pragma warning(disable : 4244)

// Ratio of degrees to radians
const float DEGREES_TO_RADIANS = 0.0174532925f;

// Constants used in moving objects from one another when piercing
const float PERCENT = 0.2f; // usually 0.2  to 0.8
const float SLOP = 0.01f;   // usually 0.01 to 0.1

// I'm not 100% sure what these represent
const float BIAS_RELATIVE = 0.95f;
const float BIAS_ABSOLUTE = 0.01f;

// Value used to determine if a point impulse is within a sprite
const float MIN_SEPARATION = 0.0001f;

namespace NCollisionResFunc2D
{
    /************************************************************************
    *    desc:  Get the collision data between two sprites  
    *
    *	 param: CSpriteGroup2D * pSpriteA - sprite to compare
    *			CSpriteGroup2D * pSpriteB - sprite to compare 
    ************************************************************************/
    CCollisionManifold GetCollisionManifold( CSpriteGroup2D * pSpriteA, CSpriteGroup2D * pSpriteB )
    {
        CCollisionManifold colMan;

        // Get the collision sprites
        CCollisionSprite2D * pColSpriteA = pSpriteA->GetCollisionSprite();
        CCollisionSprite2D * pColSpriteB = pSpriteB->GetCollisionSprite();

        // Set the starting penetration to a super negative value
        colMan.penetration = -FLT_MAX;
        
        for( uint i = 0; i < pColSpriteA->GetOuterEdgeCount(); ++i )
        {
            // Retrieve an edge normal from A
            CPoint edgeNormal = pColSpriteA->GetOuterEdge(i)->normal;

            // Retrieve support point from B along the negative edge normal
            CCollisionVertex * pSupportVert = pColSpriteB->GetSupportVert( -edgeNormal );

            // Retrieve one vertex from the edge
            CCollisionVertex * pTmpVert = pColSpriteA->GetOuterEdge(i)->pVert[0];

            // Compute penetration distance (in sprite B's model space)
            float distance = NMathFunc::DotProduct2D( CPoint(pSupportVert->GetPos() - pTmpVert->GetPos()), edgeNormal );

            // Store greatest distance
            if( distance > colMan.penetration )
            {
                colMan.penetration = distance;
                colMan.pRefEdge = pColSpriteA->GetOuterEdge(i);
            }
        }

        return colMan;

    }	// GetCollisionManifold */


    /************************************************************************
    *    desc:  Resolve the collision between two sprites  
    *
    *	 param: CCollisionManifold & colMan - manifold to hold the collision
    *										  data
    *			CSpriteGroup2D * pSpriteA   - sprite to resolve
    *			CSpriteGroup2D * pSpriteB   - sprite to resolve 
    ************************************************************************/
    bool ResolveCollision( CCollisionManifold & colMan, CSpriteGroup2D * pSpriteA, 
                                                        CSpriteGroup2D * pSpriteB )
    {
        // Get the collision sprites
        CCollisionSprite2D * pColSpriteA = pSpriteA->GetCollisionSprite();
        CCollisionSprite2D * pColSpriteB = pSpriteB->GetCollisionSprite();

        // Make sure at least one sprite doesn't have infinite mass
        if( pColSpriteA->GetBody().GetMass() != 0 || pColSpriteB->GetBody().GetMass() != 0 )
        {
            // See if the bounding boxes are intersecting
            if( NCollisionFunc2D::BoxRadiiIntersect( pSpriteA->GetPos(), pSpriteA->GetRadius(),
                                                     pSpriteB->GetPos(), pSpriteB->GetRadius() ) )
            {
                // Get the first collision data
                CCollisionManifold colManA = GetCollisionManifold( pSpriteA, pSpriteB );

                // If the penetration was positive, we're not colliding
                if( colManA.penetration > 0.f )
                    return false;

                // Get the second collision data
                CCollisionManifold colManB = GetCollisionManifold( pSpriteB, pSpriteA );

                // If the penetration was positive, we're not colliding
                if( colManB.penetration > 0.f )
                    return false;

                // Figure out which sprite should be the reference and which should be the incident sprite
                if( colManA.penetration >= colManB.penetration * BIAS_RELATIVE + colManA.penetration * BIAS_ABSOLUTE )
                {
                    colMan = colManA;
                    colMan.pRefSprite = pSpriteA;
                    colMan.pIncSprite = pSpriteB;
                }
                else
                {
                    colMan = colManB;
                    colMan.pRefSprite = pSpriteB;
                    colMan.pIncSprite = pSpriteA;
                }

                // Find the incident edge
                colMan.FindIncidentEdge();

                // Set the collision normal
                colMan.normal = colMan.pRefEdge->normal;

                // Calculate the unit vector along the reference edge
                CPoint sidePlaneNormal = CPoint( colMan.normal.y, -colMan.normal.x );

                // ax + by = c
                // c is distance from origin
                CWorldValue refC    =  NMathFunc::DotProduct2D( colMan.pRefEdge->pVert[0]->GetPos(), colMan.normal );
                CWorldValue negSide = -NMathFunc::DotProduct2D( colMan.pRefEdge->pVert[0]->GetPos(), sidePlaneNormal );
                CWorldValue posSide =  NMathFunc::DotProduct2D( colMan.pRefEdge->pVert[1]->GetPos(), sidePlaneNormal );
                CWorldPoint possibleContact[2] = { colMan.pIncEdge->pVert[0]->GetPos(), colMan.pIncEdge->pVert[1]->GetPos() };

                // Clip incident edge to determine the two possible contact points
                if( Clip( -sidePlaneNormal, negSide, possibleContact ) < 2 )
                    return false; // Due to floating point error, possible to not have required points

                if( Clip(  sidePlaneNormal, posSide, possibleContact ) < 2 )
                    return false; // Due to floating point error, possible to not have required points
                
                // Determine which of the possible contact points are, in fact, contacting
                CWorldValue separation;
                separation = NMathFunc::DotProduct2D( possibleContact[0], colMan.normal ) - refC;
                if(separation <= 0.0f)
                {
                    colMan.contactPoint[colMan.contactCount] = possibleContact[0];
                    colMan.penetration = -separation;
                    ++colMan.contactCount;
                }
                else
                    colMan.penetration = 0;

                separation = NMathFunc::DotProduct2D( possibleContact[1], colMan.normal ) - refC;
                if(separation <= 0.0f)
                {
                    colMan.contactPoint[colMan.contactCount] = possibleContact[1];
                    colMan.penetration += -separation;
                    ++colMan.contactCount;
                }

                // Average the penetration amount if there were two points of contact
                if( colMan.contactCount == 2 )
                    colMan.penetration *= 0.5f;

                return true;
            }
        }

        return false;

    }	// ResolveCollision */
    

    /************************************************************************
    *    desc:  Clip the passed in edge against the passed in side  
    *
    *	 param: CPoint & sidePlaneNormal - vector along the reference edge
    *			CWorldValue & side       - positive or negative side value 
    *			CWorldPoint * pIncVert	 - two vert positions that make up the
    *									   incident edge
    ************************************************************************/
    int Clip( CPoint & sidePlaneNormal, CWorldValue & side, CWorldPoint * pIncVert )
    {
        // The number of support points. There should be no more than two
        uint sp = 0;

        // The clipped edge
        CWorldPoint clippedPoint[2] = { pIncVert[0], pIncVert[1] };

        // Retrieve distances from each endpoint to the line
        // d = ax + by - c
        CWorldValue d1 = NMathFunc::DotProduct2D( pIncVert[0], sidePlaneNormal ) - side;
        CWorldValue d2 = NMathFunc::DotProduct2D( pIncVert[1], sidePlaneNormal ) - side;

        // If negative (behind plane) clip
        if( d1 <= 0.0f ) 
        {
            clippedPoint[sp] = pIncVert[0];
            ++sp;
        }

        if( d2 <= 0.0f ) 
        {
            clippedPoint[sp] = pIncVert[1];
            ++sp;
        }
  
        // If the points are on different sides of the plane
        if( d1 * d2 < 0.0f )
        {
            // Push interesection point
            CWorldValue alpha = d1 / (d1 - d2);
            clippedPoint[sp] = pIncVert[0] + ( pIncVert[1] - pIncVert[0] ) * alpha;
            ++sp;
        }

        // Assign our new converted values
        pIncVert[0] = clippedPoint[0];
        pIncVert[1] = clippedPoint[1];

        if( sp == 3 )
            throw NExcept::CCriticalException( "Collision Resolution Error!", 
                                               "Found three support points." );

        return sp;

    }	// Clip */


    /************************************************************************
    *    desc:  Apply an impulse from a specific point  
    *
    *	 param: CWorldPoint & point          - point of impulse
    *			float radius                 - radius of impulse 
    *			float inverseRadius			 - 1 divided by the radius
    *			float force	                 - strength of impulse
    *			CCollisionSprite2D * pSprite - sprite to apply impulse to
    *			bool diminishingForce        - whether or not the strength of
    *										   the impulse is reduced based on
    *										   the distance from the sprite
    ************************************************************************/
    void ApplyPointImpulse( CWorldPoint & point, float radius, float inverseRadius, float force, 
                            CSpriteGroup2D * pSprite, bool diminishingForce )
    {
        // Get the collision sprite
        CCollisionSprite2D * pColSprite = pSprite->GetCollisionSprite();

        // Find edge with minimum penetration
        // Exact concept as using support points in Polygon vs Polygon
        float separation = -FLT_MAX;
        CEdge * pEdge;

        // Check each edge of the collision sprite for a collision
        for( uint i = 0; i < pColSprite->GetOuterEdgeCount(); ++i )
        {
            CEdge * pTmpEdge = pColSprite->GetOuterEdge(i);

            // Find the separation
            float tmpSeparation = NMathFunc::DotProduct2D( point - pTmpEdge->pVert[0]->GetPos(), pTmpEdge->normal );

            // If the separatio is greater than the radius, the sprite is too far from the point to collide
            if( tmpSeparation > radius )
                return;

            // Otherwise, if the separation is greater than the previous one, save it along with the collided edge
            if( tmpSeparation > separation )
            {
                separation = tmpSeparation;
                pEdge = pTmpEdge;
            }
        }

        // Collision information
        CWorldPoint contact;
        float penetration;
        CPoint normal;

        // Check to see if center is within polygon
        if( separation < MIN_SEPARATION )
        {
            normal = -pEdge->normal;
            contact = point + ( normal * radius );
            penetration = radius;
        }
        else
        {
            penetration = radius - separation;

            // Determine which voronoi region of the edge center of circle lies within
            float dot0 = NMathFunc::DotProduct2D( point - pEdge->pVert[0]->GetPos(), 
                                                  pEdge->pVert[1]->GetPos() - pEdge->pVert[0]->GetPos() );
            float dot1 = NMathFunc::DotProduct2D( point - pEdge->pVert[1]->GetPos(), 
                                                  pEdge->pVert[0]->GetPos() - pEdge->pVert[1]->GetPos() );
            // Closest to vert 0
            if( dot0 <= 0 )
            {
                // See if the squared distance between vert 0 and the point is greater than the radius squared.
                // If so, we're not colliding
                if( (point - pEdge->pVert[0]->GetPos()).GetLengthSquared() > radius * radius )
                    return;

                normal = pEdge->pVert[0]->GetPos() - point;
                normal.Normalize2D();
                contact = pEdge->pVert[0]->GetPos();
            }
            // Closest to vert 1
            else if( dot1 <= 0 )
            {
                // See if the squared distance between vert 0 and the point is greater than the radius squared.
                // If so, we're not colliding
                if( (point - pEdge->pVert[1]->GetPos()).GetLengthSquared() > radius * radius )
                    return;

                normal = pEdge->pVert[1]->GetPos() - point;
                normal.Normalize2D();
                contact = pEdge->pVert[1]->GetPos();
            }
            // Closest to edge face
            else
            {
                // If the projected vector from the vert position to the point along the edge's normal is greater
                // than the radius, we're not colliding
                if( NMathFunc::DotProduct2D( point - pEdge->pVert[0]->GetPos(), pEdge->normal ) > radius )
                    return;

                normal = -pEdge->normal;
                contact = point + (normal * radius);
            }
        }

        CPoint impulseVec;
        CPoint velocityVec;
        float angVelocity;

        // If we wan't diminishing force, we multiply the force by the penetration over the radius
        if( diminishingForce )
            impulseVec = normal * force * penetration * inverseRadius;
        else
            impulseVec = normal * force;

        // Calculate and set the velocity and angular velocity of the sprite
        velocityVec = pColSprite->GetBody().GetVelocity() + impulseVec * pColSprite->GetBody().GetInverseMass();
        angVelocity = pColSprite->GetBody().GetAngVelocity() + pColSprite->GetBody().GetInverseInertia() * 
                      NMathFunc::CrossProduct2D( contact - pSprite->GetPos(), impulseVec );
        pColSprite->GetBody().SetVelocity( velocityVec );
        pColSprite->GetBody().SetAngVelocity( angVelocity );

    }	// ApplyPointImpulse */

}	// NCollisionResFunc2D