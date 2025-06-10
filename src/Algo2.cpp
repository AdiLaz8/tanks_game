#include "Algo2.h"
#include "MyBattleInfo.h"
void Algo2::updateBattleInfo(BattleInfo& info) {
    auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
    if(turnCounterSinceInfo==-1){ // first turn
        boardWidth = myInfo.getWidth();
        boardHeight = myInfo.getHeight();
        ammo = myInfo.getInitialShells();
    }
    turnCounterSinceInfo = 0;
    fullView = myInfo.getFullView();
    selfPosition = myInfo.getSelfPosition();
}

ActionRequest Algo2::getAction() {
    if (turnCounterSinceInfo == -1) { // first turn
        return ActionRequest::GetBattleInfo;
    }
    turnCounterSinceInfo++;

    if(getShootingStatus()>0){
        shootingStatus--;
    }
    //if the tank is threatened and rotated last turn so now it needs to move forward
    if (moveAfterRotate) {
        moveAfterRotate = false;
        Position newPos = selfPosition + direction.toVector();
        newPos.setx((newPos.getx() + boardWidth) % boardWidth);
        newPos.sety((newPos.gety() + boardHeight) % boardHeight);
        selfPosition = newPos;
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) { // if the tank is threatened by shells, try to escape
        return moveIfThreatened().getType();
    }
    
    //if enemy in sight --> shoot
    if (canShootInDirection()&& ammo>0 &&shootingStatus==0) {
        shootingStatus=5;
        ammo--;
        return ActionRequest::Shoot;
    }
    return ActionRequest::GetBattleInfo;
}