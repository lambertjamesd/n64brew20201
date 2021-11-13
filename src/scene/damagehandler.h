#ifndef _DAMAGE_HANDLER_H
#define _DAMAGE_HANDLER_H

struct DamageHandler {
    float hp;
    float damageTimer;
    float damageTimerAmount;
};

void damageHandlerInit(struct DamageHandler* handler, float hp);
int damageHandlerApplyDamage(struct DamageHandler* handler, float amount, float invincibleTime);
void damageHandlerUpdate(struct DamageHandler* handler);
int damageHandlerIsFlashing(struct DamageHandler* handler);

#endif