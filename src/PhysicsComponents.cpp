#include "stdafx.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"

PhysicsComponent::PhysicsComponent(b2World* physicsWorld)
    : mHeight()
    , mOnTheGround()
    , mPhysicsWorld(physicsWorld)
    , mPhysicsBody()
{
    debug_assert(physicsWorld);
}

PhysicsComponent::~PhysicsComponent()
{
    DetachAllComponents();
}

void PhysicsComponent::AttachComponent(PhysicsComponent* component, const glm::vec2& localAnchor)
{
    if (component == this)
    {
        debug_assert(false);
        return;
    }

    // test is already attached
    for (const PhysicsConnection& curr: mConnections)
    {
        if (curr.mPhysicsComponent == component)
        {
            debug_assert(false);
            return;
        }
    }

    b2WeldJointDef jointDef;
    b2Vec2 anchorPoint { localAnchor.x * PHYSICS_SCALE, localAnchor.y * PHYSICS_SCALE };
    jointDef.Initialize(mPhysicsBody, component->mPhysicsBody, anchorPoint);

    PhysicsConnection connectionInfo;
    connectionInfo.mJoint = mPhysicsWorld->CreateJoint(&jointDef);
    debug_assert(connectionInfo.mJoint);
    connectionInfo.mPhysicsComponent = component;
    mConnections.push_back(connectionInfo);
}

void PhysicsComponent::DetachComponent(PhysicsComponent* component)
{
    b2Joint* connection = nullptr;
    for (auto itercurr = mConnections.begin(); itercurr != mConnections.end(); ++itercurr)
    {
        if (itercurr->mPhysicsComponent == component)
        {
            connection = itercurr->mJoint;
            mConnections.erase(itercurr);
            break;
        }
    }
    if (connection)
    {
        mPhysicsWorld->DestroyJoint(connection);
    }
}

void PhysicsComponent::DetachAllComponents()
{
    for (const PhysicsConnection& curr: mConnections)
    {
        mPhysicsWorld->DestroyJoint(curr.mJoint);
    }
    mConnections.clear();
}

void PhysicsComponent::SetPosition(const glm::vec3& position)
{
    mHeight = position.y;

    b2Vec2 b2position { position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE };
    mPhysicsBody->SetTransform(b2position, mPhysicsBody->GetAngle());
}

void PhysicsComponent::SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    mHeight = position.y;

    b2Vec2 b2position { position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE };
    mPhysicsBody->SetTransform(b2position, rotationAngle.to_radians());
}

void PhysicsComponent::SetRotationAngle(cxx::angle_t rotationAngle)
{
    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), rotationAngle.to_radians());
}

cxx::angle_t PhysicsComponent::GetRotationAngle() const
{
    cxx::angle_t rotationAngle = cxx::angle_t::from_radians(mPhysicsBody->GetAngle());
    rotationAngle.normalize_angle_180();
    return rotationAngle;
}

void PhysicsComponent::AddForce(const glm::vec2& force)
{
    b2Vec2 b2Force { force.x * PHYSICS_SCALE, force.y * PHYSICS_SCALE };
    mPhysicsBody->ApplyForceToCenter(b2Force, true);
}

void PhysicsComponent::AddLinearImpulse(const glm::vec2& impulse)
{
    b2Vec2 b2Impulse { impulse.x * PHYSICS_SCALE, impulse.y * PHYSICS_SCALE };
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
}

glm::vec3 PhysicsComponent::GetPosition() const
{
    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x / PHYSICS_SCALE, mHeight, b2position.y / PHYSICS_SCALE };
}

glm::vec2 PhysicsComponent::GetLinearVelocity() const
{
    const b2Vec2& b2position = mPhysicsBody->GetLinearVelocity();
    return { b2position.x / PHYSICS_SCALE, b2position.y / PHYSICS_SCALE };
}

float PhysicsComponent::GetAngularVelocity() const
{
    float angularVelocity = glm::degrees(mPhysicsBody->GetAngularVelocity());
    return angularVelocity;
}

