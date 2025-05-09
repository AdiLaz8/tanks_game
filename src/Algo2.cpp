#include "Algo2.h"
#include "MyBattleInfo.h"
void Algo2::updateBattleInfo(BattleInfo& info) {
    auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
    if(turnCounterSinceInfo==-1){
        boardWidth = myInfo.getWidth();
        boardHeight = myInfo.getHeight();
        ammo = myInfo.getInitialShells();
    }
    turnCounterSinceInfo = 1;
    fullView = myInfo.getFullView();
    Position self = myInfo.getSelfPosition();
}
ActionRequest Algo2::getAction() {
    if (turnCounterSinceInfo == -1) {
        return ActionRequest::GetBattleInfo;
    }
    if(getShootingStatus()>0){
        shootingStatus--;
    }

    if (moveAfterRotate) {
        moveAfterRotate = false;
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) {
        return moveIfThreatened().getType();
    }
    if (canShootInDirection()&& ammo>0 &&shootingStatus==0) {
        shootingStatus=5;
        ammo--;
        return ActionRequest::Shoot;
    }

    return ActionRequest::GetBattleInfo;
}
