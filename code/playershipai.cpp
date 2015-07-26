
/************************************************************************
*    FILE NAME:       playershipai.cpp
*
*    DESCRIPTION:     Class to apply changes to the sprite in which the
*                     player controls using a gamepad, keyboard, and 
*                     mouse.
************************************************************************/

// Physical component dependency
#include "playershipai.h"

// Standard lib dependencies
#include <string>

// Game lib dependencies
#include <2d/actorsprite2d.h>
#include <2d/spritegroup2d.h>
#include <managers/instancemeshmanager.h>
#include <utilities/highresolutiontimer.h>
#include <utilities/genfunc.h>
#include <controller/gamecontroller.h>
#include <managers/actormanager.h>
#include <xact/xact.h>

// Game dependencies
#include "playerprojectileai.h"

// The min/max speed of the ship
const float MAX_VELOCITY = 6.0f;
const float MIN_VELOCITY = 0.001f;

// The acceleration of the ship
const float ACCELERATION = 0.003f;
const float DECCELERATION = 0.0005f;
const float BRAKE_DECCELERATION = 0.002f;

// The min/max speed of the ship's rotation
const float MAX_ANGULAR_VELOCITY = 0.7f;
const float MIN_ANGULAR_VELOCITY = 0.1f;

// The angular acceleration of the ship
const float ANGULAR_ACCELERATION = 0.005f;


/************************************************************************
*    desc:  Constructor
************************************************************************/
CPlayerShipAI::CPlayerShipAI( CActorSprite2D * _pActor )
             : iAIBase(_pActor),
               playerShootTimer(200, true),
               elapsedTime(0),
               acceleration(1,0,0),
               angularVelocity(0),
               angularAcceleration(0),
               pFireTail( pActor->GetSpriteGroup( "fireTail" ) ),
               pGun( pActor->GetSpriteGroup( "gun" ) )
{
}   // constructor


/************************************************************************
*    desc:  destructer                                                             
************************************************************************/
CPlayerShipAI::~CPlayerShipAI()
{
}	// destructer


/************************************************************************
*    desc:  Update                                                             
************************************************************************/
void CPlayerShipAI::HandleGameInput()
{
    // get the elapsed time
    elapsedTime = CHighResTimer::Instance().GetElapsedTime();

    // Reset the booleans
    accelerating = false;
    rotating = false;
    shooting = false;

    // If we're using mouse and keyboard controls, we go into this function to handle the input
    if( CGameController::Instance().GetLastDevicedUsed() == NDevice::MOUSE || 
        CGameController::Instance().GetLastDevicedUsed() == NDevice::KEYBOARD )
        HandleMouseKeyboardControls();
    
    // If we're using joypad controls, we go into this function to handle the input
    else if( CGameController::Instance().GetLastDevicedUsed() == NDevice::JOYPAD )
        HandleJoypadControls();

    // Handle the ship movement
    HandleAcceleration();

    // Handle the ship rotation
    HandleRotation();

    // Handle the ship shooting
    HandleShooting();

}	// HandleGameInput */


/************************************************************************
*    desc:  Handle the input if we're using mouse controls                                                             
************************************************************************/
void CPlayerShipAI::HandleMouseKeyboardControls()
{
    // Determine if we're moving
    if( CGameController::Instance().WasAction("Up")   == NDevice::EAP_HOLD &&
        CGameController::Instance().WasAction("Left") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 135;
    }
    else if( CGameController::Instance().WasAction("Up")    == NDevice::EAP_HOLD &&
             CGameController::Instance().WasAction("Right") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 45;
    }
    else if( CGameController::Instance().WasAction("Down") == NDevice::EAP_HOLD &&
             CGameController::Instance().WasAction("Left") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 225;
    }
    else if( CGameController::Instance().WasAction("Down")  == NDevice::EAP_HOLD &&
             CGameController::Instance().WasAction("Right") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 315;
    }
    else if( CGameController::Instance().WasAction("Up") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 90;
    }
    else if( CGameController::Instance().WasAction("Down") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 270;
    }
    else if( CGameController::Instance().WasAction("Left") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 180;
    }
    else if( CGameController::Instance().WasAction("Right") == NDevice::EAP_HOLD )
    {
        accelerating = true; rotating = true;
        rotDestination = 0;
    }

    if( CGameController::Instance().WasAction("Deccelerate") == NDevice::EAP_HOLD )
        accelerating = false;

    // Determine if a player wants to shoot a projectile
    shooting = ( CGameController::Instance().WasAction("Shoot") == NDevice::EAP_DOWN || 
                 CGameController::Instance().WasAction("Shoot") == NDevice::EAP_HOLD );

    // Calculate the rotation of the ship's gun
    CPoint tmpMousePos = CGameController::Instance().GetAbsolutePosScaled();
    gunRotation = atan2( tmpMousePos.y, tmpMousePos.x ) * 180.f / M_PI;

}	// HandleMouseKeyboardControls */


