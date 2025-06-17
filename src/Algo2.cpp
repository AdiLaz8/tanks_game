#include "Algo2.h"
#include "MyBattleInfo.h"
#include "Logger.h"

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
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": First turn, requesting battle info.");
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
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Moved forward after rotation to avoid shell threat that was detected the turn before.");
        return ActionRequest::MoveForward;
    }
    if (isThreatenedByShells()) { // if the tank is threatened by shells, try to escape
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Shell threat detected, initiating evasive move.");
        return moveIfThreatened().getType();
    }
    if (canShootInDirection()&& ammo>0 &&shootingStatus==0) { //if enemy in sight --> shoot
        shootingStatus=5;
        ammo--;
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Enemy in line of sight, firing.");
        return ActionRequest::Shoot;
    }
    Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": No action available, requesting updated battle info.");
    return ActionRequest::GetBattleInfo;
}