
/************************************************************************
*    FILE NAME:       collisionsprite.cpp
*
*    DESCRIPTION:     Class to hold and handle collision sprite information
************************************************************************/

// Physical component dependency
#include <2d/collisionsprite2d.h>

// Standard lib dependencies

// Boost lib dependencies
#include <boost/format.hpp>

// Game lib dependencies
#include <common/defs.h>
#include <common/worldpoint.h>
#include <common/pointint.h>
#include <2d/objectdatalist2d.h>
#include <2d/objectdata2d.h>
#include <common/object.h>
#include <2d/physicsworld.h>
#include <managers/physicsworldmanager.h>
#include <utilities/exceptionhandling.h>
#include <utilities/genfunc.h>

// Required namespace(s)
using namespace std;

// Turn off the data type conversion warning (ie. int to float, float to int etc.)
// We do this all the time in 3D. Don't need to be bugged by it all the time.
#pragma warning(disable : 4244)
// disable warning about the use of the "this" pointer in the constructor list
#pragma warning(disable : 4355)

const float PX_TO_B2D = 0.1f;
const float B2D_TO_PX = 10.f;


/************************************************************************
*    desc:  Constructer
************************************************************************/
CCollisionSprite2D::CCollisionSprite2D()
                   : preStepRot(0),
                     postStepRot(0),
                     pWorld(NULL),
                     pBody(NULL),
                     pObjectData(NULL),
                     pParent(NULL)
{
}   // constructor


/************************************************************************
*    desc:  Constructer
*
*	 param:	CObjectData2D * pObjData - data to intialize with
*			CObject * pParentObj     - collision sprite's parent
************************************************************************/
CCollisionSprite2D::CCollisionSprite2D( CObjectData2D * pObjData, CObject * pParentObj )
                   : preStepRot(0),
                     postStepRot(0),
                     pWorld(NULL),
                     pBody(NULL),
                     pObjectData(pObjData),
                     pParent(pParentObj)
{
    Init();

}   // constructor


/************************************************************************
*    desc:  Constructer
*
*	 param:	const CObjectCollisionData2D & colData - data to intialize with
*			CObject * pParentObj                   - collision sprite's parent
************************************************************************/
CCollisionSprite2D::CCollisionSprite2D( const CObjectCollisionData2D & colData, CObject * pParentObj )
                   : preStepRot(0),
                     postStepRot(0),
                     pWorld(NULL),
                     pBody(NULL),
                     pObjectData(NULL),
                     pParent(pParentObj)
{
    Init( colData );

}   // constructor


/************************************************************************
*    desc:  destructer                                                             
************************************************************************/
CCollisionSprite2D::~CCollisionSprite2D()
{
    for( size_t i = 0; i < fixtureDefVec.size(); ++i )
        delete fixtureDefVec[i].shape;

    if( pBody )
        pWorld->DestroyBody( pBody );

}	// destructer


