#ifndef _TEAM_ENTITY_H
#define _TEAM_ENTITY_H

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