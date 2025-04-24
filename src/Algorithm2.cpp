#include "Algorithm2.h"
#include <iostream>
#include "Logger.h"

Action Algorithm2::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    if (isThreatenedByShells(board, self.getPosition())) {
        Logger::debug("Player 2: Threatened By Shells");
        return moveIfThreatened(board, self);
    }
    
    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0 && self.getAmmo() > 0) {
        Logger::debug("Player 2: Enemy in direct line of fire. Shooting now.");
        return Action(ActionType::Shoot);
    }
    if (enemy.getAmmo() == 0) {
        Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
        Direction::Value from = self.getDirection().getDirection();
        if (from == to && self.getAmmo() > 0) {
            if(self.getShootingStatus()==0){
                Logger::debug("Player 2: Trying to shoot towerds the enemy due to stuck state.");
                return Action(ActionType::Shoot);
            }
            else{
                return Action(ActionType::None);
            }
        }
        Logger::debug("Player 2: Trying to rotate towerds the enemy due to stuck state.");
        return Action(rotateTowards(from, to));
    }
    // Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
    // Direction::Value from = self.getDirection().getDirection();
    // if (from != to) {
    //     Logger::debug("Player 2: Rotating to face enemy.");
    //     return Action(rotateTowards(from, to));
    // }
    return Action(ActionType::None);
}