void PhysicsComponent::AddAngularImpulse(float impulse)
{
    mPhysicsBody->ApplyAngularImpulse(impulse, true);
}

void PhysicsComponent::SetAngularVelocity(float angularVelocity)
{
    mPhysicsBody->SetAngularVelocity(glm::radians(angularVelocity));
}

void PhysicsComponent::SetLinearVelocity(const glm::vec2& velocity)
{
    b2Vec2 b2vec { velocity.x * PHYSICS_SCALE, velocity.y * PHYSICS_SCALE };
    mPhysicsBody->SetLinearVelocity(b2vec);
}

void PhysicsComponent::ClearForces()
{
    b2Vec2 nullVector { 0.0f, 0.0f };
    mPhysicsBody->SetLinearVelocity(nullVector);
    mPhysicsBody->SetAngularVelocity(0.0f);
}

glm::vec2 PhysicsComponent::GetSignVector() const
{
    float angleRadians = mPhysicsBody->GetAngle();
    glm::vec2 signVector 
    {
        cos(angleRadians), sin(angleRadians)
    };
    return signVector;
}

glm::vec2 PhysicsComponent::GetWorldPoint(const glm::vec2& localPosition) const
{
    b2Vec2 b2LocalPosition { localPosition.x * PHYSICS_SCALE, localPosition.y * PHYSICS_SCALE };
    b2Vec2 b2WorldPosition = mPhysicsBody->GetWorldPoint(b2LocalPosition);

    return glm::vec2 { b2WorldPosition.x / PHYSICS_SCALE, b2WorldPosition.y / PHYSICS_SCALE };
}

glm::vec2 PhysicsComponent::GetLocalPoint(const glm::vec2& worldPosition) const
{
    b2Vec2 b2WorldPosition { worldPosition.x * PHYSICS_SCALE, worldPosition.y * PHYSICS_SCALE };
    b2Vec2 b2LocalPosition = mPhysicsBody->GetWorldPoint(b2WorldPosition);

    return glm::vec2 { b2LocalPosition.x / PHYSICS_SCALE, b2LocalPosition.y / PHYSICS_SCALE };
}

//////////////////////////////////////////////////////////////////////////

PedPhysicsComponent::PedPhysicsComponent(b2World* physicsWorld, const glm::vec3& startPosition, cxx::angle_t startRotation)
    : PhysicsComponent(physicsWorld)
{
    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.userData = this;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = PHYSICS_PED_BOUNDING_SPHERE_RADIUS * PHYSICS_SCALE;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 0.3f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    // create sensor
    shapeDef.m_radius = PHYSICS_PED_SENSOR_SPHERE_RADIUS * PHYSICS_SCALE;
    fixtureDef.shape = &shapeDef;
    fixtureDef.isSensor = true;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED_SENSOR;
    fixtureDef.filter.maskBits = PHYSICS_OBJCAT_PED | PHYSICS_OBJCAT_CAR;

    b2Fixture* b2sensorFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    SetPosition(startPosition, startRotation);
}

PedPhysicsComponent::~PedPhysicsComponent()
{
}

void PedPhysicsComponent::UpdateFrame(Timespan deltaTime)
{

}

void PedPhysicsComponent::SetFalling(bool isFalling)
{
    if (isFalling == mFalling)
        return;

    mFalling = isFalling;
    if (mFalling)
    {
        b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
        ClearForces();
        velocity.Normalize();
        mPhysicsBody->SetLinearVelocity(velocity);
    }
}

void PedPhysicsComponent::HandleCarContactBegin()
{
    ++mContactingCars;
}

void PedPhysicsComponent::HandleCarContactEnd()
{
    --mContactingCars;

    if (mContactingCars < 0)
    {
        debug_assert(false);
        mContactingCars = 0;
    }
}

void PedPhysicsComponent::HandleCarEnter()
{
    mPhysicsBody->SetFixedRotation(false);
}

void PedPhysicsComponent::HandleCarLeave()
{
    mPhysicsBody->SetFixedRotation(true);
}

