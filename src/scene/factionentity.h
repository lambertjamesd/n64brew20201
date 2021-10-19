#ifndef _FACTION_ENTITY_H
#define _FACTION_ENTITY_H

enum FactionEntityType {
    FactionEntityTypeMinion,
    FactionEntityTypeBase,
    FactionEntityTypePlayer,
};

struct FactionEntity {
    unsigned short entityType;
    unsigned short entityFaction;
};

#endif