#include "Algorithm2.h"
#include <iostream>
#include "Logger.h"

// returns the next action of the tank
Action Algorithm2::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    // first, if the tank is threatend by shells, try to escape
    if (isThreatenedByShells(board, self.getPosition())) {
        Logger::debug("Player 2: Threatened By Shells");
        return moveIfThreatened(board, self);
    }
    // if not threatened, try to see if you can shoot an enemy tank directly from the current position and direction
    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0 && self.getAmmo() > 0) {
        Logger::debug("Player 2: Enemy in direct line of fire. Shooting now.");
        return Action(ActionType::Shoot);
    }
    // because this is the chased algorithm, if we managed to escape for the entire game and the enemy tank wasted all of it's ammo,
    // then be active and try to shoot the enemy tank if you have clear path/can rotate towards it
    if (enemy.getAmmo() == 0) {
        Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
        Direction::Value from = self.getDirection().getDirection();
        if (from == to && self.getAmmo() > 0) {
            if(self.getShootingStatus()==0){
                Logger::debug("Player 2: Trying to shoot towards the enemy because the enemy tank wasted its entire ammo.");
                return Action(ActionType::Shoot);
            }
            else{
                return Action(ActionType::None);
            }
        }
        Logger::debug("Player 2: Trying to rotate towards the enemy in order to shoot him because the enemy tank wasted its entire ammo.");
        return Action(rotateTowards(from, to));
    }
    // if not threatened, try to see if you can rotate to a shooting direction
    Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
    Direction::Value from = self.getDirection().getDirection();
    if (from != to) {
        Logger::debug("Player 2: Rotating to face enemy.");
        return Action(rotateTowards(from, to));
    }
    
    // if not threatened and also can't shoot, be passive and don't do anything
    return Action(ActionType::None);
}