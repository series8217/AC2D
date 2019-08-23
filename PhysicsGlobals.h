//** Ported from ACE: https://github.com/ACEmulator/ACE

#pragma once

namespace Physics
{
    namespace Globals
    {
        const float EPSILON = 0.00019999999f;

        const float EpsilonSq = EPSILON * EPSILON;

        const float Gravity = -9.8000002f;

        const float DefaultFriction = 0.94999999f;

        const float DefaultElasticity = 0.050000001f;

        const float DefaultTranslucency = 0.0f;

        const float DefaultMass = 1.0f;

        const float DefaultScale = 1.0f;

        // TODO: implement me
        //const PhysicsState DefaultState =
        //    PhysicsState.EdgeSlide | PhysicsState.LightingOn | PhysicsState.Gravity | PhysicsState.ReportCollisions;

        const float MaxElasticity = 0.1f;

        const float MaxVelocity = 50.0f;

        const float MaxVelocitySquared = MaxVelocity * MaxVelocity;

        const float SmallVelocity = 0.25f;

        const float SmallVelocitySquared = SmallVelocity * SmallVelocity;

        const float MinQuantum = 1.0f / 30.0f;     // 0.0333... 30fps

        //const float MaxQuantum = 0.2f;     // 5fps   // this is buggy with MoveToManager turning
        const float MaxQuantum = 0.1f;     // 10fps

        const float HugeQuantum = 2.0f;    // 0.5fps

        /// <summary>
        /// The walkable allowance when landing on the ground
        /// </summary>
        const float LandingZ = 0.0871557f;

        const float FloorZ = 0.66417414618662751f;

        const float DummySphereRadius = 0.1f;

        // TODO: implement me
        //const Sphere DummySphere = new Sphere(new Vector3(0, 0, DummySphereRadius), DummySphereRadius);

        // TODO: implement me
        //const Sphere DefaultSortingSphere;

        const float DefaultStepHeight = 0.0099999998f;
    }
}
