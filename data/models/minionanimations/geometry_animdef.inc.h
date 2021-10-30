extern unsigned short minion_animations_Idle[];
extern unsigned short minion_animations_MoveForward[];
extern unsigned short minion_animations_MoveForward_002[];
extern unsigned short minion_animations_Attack_001[];
extern unsigned short minion_animations_Attack_002[];
extern unsigned short minion_animations_Die_001[];
extern unsigned short minion_animations_Armature_Attack[];
extern unsigned short minion_animations_Armature_Die[];
extern unsigned short minion_animations_Armature_Idle[];
extern unsigned short minion_animations_Armature_MoveForward[];
struct SKAnimationHeader minion_animations_animations[] = {
    {64, 30, 13, 0, (struct SKAnimationChunk*)minion_animations_Idle, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_MoveForward, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_MoveForward_002, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Attack_001, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Attack_002, 0},
    {64, 30, 28, 0, (struct SKAnimationChunk*)minion_animations_Die_001, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Armature_Attack, 0},
    {64, 30, 32, 0, (struct SKAnimationChunk*)minion_animations_Armature_Die, 0},
    {64, 30, 13, 0, (struct SKAnimationChunk*)minion_animations_Armature_Idle, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Armature_MoveForward, 0},
};
unsigned short minion_animations_bone_parent[] = {
    0xFFFF,
};