/************************************************************************
*    desc:  Handle the input if we're using joypad controls                                                             
************************************************************************/
void CPlayerShipAI::HandleJoypadControls()
{
    // Determine if we are rotating
    const CDeviceMovement &devMovement = CGameController::Instance().GetMovement();

    // Find out if we're accelerating. And if we're accelerating, we're rotating
    accelerating = ( abs(devMovement.gamepad1Y) > 5000 || abs(devMovement.gamepad1X) > 5000 );
    rotating = accelerating;

    // Calculate the rotation of the ship's gun
    if( abs(devMovement.gamepad2Y) > 3000 || abs(devMovement.gamepad2X) > 3000 )
        gunRotation = atan2( (float)devMovement.gamepad2Y, (float)devMovement.gamepad2X ) * -180.f / M_PI;

    // Determine if a player wants to shoot a projectile
    shooting = ( CGameController::Instance().WasAction("Shoot") == NDevice::EAP_DOWN || 
                 CGameController::Instance().WasAction("Shoot") == NDevice::EAP_HOLD );

    // If we have movement from the joypad's analog stick, we go in here and handle the ship's rotation
    if( rotating )
    {
        // Calculate the rotation value we're rotating towards
        rotDestination = atan2( (float)devMovement.gamepad1Y, (float)devMovement.gamepad1X );
        rotDestination = (rotDestination * -180.f) / M_PI;

        // Restrict the rotation to a positive number between 0 and 360
        if( rotDestination < 0 )
            rotDestination += 360.f;
        else if( rotDestination > 360 )
            rotDestination -= 360.f;
    }

}	// HandleJoypadControls */


/************************************************************************
*    desc:  Handle the ship's acceleration
************************************************************************/
void CPlayerShipAI::HandleAcceleration()
{
    // Hide the fire tail if we're not accelerating
    pFireTail->SetVisible( accelerating );

    // If the player is accelerating, we handle it here
    if( accelerating )
    {		
        CMatrix tmpMatrix = pActor->GetUnscaledMatrix();
        tmpMatrix.ClearTranlate();

        // Transform the acceleration vector
        tmpMatrix.Transform( acceleration, CPoint(1,0,0) );

        // Incorporate the acceleration into the velocity
        velocity = velocity + (acceleration * ACCELERATION * elapsedTime);

        // Get the magnitude of the velocity
        float velocityMag = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        // If the magnitude is greater than our max velocity, we restrict it to that max velocity
        if( velocityMag > MAX_VELOCITY )
        {
            velocity.Normalize();
            velocity *= MAX_VELOCITY;
        }

        // Increment the ship's position
        pActor->IncPos( velocity * elapsedTime );
    }
    else
    {
        // Get the magnitude of the velocity
        float velocityMag = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        // If the magnitude is greater than the min velocity, we want to deccelerate until we've stopped
        if( velocityMag > MIN_VELOCITY )
        {
            CPoint tmp = velocity;

            // We set the acceleration vector to the velocity vector because we want to deccelerate in the opposite
            // direction of our velocity vector
            acceleration = velocity;
            acceleration.Normalize();

            // Incorporate the decceleration into the velocity
            if( CGameController::Instance().WasAction("Deccelerate") == NDevice::EAP_HOLD )
                velocity = velocity + (acceleration * -BRAKE_DECCELERATION * elapsedTime);
            else
                velocity = velocity + (acceleration * -DECCELERATION * elapsedTime);

            // Once the signs on the x and y component of the velocity vector cross zero, we set them to zero
            tmp *= velocity;

            if( tmp.x < 0 )
                velocity.x = 0;

            if( tmp.y < 0 )
                velocity.y = 0;
            
            // Increment the ship's position
            pActor->IncPos( velocity * elapsedTime );
        }
        // If the magnitude is less than or equal to our min velocity, we set the velocity to zero
        else
            velocity.Clear();
    }

}	// HandleMovement */