bool PedPhysicsComponent::ShouldCollideWith(unsigned int bits) const
{
    debug_assert(bits);

    ePedestrianState currState = mReferencePed->GetCurrentStateID();
    if (currState == ePedestrianState_Falling || currState == ePedestrianState_SlideOnCar)
    {
        return (bits & (PHYSICS_OBJCAT_MAP_SOLID_BLOCK | PHYSICS_OBJCAT_WALL)) > 0;
    }

    if (currState == ePedestrianState_EnteringCar || currState == ePedestrianState_ExitingCar || 
        currState == ePedestrianState_DrivingCar)
    {
        return false;
    }

    if (currState == ePedestrianState_Dying || currState == ePedestrianState_Dead)
    {
        return false;
    }

    if (currState == ePedestrianState_KnockedDown)
    {
        return (bits & PHYSICS_OBJCAT_PED) == 0; // collide to all except for other peds
    }

    return true;    
}

//////////////////////////////////////////////////////////////////////////

CarPhysicsComponent::CarPhysicsComponent(b2World* physicsWorld, CarStyle* desc, const glm::vec3& startPosition, cxx::angle_t startRotation)
    : PhysicsComponent(physicsWorld)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;
    //bodyDef.linearDamping = 0.15f;
    bodyDef.angularDamping = 2.0f;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    //physicsObject->mDepth = (1.0f * desc->mDepth) / MAP_PIXELS_PER_TILE;
    
    float shape_size_w = ((1.0f * desc->mWidth) / MAP_PIXELS_PER_TILE) * 0.5f * PHYSICS_SCALE;
    float shape_size_h = ((1.0f * desc->mHeight) / MAP_PIXELS_PER_TILE) * 0.5f * PHYSICS_SCALE;

    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(shape_size_h, shape_size_w); // swap h and w

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 0.1f;
    //fixtureDef.friction = 0.0f;
    //fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    mChassisFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(mChassisFixture);

    SetPosition(startPosition, startRotation);

    SetupWheels(desc);
}

CarPhysicsComponent::~CarPhysicsComponent()
{
    FreeWheels();
}

void CarPhysicsComponent::UpdateFrame(Timespan deltaTime)
{

}

void CarPhysicsComponent::GetChassisCorners(glm::vec2 corners[4]) const
{
    const b2PolygonShape* shape = (const b2PolygonShape*) mChassisFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        b2Vec2 point = mPhysicsBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner].x = point.x / PHYSICS_SCALE;
        corners[icorner].y = point.y / PHYSICS_SCALE;
    }
}

void CarPhysicsComponent::GetWheelCorners(eCarWheelID wheelID, glm::vec2 corners[4]) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mPhysicsBody == nullptr)
    {
        debug_assert(false);
        return;
    }

    const b2PolygonShape* shape = (const b2PolygonShape*) wheel.mFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        b2Vec2 point = wheel.mPhysicsBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner].x = point.x / PHYSICS_SCALE;
        corners[icorner].y = point.y / PHYSICS_SCALE;
    }
}

bool CarPhysicsComponent::HasWheel(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    return mCarWheels[wheelID].mPhysicsBody != nullptr;
}

glm::vec2 CarPhysicsComponent::GetWheelLateralVelocity(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    glm::vec2 result_velocity;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mPhysicsBody)
    {
        b2Vec2 literal_vel = GetWheelLateralVelocity(wheel.mPhysicsBody);
        result_velocity.x = literal_vel.x / PHYSICS_SCALE;
        result_velocity.y = literal_vel.y / PHYSICS_SCALE;
    }
    else
    {
        debug_assert(false);
    }
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelForwardVelocity(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    glm::vec2 result_velocity;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mPhysicsBody)
    {
        b2Vec2 literal_vel = GetWheelForwardVelocity(wheel.mPhysicsBody);
        result_velocity.x = literal_vel.x / PHYSICS_SCALE;
        result_velocity.y = literal_vel.y / PHYSICS_SCALE;
    }
    else
    {
        debug_assert(false);
    }
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelPosition(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    glm::vec2 position;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mPhysicsBody)
    {
        b2Vec2 world_center = wheel.mPhysicsBody->GetWorldCenter();
        position.x = world_center.x / PHYSICS_SCALE;
        position.y = world_center.y / PHYSICS_SCALE;
    }
    else
    {
        debug_assert(false);
    }
    return position;
}

