#ifndef ALGORITHM_318772340_206580102_MY_TANK_ALGORITHM_H
#define ALGORITHM_318772340_206580102_MY_TANK_ALGORITHM_H

#include "../common/TankAlgorithm.h"
#include "Action.h"
#include "../UserCommon/Direction.h"
#include "../UserCommon/Position.h"
#include <cstddef>
#include <vector>
#include <utility>
#include <queue>

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

class TankAlgorithm_318772340_206580102 : public TankAlgorithm {
protected:
    int turnCounterSinceInfo = -1;
    int playerId;
    int tankId;
    size_t boardWidth;
    size_t boardHeight;
    std::vector<std::pair<Position, char>> fullView;
    Direction direction;
    Position selfPosition;
    int ammo = 0;
    int shootingStatus = 0;
    int backwardStatus = 0;
    std::vector<Position> minePositions;
    bool minesInitialized = false;
    bool moveAfterRotate = false;
    
    // Player 1 specific (Algo1) variables
    std::vector<Direction::Value> currentPath;
    Position targetPos{0,0};
    bool chasing = false;
    bool needsNewInfo = true;
    int check = 0;
    bool isMine(const Position& pos) const;
    char selfSymbol() const { return playerId == 1 ? '1' : '2'; }
    char enemySymbol() const { return playerId == 1 ? '2' : '1'; }
    Direction::Value getDirectionTo(const Position& from, const Position& to) const;
    ActionRequest rotateTowards(Direction::Value current, Direction::Value desired);
    bool canShootInDirection() const;
    bool canShootInDirection(Direction dir) const;
    bool isThreatenedByShells() const;
    Action moveIfThreatened();
    int getAmmo() const;
    int getShootingStatus() const;
    Direction getTankDirection() const;
    Position getTankPosition() const;
    void decreaseShooting();
    int getTankId() const;
    void updateBattleInfo(BattleInfo& info) override;
    
    // Player 1 (Algo1) specific methods
    bool needsNewBattleInfo() const;
    void computeShootingPath();
    std::vector<Direction::Value> computeBFS(const Position& from, const Position& to,
                                             const std::vector<std::pair<Position, char>>& fullView,
                                             size_t width, size_t height);
    ActionRequest getShootingActionIfAvailable();
    ActionRequest moveForwardAfterRotate();
    ActionRequest getActionFromPath();
    
    // Player-specific action methods
    ActionRequest getPlayer1Action();
    ActionRequest getPlayer2Action();
    
    // Debug flag
    bool verbose = false;

public:
    TankAlgorithm_318772340_206580102(int playerIndex, int tankIndex);
    ActionRequest getAction() override;
    int getBackwardStatus() const;
    void decreaseBackward();
    void setBackward(int i);
};

} // namespace Algorithm_318772340_206580102

#endif // ALGORITHM_318772340_206580102_MY_TANK_ALGORITHM_H 