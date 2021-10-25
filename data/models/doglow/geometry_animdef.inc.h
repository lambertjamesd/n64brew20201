extern unsigned short doglow_Armature_001_Idle[];
extern unsigned short doglow_Armature_001_Jump[];
extern unsigned short doglow_Armature_001_JumpLand[];
extern unsigned short doglow_Armature_001_JumpLand_002[];
extern unsigned short doglow_Armature_001_Punch_001[];
extern unsigned short doglow_Armature_001_Punch_002[];
extern unsigned short doglow_Armature_001_T_pose[];
struct SKAnimationHeader doglow_animations[] = {
    {480, 30, 58, 0, (struct SKAnimationChunk*)doglow_Armature_001_Idle, 0},
    {464, 30, 2, 0, (struct SKAnimationChunk*)doglow_Armature_001_Jump, 0},
    {496, 30, 8, 0, (struct SKAnimationChunk*)doglow_Armature_001_JumpLand, 0},
    {496, 30, 8, 0, (struct SKAnimationChunk*)doglow_Armature_001_JumpLand_002, 0},
    {480, 30, 18, ATTACK_001_EVENT_COUNT, (struct SKAnimationChunk*)doglow_Armature_001_Punch_001, gAttack001Events},
    {480, 30, 18, 0, (struct SKAnimationChunk*)doglow_Armature_001_Punch_002, 0},
    {496, 30, 123, 0, (struct SKAnimationChunk*)doglow_Armature_001_T_pose, 0},
};
