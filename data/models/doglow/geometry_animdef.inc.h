extern unsigned short doglow_Armature_001_Idle[];
extern unsigned short doglow_Armature_001_Jump[];
extern unsigned short doglow_Armature_001_Punch_001[];
extern unsigned short doglow_Armature_001_Punch_002[];
struct SKAnimationHeader doglow_animations[] = {
    {480, 30, 58, 0, (struct SKAnimationChunk*)doglow_Armature_001_Idle, 0},
    {496, 30, 24, 0, (struct SKAnimationChunk*)doglow_Armature_001_Jump, 0},
    {496, 30, 79, 0, (struct SKAnimationChunk*)doglow_Armature_001_Punch_001, 0},
    {480, 30, 18, 0, (struct SKAnimationChunk*)doglow_Armature_001_Punch_002, 0},
};
