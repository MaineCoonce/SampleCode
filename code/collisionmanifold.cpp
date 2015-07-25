
/***********************************************************************
*    FILE NAME:       collisionmanifold.cpp
*
*    DESCRIPTION:     Class to hold the collision information between two
*					  sprites
************************************************************************/

// Physical component dependency
#include <common/collisionmanifold.h>

// Standard lib dependencies

// Boost lib dependencies

// Game lib dependencies
#include <2d/spritegroup2d.h>
#include <2d/collisionsprite2d.h>
#include <common/defs.h>
#include <common/edge.h>
#include <common/collisionbody.h>
#include <utilities/mathfunc.h>

// Required namespace(s)
using namespace std;

// Turn off the data type conversion warning (ie. int to float, float to int etc.)
// We do this all the time in 3D. Don't need to be bugged by it all the time.
#pragma warning(disable : 4244)
// disable warning about the use of the "this" pointer in the constructor list
#pragma warning(disable : 4355)


// Constants used in moving objects from one another when piercing
const float PERCENT = 0.2f; // usually 0.2  to 0.8
const float SLOP = 0.01f;   // usually 0.01 to 0.1

// Radian and degrees conversion ratios
//const float RAD_TO_DEG = 57.2957795f;
//const float DEG_TO_RAD = 0.0174532925f;


/************************************************************************
*    desc:  Constructer
************************************************************************/
CCollisionManifold::CCollisionManifold()
                  : pRefSprite(NULL),
                    pIncSprite(NULL),
                    penetration(0),
                    pRefEdge(NULL),
                    pIncEdge(NULL),
                    contactCount(0)
{
}   // constructor


/************************************************************************
*    desc:  Push the two sprites away from each other if they're intersecting
************************************************************************/
void CCollisionManifold::PositionalCorrection()
{
    // Get the collision sprites
    CCollisionSprite2D * pRefColSprite = pRefSprite->GetCollisionSprite();
    CCollisionSprite2D * pIncColSprite = pIncSprite->GetCollisionSprite();

    // Push the sprites slightly away from each other
    CPoint correction = normal * PERCENT * max( penetration - SLOP, 0.f );
    correction /= pRefColSprite->GetBody().GetInverseMass() + 
                  pIncColSprite->GetBody().GetInverseMass();

    pRefColSprite->GetBody().SetPositionCorrection( correction * -pRefColSprite->GetBody().GetInverseMass() );
    pIncColSprite->GetBody().SetPositionCorrection( correction *  pIncColSprite->GetBody().GetInverseMass() );

}	// PositionalCorrection */


/************************************************************************
*    desc:  Find the incident edge using the collision data
************************************************************************/
void CCollisionManifold::FindIncidentEdge()
{
    // Get the incident collision sprite
    CCollisionSprite2D * pIncColSprite = pIncSprite->GetCollisionSprite();

    // Calculate the reference's normal in respect to the incident's matrix
    CPoint referenceNormal = pRefEdge->normal;
    
    float minValue = FLT_MAX;

    // Find the incident edge whose normal is the most unlike the reference normal
    for( uint i = 0; i < pIncColSprite->GetOuterEdgeCount(); ++i )
    {
        CEdge * pTmpEdge = pIncColSprite->GetOuterEdge(i);

        float value = NMathFunc::DotProduct2D( referenceNormal, pTmpEdge->normal );

        // If we find a value less than our last smallest value, save it and the edge pointer
        if( value < minValue )
        {
            minValue = value;
            pIncEdge = pTmpEdge;
        }
    }
    
}	// FindIncidentEdge */


/************************************************************************
*    desc:  Apply the impulse to the incident and reference sprites
************************************************************************/
void CCollisionManifold::ApplyImpulse()
{
    // Get the collision bodies to more easily retrieve data from them
    CCollisionBody * pRefBody = &pRefSprite->GetCollisionSprite()->GetBody();
    CCollisionBody * pIncBody = &pIncSprite->GetCollisionSprite()->GetBody();

    // Perform an impulse on each contect point
    for( int i = 0; i < contactCount; ++i )
    {
        // Calculate the vectors from the center of mass to the contact point
        CPoint refRadius = contactPoint[i] - pRefSprite->GetPos();
        CPoint incRadius = contactPoint[i] - pIncSprite->GetPos();

        // Calculate relative velocity
        CPoint relVelocity = pIncBody->GetVelocity() + NMathFunc::CrossProduct2D( pIncBody->GetAngVelocity(), incRadius ) -
                             pRefBody->GetVelocity() - NMathFunc::CrossProduct2D( pRefBody->GetAngVelocity(), refRadius );

        // Calculate the contact velocity
        float contactVelocity = NMathFunc::DotProduct2D( relVelocity, normal );
 
        // Do not resolve if velocities are separating
        if( contactVelocity > 0 )
            return;
 
        float refRadCrossN = NMathFunc::CrossProduct2D( refRadius, normal );
        float incRadCrossN = NMathFunc::CrossProduct2D( incRadius, normal );
        float invMassSum = pRefBody->GetInverseMass() + pIncBody->GetInverseMass() + ( refRadCrossN * refRadCrossN ) * 
                           pRefBody->GetInverseInertia() + ( incRadCrossN * incRadCrossN ) * pIncBody->GetInverseInertia();

        // Use the minimum restitution
        float minRestitution = min( pRefBody->GetRestitution(), pIncBody->GetRestitution() );

        // Calculate impulse scalar
        float impulse = -(1.0f + minRestitution) * contactVelocity;
        impulse /= invMassSum;

        // Calculate the impulse vector
        CPoint impulseVec = normal * impulse;

        // Temporary velocities to set to the sprites
        CPoint velocityVec;
        float angVelocity;

        // Calculate and set the velocity and angular velocity of the reference sprite
        velocityVec = pRefBody->GetVelocity() - impulseVec * pRefBody->GetInverseMass();
        angVelocity = pRefBody->GetAngVelocity() - pRefBody->GetInverseInertia() * NMathFunc::CrossProduct2D( refRadius, impulseVec );
        pRefBody->SetVelocity( velocityVec );
        pRefBody->SetAngVelocity( angVelocity );

        // Calculate and set the velocity and angular velocity of the incident sprite
        velocityVec = pIncBody->GetVelocity() + impulseVec * pIncBody->GetInverseMass();
        angVelocity = pIncBody->GetAngVelocity() + pIncBody->GetInverseInertia() * NMathFunc::CrossProduct2D( incRadius, impulseVec );
        pIncBody->SetVelocity( velocityVec );
        pIncBody->SetAngVelocity( angVelocity );
    }

}	// ApplyImpulse */