#include "stdafx.h"
#include "cPhysics.h"
#include "math.h"



//** From ACE: https://github.com/ACEmulator/ACE

namespace Physics {

    namespace EncumbranceSystem
    {
        int EncumbranceCapacity(int strength, int numAugs)
        {
            if (strength <= 0) return 0;

            float bonusBurden = 30 * numAugs;

            if (bonusBurden >= 0)
            {
                if (bonusBurden > 150)
                    bonusBurden = 150;

                return 150 * strength + strength * bonusBurden;
            }
            else
                return 150 * strength;
        }

        float GetBurden(int capacity, int encumbrance)
        {
            if (capacity <= 0) return 3.0f;

            if (encumbrance >= 0)
                return (float)encumbrance / capacity;
            else
                return 0.0f;
        }

        float GetBurdenMod(float burden)
        {
            if (burden < 1.0f) return 1.0f;

            if (burden < 2.0f)
                return 2.0f - burden;
            else
                return 0.0f;
        }
    }


    namespace MovementSystem
    {
        float GetJumpHeight(float burden, unsigned int jumpSkill, float power, float scaling)
        {
            if (power < 0.0f) power = 0.0f;
            if (power > 1.0f) power = 1.0f;

            float result = EncumbranceSystem::GetBurdenMod(burden) * (jumpSkill / (jumpSkill + 1300.0f) * 22.2f + 0.05f) * power / scaling;

            if (result < 0.35f)
                result = 0.35f;

            return result;
        }

        double GetRunRate(float burden, int runSkill, float scaling)
        {
            float loadMod = EncumbranceSystem::GetBurdenMod(burden);

            if (runSkill >= 800.0f)     // max run speed?
                return 18.0f / 4.0f;
            else
                return ((loadMod * ((float)runSkill / (runSkill + 200) * 11) + 4) / scaling) / 4.0f;
        }

        int JumpStaminaCost(float power, float burden, bool pk)
        {
            if (pk)
                return (int)((power + 1.0f) * 100.0f);
            else
                return (int)ceil((burden + 0.5f) * power * 8.0f + 2.0f);
        }

        float GetJumpPower(unsigned int stamina, float burden, bool pk)
        {
            if (pk)
                return stamina / 100.0f - 1.0f;
            else
                return (stamina - 2.0f) / (burden * 8.0f + 4.0f);
        }
    }
}