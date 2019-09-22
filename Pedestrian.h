#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"

// defines pedestrian control interface
class PedestrianControl final
{
public:
    PedestrianControl(Pedestrian& pedestrian);
    void ResetControl();
    void SetTurnLeft(bool turnEnabled);
    void SetTurnRight(bool turnEnabled);
    void SetTurnAngle(float turnAngle);
    bool IsTurnAround() const;
    void SetWalkForward(bool walkEnabled);
    void SetWalkBackward(bool walkEnabled);
    void SetRunning(bool runEnabled);
    bool IsMoves() const;
public:
    Pedestrian& mPedestrian;
    float mTurnAngle; // specified in degrees
    bool mTurnLeft;
    bool mTurnRight;
    bool mWalkForward;
    bool mWalkBackward;
    bool mRunning;
};

//////////////////////////////////////////////////////////////////////////

// defines generic city pedestrian
class Pedestrian final: public cxx::noncopyable
{
public:
    PedestrianControl mControl; // control pedestrian actions

    // public for convenience, should not be modified directly
    const unsigned int mID; // unique identifier

    PhysicsObject* mPhysicalBody;
    bool mDead;
    bool mMarkForDeletion;
    Timespan mLiveTicks; // time since spawn

    eSpriteAnimationID mCurrentAnimID;
    SpriteAnimation mAnimation;

public:
    Pedestrian(unsigned int id);
    ~Pedestrian();

    // setup initial state when spawned on level
    void EnterTheGame();

    // state control
    void SetHeading(float rotationDegrees);
    void SetPosition(const glm::vec3& position);

    // process current animation and logic
    void UpdateFrame(Timespan deltaTime);

    // change current animation
    void SwitchToAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode);

    bool IsFalling() const;

private:
    friend class PedestrianManager;

    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<Pedestrian> mActivePedsNode;
    cxx::intrusive_node<Pedestrian> mDeletePedsNode;
};

//////////////////////////////////////////////////////////////////////////

// defines peds manager class
class PedestrianManager final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    cxx::intrusive_list<Pedestrian> mActivePedestriansList;
    cxx::intrusive_list<Pedestrian> mDeletePedestriansList;

public:
    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    // add random pedestrian to map at specific location
    // @param position: Real world position
    Pedestrian* CreateRandomPed(const glm::vec3& position);

    // will immediately destroy pedestrian object, make sure it is not in use at this moment
    // @param ped: Pedestrian
    void DestroyPedestrian(Pedestrian* ped);

private:
    void DestroyPedsInList(cxx::intrusive_list<Pedestrian>& pedsList);
    void DestroyPendingPeds();
    void AddToActiveList(Pedestrian* ped);
    void RemoveFromActiveList(Pedestrian* ped);

    unsigned int GenerateUniqueID();

private:
    unsigned int mIDsCounter;

    cxx::object_pool<Pedestrian> mPedsPool;
};