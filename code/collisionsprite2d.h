
/************************************************************************
*    FILE NAME:       collisionsprite.h
*
*    DESCRIPTION:     Class to hold and handle collision sprite 
*					  information.
************************************************************************/

#ifndef __new_collision_sprite_h__
#define __new_collision_sprite_h__

// Standard lib dependencies
#include <string>
#include <vector>

// Boost lib dependencies
#include <boost/container/vector.hpp>

// Game lib dependencies
#include <Box2D/Box2D.h>
#include <common/worldpoint.h>
#include <common/point.h>
#include <common/size.h>

// Forward declaration(s)
class CWorldPoint;
class CObjectData2D;
class CObjectCollisionData2D;
class CPointInt;
class CPhysicsWorld;
class CObject;

class CCollisionSprite2D
{
public:

    // Constructor
    CCollisionSprite2D();
    CCollisionSprite2D( CObjectData2D * pObjData, CObject * pParentObj );
    CCollisionSprite2D( const CObjectCollisionData2D & colData, CObject * pParentObj );

    // Destructor
    virtual ~CCollisionSprite2D();

    // Initialize the sprite
    void Init();
    void Init( const CObjectCollisionData2D & colData );
    void Init( const std::string & worldName, const CSize<float> & size );

    // Apply transformations either to the parent or the collision sprite
    void Transform();

    // Get the collision sprite's position
    CWorldPoint GetPos() const;
    const b2Vec2 & GetB2DPos() const;
    CWorldPoint GetInterpPos() const;

    // Set pre and post step positions and rotations
    void SetPrePostData();

    // Get the collision sprite's rotation
    float GetRot( bool inDegrees = true ) const;
    float GetInterpRot() const;

    // Get the collision sprite's scale
    CPoint GetScale() const;

    // Apply an acceleration to the sprite. This function must be called repeatedly
    // for constant acceleration
    void ApplyAcceleration( const CPoint & accel );

    // Get the sprite's velocity
    CPoint GetVelocity() const;
    float GetVelocityMag() const;

    // Check if the passed in point is within the sprite
    bool IsPointInSprite( const CPoint & point );
    bool IsPointInSprite( const CPointInt & point );
    bool IsPointInSprite( const CWorldPoint & point );

    // Set the body pointer
    void SetBody( b2Body * pBdy );

    // Set the sprite's parent
    void SetParent( CObject * pObj );

    // Get the type of collision sprite
    b2BodyType GetType() const;

    // Is the collision sprite awake
    bool IsAwake() const;

    // Is the collision sprite unable to rotate based on collisions
    bool IsRotationFixed() const;

    // Set-Get the active flag of the collision sprite
    void SetActive( bool value );
    bool IsActive() const;

    // Get the physics world this sprite belongs to
    const CPhysicsWorld * GetWorld() const;

private:

    // Reassemble the collision sprite's body
    void ReassembleBody();

private:

    // The positions and rotations before and after a step of physics calculations
    CWorldPoint preStepPos, postStepPos;
    float preStepRot, postStepRot;

    // The physics world that this collision sprite belongs to
    // NOTE: This pointer belongs to the CPhysicsWorldManager singleton
    CPhysicsWorld * pWorld;

    // Box2D body pointer
    b2Body * pBody;

    // Vector to hold the unscaled fixture definitions. This is so we can
    // scale collision sprites
    std::vector<b2FixtureDef> fixtureDefVec;

    // Vector to hold fixture pointers
    std::vector<b2Fixture *> pFixtureVec;

    // Object's data and parent pointers
    // NOTE: This data does not belong to the collision sprite class
    CObjectData2D * pObjectData;
    CObject * pParent;

};

#endif  // __new_collision_sprite_h__