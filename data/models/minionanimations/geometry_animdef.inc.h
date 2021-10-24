extern unsigned short output_Armature_Attack[];
extern unsigned short output_Armature_Idle[];
struct SKAnimationHeader output_animations[] = {
    {64, 30, 26, 0, (struct SKAnimationChunk*)output_Armature_Attack, 0},
    {64, 30, 148, 0, (struct SKAnimationChunk*)output_Armature_Idle, 0},
};
