#pragma once

//** From ACE: https://github.com/ACEmulator/ACE

namespace Skill {
    enum Skill {
        None = 0,
        Axe,                 /* Retired */
        Bow,                 /* Retired */
        Crossbow,            /* Retired */
        Dagger,              /* Retired */
        Mace,                /* Retired */
        MeleeDefense,
        MissileDefense,
        Sling,               /* Retired */
        Spear,               /* Retired */
        Staff,               /* Retired */
        Sword,               /* Retired */
        ThrownWeapon,        /* Retired */
        UnarmedCombat,       /* Retired */
        ArcaneLore,
        MagicDefense,
        ManaConversion,
        Spellcraft,          /* Unimplemented */
        ItemTinkering,
        AssessPerson,
        Deception,
        Healing,
        Jump,
        Lockpick,
        Run,
        Awareness,           /* Unimplemented */
        ArmsAndArmorRepair,  /* Unimplemented */
        AssessCreature,
        WeaponTinkering,
        ArmorTinkering,
        MagicItemTinkering,
        CreatureEnchantment,
        ItemEnchantment,
        LifeMagic,
        WarMagic,
        Leadership,
        Loyalty,
        Fletching,
        Alchemy,
        Cooking,
        Salvaging,
        TwoHandedCombat,
        Gearcraft,           /* Retired */
        VoidMagic,
        HeavyWeapons,
        LightWeapons,
        FinesseWeapons,
        MissileWeapons,
        Shield,
        DualWield,
        Recklessness,
        SneakAttack,
        DirtyFighting,
        Challenge,          /* Unimplemented */
        Summoning
    };
};