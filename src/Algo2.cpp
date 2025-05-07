#include "Algo2.h"

ActionRequest Algo2::getAction() {
    if (!currentInfo) {
        return ActionRequest::GetBattleInfo;
    }

    if (moveAfterRotate) {
        moveAfterRotate = false;
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) {
        return moveIfThreatened().getType();
    }
    if (canShootInDirection()) {
        return ActionRequest::Shoot;
    }

    return ActionRequest::GetBattleInfo;
}
