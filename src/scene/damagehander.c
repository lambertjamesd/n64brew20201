#include "damagehandler.h"
#include "util/time.h"
#include "math/mathf.h"

#define INVINCIBLE_FLASH_FREQ                      0.1f

void damageHandlerInit(struct DamageHandler* handler, float hp) {
    handler->damageTimer = 0.0f;
    handler->damageTimerAmount = 0.0f;
    handler->hp = hp;
}

int damageHandlerApplyDamage(struct DamageHandler* handler, float amount, float invincibleTime) {
    if ((handler->damageTimer <= 0.0f || amount > handler->damageTimerAmount)) {
        handler->damageTimer = invincibleTime;
        handler->hp -= (amount - handler->damageTimerAmount);
        handler->damageTimerAmount = amount;
        return 1;
    }
    return 0;
}

void damageHandlerUpdate(struct DamageHandler* handler) {
    if (handler->damageTimer > 0.0f) {
        handler->damageTimer -= gTimeDelta;

        if (handler->damageTimer < 0.0f) {
            handler->damageTimer = 0.0f;
            handler->damageTimerAmount = 0.0f;
        }
    }
}

int damageHandlerIsFlashing(struct DamageHandler* handler) {
    return handler->damageTimer > 0.0f && mathfMod(handler->damageTimer, INVINCIBLE_FLASH_FREQ) > (INVINCIBLE_FLASH_FREQ * 0.5f);
}