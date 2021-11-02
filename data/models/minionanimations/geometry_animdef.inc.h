extern unsigned short minion_animations_Armature_Attack[];
extern unsigned short minion_animations_Armature_Die[];
extern unsigned short minion_animations_Armature_Idle[];
extern unsigned short minion_animations_Armature_MoveForward[];
struct SKAnimationHeader minion_animations_animations[] = {
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Armature_Attack, 0},
    {80, 30, 22, 0, (struct SKAnimationChunk*)minion_animations_Armature_Die, 0},
    {64, 30, 13, 0, (struct SKAnimationChunk*)minion_animations_Armature_Idle, 0},
    {64, 30, 9, 0, (struct SKAnimationChunk*)minion_animations_Armature_MoveForward, 0},
};
unsigned short minion_animations_bone_parent[] = {
    0xFFFF,
};
