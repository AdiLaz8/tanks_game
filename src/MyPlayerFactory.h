#ifndef MY_PLAYER_FACTORY_H
#define MY_PLAYER_FACTORY_H

#include "PlayerFactory.h"
#include "Player1.h"
#include "Player2.h"
#include <memory>
class MyPlayerFactory : public PlayerFactory {
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override;
};

#endif