void CarPhysicsComponent::SetupWheels(CarStyle* desc)
{
    CreateWheel(desc, eCarWheelID_Steering);
    CreateWheel(desc, eCarWheelID_Drive);

    // setup joints
    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0.0f;
    jointDef.upperAngle = 0.0f;

    // front
	jointDef.Initialize(mPhysicsBody, mCarWheels[eCarWheelID_Steering].mPhysicsBody, mPhysicsBody->GetPosition());
	mFrontWheelJoint = (b2RevoluteJoint*)mPhysicsWorld->CreateJoint(&jointDef);

    // rear
	jointDef.Initialize(mPhysicsBody, mCarWheels[eCarWheelID_Drive].mPhysicsBody, mPhysicsBody->GetPosition());
	mRearWheelJoint = (b2RevoluteJoint*)mPhysicsWorld->CreateJoint(&jointDef);

    mPhysicsBody->ResetMassData();
}

void CarPhysicsComponent::FreeWheels()
{
    // destroy joints
    if (mFrontWheelJoint)
    {
        mPhysicsWorld->DestroyJoint(mFrontWheelJoint);
        mFrontWheelJoint = nullptr;
    }

    if (mRearWheelJoint)
    {
        mPhysicsWorld->DestroyJoint(mRearWheelJoint);
        mRearWheelJoint = nullptr;
    }

    for (WheelData& currWheel: mCarWheels)
    {
        if (currWheel.mPhysicsBody == nullptr)
            continue;

        mPhysicsWorld->DestroyBody(currWheel.mPhysicsBody);

        currWheel.mPhysicsBody = nullptr;
        currWheel.mFixture = nullptr;
    }
}

void CarPhysicsComponent::CreateWheel(CarStyle* desc, eCarWheelID wheelID)
{
    WheelData& wheel = mCarWheels[wheelID];

    debug_assert(wheel.mPhysicsBody == nullptr && wheel.mFixture == nullptr);

    const int wheel_pixels_w = 6;
    const int wheel_pixels_h = 12;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;
    bodyDef.position = mPhysicsBody->GetPosition();

    // fix position
    if (wheelID == eCarWheelID_Steering)
    {
        bodyDef.position.x += ((1.0f * desc->mSteeringWheelOffset) / MAP_PIXELS_PER_TILE) * PHYSICS_SCALE;
    }
    else if (wheelID == eCarWheelID_Drive)
    {
        bodyDef.position.x += ((1.0f * desc->mDriveWheelOffset) / MAP_PIXELS_PER_TILE) * PHYSICS_SCALE;
    }
    else
    {
        debug_assert(false);
    }

    wheel.mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(wheel.mPhysicsBody);

    float wheel_size_w = ((1.0f * wheel_pixels_w) / MAP_PIXELS_PER_TILE) * 0.5f * PHYSICS_SCALE;
    float wheel_size_h = ((1.0f * wheel_pixels_h) / MAP_PIXELS_PER_TILE) * 0.5f * PHYSICS_SCALE;
    
    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(wheel_size_h, wheel_size_w); // swap h and w

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 1.0f;
    fixtureDef.filter.categoryBits = 0; // no collisions

    wheel.mFixture = wheel.mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(wheel.mFixture);
}

b2Vec2 CarPhysicsComponent::GetWheelLateralVelocity(b2Body* carWheel) const
{
	const b2Vec2 right_normal = carWheel->GetWorldVector(b2Vec2(1.0f, 0.0f));
    return b2Dot(right_normal, carWheel->GetLinearVelocity()) * right_normal;
}

b2Vec2 CarPhysicsComponent::GetWheelForwardVelocity(b2Body* carWheel) const
{
	const b2Vec2 forward_normal = carWheel->GetWorldVector(b2Vec2(0.0f, 1.0f));
    return b2Dot(forward_normal, carWheel->GetLinearVelocity()) * forward_normal;
}