/************************************************************************
*    desc:  Handle the ship's rotation 
************************************************************************/
void CPlayerShipAI::HandleRotation()
{
    // If the ship is turning, we handle it here
    if( rotating )
    {
        float accelDirection, rotDiff, rotDiffA, rotDiffB;

        // Get the difference between the ship's current rotation and where it wants to be
        rotDiffA = rotDestination - pActor->GetRot().z;

        // Set the rotDiffB to have the opposite sign of rotDiffA, but have the same rotation position
        if( rotDiffA > 0 )
            rotDiffB = rotDiffA - 360.f;
        else
            rotDiffB = rotDiffA + 360.f;

        // Find the shortest direction to rotate in
        if( abs(rotDiffA) < abs(rotDiffB) )
            rotDiff = rotDiffA;
        else
            rotDiff = rotDiffB;
        
        if( rotDiff > 0 )
            accelDirection = 1;
        else
            accelDirection = -1;
        
        // Calculate the angularVelocity
        angularVelocity = angularVelocity + (accelDirection * ANGULAR_ACCELERATION * elapsedTime);

        // We calculate the peak velocity. The peak velocity will deccelerate us if we're nearing the angle we want
        float peakVelocity = sqrt( 2 * ANGULAR_ACCELERATION * abs(rotDiff) ) * 0.5f;

        if( (angularVelocity > 0 && angularVelocity > peakVelocity) ||
            (angularVelocity < 0 && angularVelocity < -peakVelocity) )
            angularVelocity = peakVelocity * accelDirection;

        // We don't want to accelerate past our maximum velocity
        if( abs(angularVelocity) > MAX_ANGULAR_VELOCITY )
            angularVelocity = MAX_ANGULAR_VELOCITY * accelDirection;

        if( abs( rotDiff ) < abs(angularVelocity * elapsedTime) )
        {
            pActor->SetRot( CPoint(0,0,rotDestination) );
            angularVelocity = 0;
        }
        else
            pActor->IncRot( CPoint(0,0,angularVelocity * elapsedTime) );
    }
    // If there's no movement, we want to begin deccelerating
    else
    {
        if( angularVelocity > MIN_ANGULAR_VELOCITY )
        {
            angularVelocity = angularVelocity + (-ANGULAR_ACCELERATION * elapsedTime);

            if( angularVelocity * elapsedTime < 0 )
                angularVelocity = 0;
            else
                pActor->IncRot( CPoint(0,0,angularVelocity * elapsedTime) );
        }
        else if( angularVelocity < -MIN_ANGULAR_VELOCITY )
        {
            angularVelocity = angularVelocity + (ANGULAR_ACCELERATION * elapsedTime);

            if( angularVelocity * elapsedTime > 0 )
                angularVelocity = 0;
            else
                pActor->IncRot( CPoint(0,0,angularVelocity * elapsedTime) );
        }
        else
            angularVelocity = 0;

        //pActor->IncRot( CPoint(0,0,angularVelocity * elapsedTime) );
    }

}	// HandleRotation */


/************************************************************************
*    desc:  Handle the ship's shooting 
************************************************************************/
void CPlayerShipAI::HandleShooting()
{
    // Set the rotation of the gun
    pGun->SetRot( CPoint(0,0,gunRotation - pActor->GetRot().z) );

    // If the player hits the shoot button, and our time has expired, create a projectile actor
    if( playerShootTimer.Expired() && shooting )
    {
        // Set the projectile's offset
        CPoint projectileOffset;
        float tmpRot = gunRotation * DEG_TO_RAD;
        projectileOffset.x = 30 * cos( tmpRot );
        projectileOffset.y = 30 * sin( tmpRot );
        projectileOffset.z = 0;
        
        // Create the projectile actor
        CActorSprite2D * pProjectile = CActorManager::Instance().CreateActorPtr2D(
            "player_projectile", pActor->GetPos() + projectileOffset, CPoint(0,0,gunRotation) );

        // Add the projectile actor to the actor manager
        CActorManager::Instance().AddActorToVec2D( "player_projectiles", pProjectile );

        // Get the projectile's AI and initialize it with the ship's velocity
        CPlayerProjectileAI * pBulletAI = NGenFunc::DynCast<CPlayerProjectileAI*>( pProjectile->GetAIPtr() );
        pBulletAI->Init( velocity );

        // Add the bullet to the actor instance mesh
        CInstanceMeshManager::Instance().InitInstanceSprite( "(actors)", pProjectile );

        playerShootTimer.Reset();
    }

}	// HandleShooting */