#ifndef PLAYER_FACTORY_H
#define PLAYER_FACTORY_H
#include <memory>
#include "Player.h"
class PlayerFactory {
public:
    virtual ~PlayerFactory() {}
    virtual std::unique_ptr<Player> create(
        int player_index,
        size_t x, size_t y,
        size_t max_steps,
        size_t num_shells) const = 0;
};
#endif 
