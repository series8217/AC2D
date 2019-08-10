#pragma once

//** From ACE: https://github.com/ACEmulator/ACE

namespace Physics {
    namespace EncumbranceSystem {
        int EncumbranceCapacity(int strength, int numAugs);
        float GetBurden(int capacity, int encumbrance);
        float GetBurdenMod(float burden);
    }

    namespace MovementSystem {
        float GetJumpHeight(float burden, unsigned int jumpSkill, float power, float scaling);
        double GetRunRate(float burden, int runSkill, float scaling);
        int JumpStaminaCost(float power, float burden, bool pk);
        float GetJumpPower(unsigned int stamina, float burden, bool pk);
    }
}