/************************************************************************
*    desc:  Initialize the sprite. Should be called after the parent's
*			position, rotation, and scale are set
************************************************************************/
void CCollisionSprite2D::Init()
{
    // If we already have a body, we're initialized, and if we don't have
    // any object data, we can't initialize with this function
    if( pBody || !pObjectData )
        throw NExcept::CCriticalException("Collision Sprite 2D Error!",
                boost::str( boost::format("Sprite can't be initialized.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

    // Get the physics world
    pWorld = CPhysicsWorldManager::Instance().GetWorld( pObjectData->GetCollisionData().GetWorld() );

    if( pObjectData->GetCollisionData().GetFile() == "rect" )
    {
        // Set the size of the collision mesh
        CSize<float> size;

        // If there is no collision size, use the visual size
        if( !pObjectData->GetCollisionData().GetSize().IsEmpty() )
            size = pObjectData->GetCollisionData().GetSize() * 0.5f;
        else
            size = pObjectData->GetVisualData().GetSize() * 0.5f;

        // If the size is zero, we can't initialize it
        if( size.IsEmpty() )
            throw NExcept::CCriticalException("Collision Sprite 2D Error!",
                boost::str( boost::format("Sprite can't be initialized with no size.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

        // Set the box shape
        b2Vec2 vertArr[4];

        // Bottom and left mod have their signs flipped so that a positive mod always means
        // expansion of the side, and a negative mod always means a contraction of the side
        float topMod = pObjectData->GetCollisionData().GetTopMod();
        float bottomMod = -pObjectData->GetCollisionData().GetBottomMod();
        float leftMod = -pObjectData->GetCollisionData().GetLeftMod();
        float rightMod = pObjectData->GetCollisionData().GetRightMod();

        // Get the vertex positions
        vertArr[0].x = (-size.w + leftMod) * PX_TO_B2D;
        vertArr[0].y = (-size.h + bottomMod) * PX_TO_B2D;
        vertArr[1].x = ( size.w + rightMod) * PX_TO_B2D;
        vertArr[1].y = (-size.h + bottomMod) * PX_TO_B2D;
        vertArr[2].x = ( size.w + rightMod) * PX_TO_B2D;
        vertArr[2].y = ( size.h + topMod) * PX_TO_B2D;
        vertArr[3].x = (-size.w + leftMod) * PX_TO_B2D;
        vertArr[3].y = ( size.h + topMod) * PX_TO_B2D;

        // Define the shape
        b2PolygonShape * pShape = new b2PolygonShape();
        pShape->Set( vertArr, 4 );

        // Define the fixture
        b2FixtureDef fd;
        fd.shape = pShape;
        fd.density = pObjectData->GetCollisionData().GetDensity();
        fd.restitution = pObjectData->GetCollisionData().GetRestitution();
        fixtureDefVec.push_back( fd );

        // Create the body
        b2BodyDef bd;
        bd.type = pObjectData->GetCollisionData().GetType();
        bd.fixedRotation = pObjectData->GetCollisionData().IsRotationFixed();
        bd.userData = (void*)this;

        // Create the body
        pBody = pWorld->CreateBody( &bd );

        // Set the damping
        pBody->SetLinearDamping( pObjectData->GetCollisionData().GetDamping() );
        pBody->SetAngularDamping( pObjectData->GetCollisionData().GetAngDamping() );

        // Now assemble the body from the fixture defs
        ReassembleBody();

        // Set the initial transformations
        if( pParent )
        {
            CPoint pos = pParent->GetPos() - pWorld->GetFocus();
            pBody->SetTransform( b2Vec2( pos.x * PX_TO_B2D, pos.y * PX_TO_B2D ), 
                                 pParent->GetRot().z );
        }
    }

}	// Init


/************************************************************************
*    desc:  Initialize the sprite
*
*	 param:	const CObjectCollisionData2D & colData - data to initalize with
************************************************************************/
void CCollisionSprite2D::Init( const CObjectCollisionData2D & colData )
{
    // If we already have a body, we're initialized, and if we don't have
    // any object data, we can't initialize with this function
    if( pBody )
        throw NExcept::CCriticalException("Collision Sprite 2D Error!",
                boost::str( boost::format("Sprite can't be initialized.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

    // Get the physics world
    pWorld = CPhysicsWorldManager::Instance().GetWorld( colData.GetWorld() );

    if( colData.GetFile() == "rect" )
    {
        // Set the size of the collision mesh
        CSize<float> size = colData.GetSize() * 0.5f;

        // If the size is zero, we can't initialize it
        if( size.IsEmpty() )
            throw NExcept::CCriticalException("Collision Sprite 2D Error!",
                boost::str( boost::format("Sprite can't be initialized with no size.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

        // Set the box shape
        b2Vec2 vertArr[4];

        // Bottom and left mod have their signs flipped so that a positive mod always means
        // expansion of the side, and a negative mod always means a contraction of the side
        float topMod = colData.GetTopMod();
        float bottomMod = -colData.GetBottomMod();
        float leftMod = -colData.GetLeftMod();
        float rightMod = colData.GetRightMod();

        // Get the vertex positions
        vertArr[0].x = (-size.w + leftMod) * PX_TO_B2D;
        vertArr[0].y = (-size.h + bottomMod) * PX_TO_B2D;
        vertArr[1].x = ( size.w + rightMod) * PX_TO_B2D;
        vertArr[1].y = (-size.h + bottomMod) * PX_TO_B2D;
        vertArr[2].x = ( size.w + rightMod) * PX_TO_B2D;
        vertArr[2].y = ( size.h + topMod) * PX_TO_B2D;
        vertArr[3].x = (-size.w + leftMod) * PX_TO_B2D;
        vertArr[3].y = ( size.h + topMod) * PX_TO_B2D;

        // Define the shape
        b2PolygonShape * pShape = new b2PolygonShape();
        pShape->Set( vertArr, 4 );

        // Define the fixture
        b2FixtureDef fd;
        fd.shape = pShape;
        fd.density = colData.GetDensity();
        fd.restitution = colData.GetRestitution();
        fixtureDefVec.push_back( fd );

        // Create the body
        b2BodyDef bd;
        bd.type = colData.GetType();
        bd.fixedRotation = colData.IsRotationFixed();
        bd.userData = (void*)this;

        // Create the body
        pBody = pWorld->CreateBody( &bd );

        // Set the damping
        pBody->SetLinearDamping( colData.GetDamping() );
        pBody->SetAngularDamping( colData.GetAngDamping() );

        // Now assemble the body from the fixture defs
        ReassembleBody();

        // Set the initial transformations
        if( pParent )
        {
            CPoint pos = pParent->GetPos() - pWorld->GetFocus();
            pBody->SetTransform( b2Vec2( pos.x * PX_TO_B2D, pos.y * PX_TO_B2D ), 
                                 pParent->GetRot().z );
        }
    }

}	// Init


/************************************************************************
*    desc:  Quickly initialize a static collision sprite
*
*	 param:	const string & worldName  - the physics world the sprite belongs to
*			const CSize<float> & size - the size to make the sprite
************************************************************************/
void CCollisionSprite2D::Init( const string & worldName, const CSize<float> & size )
{
    // If we already have a body, we're initialized
    if( pBody )
        throw NExcept::CCriticalException("Collision Sprite 2D Error!",
                boost::str( boost::format("Sprite can't be initialized.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

    // Get the physics world
    pWorld = CPhysicsWorldManager::Instance().GetWorld( worldName );

    // Define the body
    b2BodyDef bd;
    bd.type = b2_staticBody;
    bd.userData = (void*)this;

    // Define the shape
    CPoint tmpScale = GetScale();
    b2PolygonShape * pShape = new b2PolygonShape();

    // Set the box shape
    pShape->SetAsBox( tmpScale.x * size.w * 0.5f * PX_TO_B2D, 
                      tmpScale.y * size.h * 0.5f * PX_TO_B2D );

    // Define the fixture
    b2FixtureDef fd;
    fd.shape = pShape;
    fixtureDefVec.push_back( fd );

    // Create the body
    pBody = pWorld->CreateBody( &bd );
    
    // Now assemble the body from the fixture defs
    ReassembleBody();

}	// Init


/************************************************************************
*    desc:  Apply transformations either to the parent or the collision 
*			sprite
************************************************************************/
void CCollisionSprite2D::Transform()
{
    // If the sprite has no body, no parent, or is new, leave the function
    if( !pBody || !pParent )
        return;

    // If the parent was translated, set the position of the body
    if( pParent->GetParameters().IsSet( CObject::TRANSLATE ) )
    {
        CPoint pos = ( pParent->GetPos() - pWorld->GetFocus() ) * PX_TO_B2D;
        preStepPos = pos;
        postStepPos = pos;
        pBody->SetTransform( b2Vec2( pos.x, pos.y ), pBody->GetAngle() );
    }

    // If the parent was rotated, set the rotation of the body
    if( !pBody->IsFixedRotation() && pParent->GetParameters().IsSet( CObject::ROTATE ) )
    {
        float rot = pParent->GetRot().z * DEG_TO_RAD;
        preStepRot = rot;
        postStepRot = rot;
        pBody->SetTransform( pBody->GetPosition(), rot );
    }

    // If the parent was scaled, we need to recreate the body
    if( pParent->GetParameters().IsSet( CObject::SCALE ) )
        ReassembleBody();

}	// Transform


/************************************************************************
*    desc:  Get the collision sprite's world position
*  
*    ret:	CWorldPoint - position
************************************************************************/
CWorldPoint CCollisionSprite2D::GetPos() const
{
    if( pBody )
    {
        CPoint tmpPos( pBody->GetPosition().x, pBody->GetPosition().y );
        return tmpPos * B2D_TO_PX;
    }

    return CWorldPoint();

}   // GetPos


/************************************************************************
*    desc:  Get the collision sprite's Box2D position
*  
*    ret:	const b2Vec2 & - position
************************************************************************/
const b2Vec2 & CCollisionSprite2D::GetB2DPos() const
{
    if( !pBody )
        throw NExcept::CCriticalException("Collision Sprite Error!",
                boost::str( boost::format("Physics body doesn't exist.\n\n%s\nLine: %s") 
                            % __FUNCTION__ % __LINE__ ));

    return pBody->GetPosition();

}   // GetB2DPos


/************************************************************************
*    desc:  Get the collision sprite's interpolated position between the
*			pre and post step positions
*  
*    ret:	CWorldPoint - position
************************************************************************/
CWorldPoint CCollisionSprite2D::GetInterpPos() const
{
    return ( postStepPos - preStepPos ) * pWorld->GetTimeRatio() + preStepPos;

}   // GetInterpPos


/************************************************************************
*    desc:  Set pre and post step positions and rotations
************************************************************************/
void CCollisionSprite2D::SetPrePostData()
{
    // The pre position becomes the post position and the post position
    // is set by the body's current position
    preStepPos = postStepPos;
    postStepPos = GetPos();
    preStepRot = postStepRot;
    postStepRot = GetRot();

}	// SetPrePostPos


/************************************************************************
*    desc:  Get the collision sprite's rotation
*
*	 param: bool inDegrees - whether to return the rotation in degrees
*							 or radians
*  
*    ret:	float - rotation ammount
************************************************************************/
float CCollisionSprite2D::GetRot( bool inDegrees ) const
{
    if( pBody )
    {
        if( inDegrees )
            return pBody->GetAngle() * RAD_TO_DEG;

        return pBody->GetAngle();
    }

    return 0;

}   // GetRot


/************************************************************************
*    desc:  Get the collision sprite's interpolated rotation between the
*			pre and post step rotations
*  
*    ret:	float - rotation ammount
************************************************************************/
float CCollisionSprite2D::GetInterpRot() const
{
    return ( postStepRot - preStepRot ) * pWorld->GetTimeRatio() + preStepRot;

}   // GetInterpRot


/************************************************************************
*    desc:  Get the collision sprite's scale
*  
*    ret:	const CPoint & - sprite scale
************************************************************************/
CPoint CCollisionSprite2D::GetScale() const
{
    if( pParent )
        return pParent->GetScale();

    return CPoint(1,1,1);

}   // GetScale


/************************************************************************
*    desc:  Apply an acceleration to the sprite. This function must be called 
*			repeatedly for constant acceleration
*  
*    param:	const CPoint & accel - acceleration to apply
************************************************************************/
void CCollisionSprite2D::ApplyAcceleration( const CPoint & accel )
{
    // Multiply mass into the force so that we accelerate at the same rate for
    // objects of any mass
    b2Vec2 force( accel.x * pBody->GetMass(), accel.y * pBody->GetMass());
    pBody->ApplyForceToCenter( force, true );

}	// SetVelocity


/************************************************************************
*    desc:  Get the sprite's velocity
*  
*    ret:	CPoint - velocity of the sprite
************************************************************************/
CPoint CCollisionSprite2D::GetVelocity() const
{
    if( pBody )
        return CPoint( pBody->GetLinearVelocity().x, 
                       pBody->GetLinearVelocity().y );

    return CPoint();

}	// GetVelocity


/************************************************************************
*    desc:  Get the magnitude of the sprite's velocity
*  
*    ret:	float - velocity of the sprite
************************************************************************/
float CCollisionSprite2D::GetVelocityMag() const
{
    if( pBody )
    {
        b2Vec2 v =  pBody->GetLinearVelocity();
        return sqrt(v.x * v.x + v.y * v.y);
    }

    return 0;

}	// GetVelocityMag


/************************************************************************
*    desc:  Check if the passed in point is within the sprite
*  
*    param:	const CPoint/CPointInt/CWorldPoint & - point to check
*
*	 ret:	bool - result
************************************************************************/
bool CCollisionSprite2D::IsPointInSprite( const CPoint & point )
{
    if( pBody )
    {
        CPoint tmpPos = (point - pWorld->GetFocus()) * PX_TO_B2D;
        b2Vec2 tmpVec(tmpPos.x, tmpPos.y);

        for( size_t i = 0; i < pFixtureVec.size(); ++i )
            if( pFixtureVec[i]->GetShape()->TestPoint( pBody->GetTransform(), tmpVec ) )
                return true;
    }

    return false;

}	// IsPointInSprite

bool CCollisionSprite2D::IsPointInSprite( const CPointInt & point )
{
    if( pBody )
    {
        CPoint tmpPos = (point - pWorld->GetFocus()) * PX_TO_B2D;
        b2Vec2 tmpVec(tmpPos.x, tmpPos.y);

        for( size_t i = 0; i < pFixtureVec.size(); ++i )
            if( pFixtureVec[i]->GetShape()->TestPoint( pBody->GetTransform(), tmpVec ) )
                return true;
    }

    return false;

}	// IsPointInSprite

bool CCollisionSprite2D::IsPointInSprite( const CWorldPoint & point )
{
    if( pBody )
    {
        CPoint tmpPos = (point - pWorld->GetFocus()) * PX_TO_B2D;
        b2Vec2 tmpVec(tmpPos.x, tmpPos.y);

        for( size_t i = 0; i < pFixtureVec.size(); ++i )
            if( pFixtureVec[i]->GetShape()->TestPoint( pBody->GetTransform(), tmpVec ) )
                return true;
    }

    return false;

}	// IsPointInSprite


/************************************************************************
*    desc:  Set the body pointer
*  
*	 NOTE:	Do not call this function. This function is only meant to be
*			called by Box2D
*
*    param:	b2Body * pBdy - new body pointer
************************************************************************/
void CCollisionSprite2D::SetBody( b2Body * pBdy )
{
    pBody = pBdy;

}	// SetBody


/************************************************************************
*    desc:  Set the sprite's parent
*
*    param:	CObject * pObj - parent pointer
************************************************************************/
void CCollisionSprite2D::SetParent( CObject * pObj )
{
    pParent = pObj;

}	// SetBody


/************************************************************************
*    desc:  Get the type of collision sprite
*
*    ret:	b2BodyType - type
************************************************************************/
b2BodyType CCollisionSprite2D::GetType() const
{
    if( pBody )
        return pBody->GetType();

    return b2_staticBody;

}	// GetType


/************************************************************************
*    desc:  Is the collision sprite awake
*
*    ret:	bool - result
************************************************************************/
bool CCollisionSprite2D::IsAwake() const
{
    if( pBody )
        return pBody->IsAwake();

    return false;

}	// IsAwake


/************************************************************************
*    desc:  Is the collision sprite unable to rotate based on collisions
*
*    ret:	bool - result
************************************************************************/
bool CCollisionSprite2D::IsRotationFixed() const
{
    if( pBody )
        return pBody->IsFixedRotation();

    return false;

}	// IsRotationFixed


/************************************************************************
*    desc:  Set the active flag of the collision sprite
*  
*    param:	bool value - active flag
************************************************************************/
void CCollisionSprite2D::SetActive( bool value )
{
    if( pBody )
        pBody->SetActive( value );

}	// SetActive


/************************************************************************
*    desc:  Get the active flag of the collision sprite
*  
*    ret:	bool - active flag
************************************************************************/
bool CCollisionSprite2D::IsActive() const
{
    if( pBody )
        return pBody->IsActive();

    return false;

}	// IsActive


/************************************************************************
*    desc:  Get the physics world this sprite belongs to
*  
*    ret:	const CPhysicsWorld * - world
************************************************************************/
const CPhysicsWorld * CCollisionSprite2D::GetWorld() const
{
    return pWorld;

}	// GetWorld


/************************************************************************
*    desc:  Reassemble the collision sprite's body
************************************************************************/
void CCollisionSprite2D::ReassembleBody()
{
    if( !pBody )
        return;

    // Make sure there are no fixtures
    for( size_t i = 0; i < pFixtureVec.size(); ++i )
        pBody->DestroyFixture( pFixtureVec[i] );

    pFixtureVec.clear();

    // Scale used to reconstruct the geometry
    CPoint scale;

    if( pParent )
        scale = pParent->GetScale();

    // Loop through each fixture and recreate it
    for( size_t i = 0; i < fixtureDefVec.size(); ++i )
    {
        switch( fixtureDefVec[i].shape->GetType() )
        {
            // If the shape is a polygon, we go here
            case b2Shape::e_polygon:
            {
                b2PolygonShape * pUnscaledShape = (b2PolygonShape *)fixtureDefVec[i].shape;

                // Get the vertices of the shape
                b2Vec2 vertArrScaled[b2_maxPolygonVertices];

                // Create a new vertex list and apply the scale
                for( int j = 0; j < pUnscaledShape->GetVertexCount(); ++j )
                {
                    vertArrScaled[j].x = pUnscaledShape->GetVertex(j).x * scale.x;
                    vertArrScaled[j].y = pUnscaledShape->GetVertex(j).y * scale.y;
                }
                
                // Define the shape
                b2PolygonShape shape;
                shape.Set( vertArrScaled, pUnscaledShape->GetVertexCount() );

                // Define the fixture
                b2FixtureDef fd;
                fd.shape = &shape;
                fd.density = fixtureDefVec[i].density;
                fd.restitution = fixtureDefVec[i].restitution;
                
                // Create the fixture and place it in our vector
                pFixtureVec.push_back( pBody->CreateFixture( &fd ) );
                break;
            }
        }
    }

}	// ReassembleBody