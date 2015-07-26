
/************************************************************************
*    FILE NAME:       playershipai.h
*
*    DESCRIPTION:     Class to apply changes to the sprite in which the
*                     player controls using a gamepad, keyboard, and 
*                     mouse.
************************************************************************/

#ifndef __player_ship_ai_h__
#define __player_ship_ai_h__

// Physical component dependency
#include <2d/iaibase.h>

// Game lib dependencies
#include <utilities/timer.h>
#include <common/point.h>

// Forward declaration(s)
class CActorSprite2D;
class CSpriteGroup2D;

class CPlayerShipAI : public iAIBase
{
public:

    // Constructor
    explicit CPlayerShipAI( CActorSprite2D * _pActor );

    // Destructor
    virtual ~CPlayerShipAI();

    // React to what the player is doing
    virtual void HandleGameInput();

    // Check for collision and react to it
    virtual void ReactToCollision(){};

    // Update animations, Move sprites
    virtual void Update(){}

private:

    // Handle the input if we're using mouse controls
    void HandleMouseKeyboardControls();

    // Handle the input if we're using joypad controls
    void HandleJoypadControls();

    // Handle the ship's acceleration
    void HandleAcceleration();

    // Handle the ship's rotation
    void HandleRotation();

    // Handle the ship's shooting
    void HandleShooting();

    // Player shoot timer
    CTimer playerShootTimer;

    // The velocity and acceleration vectors of the ship
    CPoint velocity;
    CPoint acceleration;

    // The speed the ship is rotating
    float angularVelocity;
    float angularAcceleration;

    // The elapsed time of the last frame
    float elapsedTime;

    // Rotation we're transitioning to
    float rotDestination;

    // The flags for accelerating and rotating
    bool accelerating, rotating, shooting;

    // The rotation amount of the ship's gun
    float gunRotation;

    // Fire tail sprite group - NOTE: Do not free. Don't own these pointers
    CSpriteGroup2D * pFireTail;

    // Fire tail sprite group - NOTE: Do not free. Don't own these pointers
    CSpriteGroup2D * pGun;

};

#endif  // __player_ship_ai_h__


