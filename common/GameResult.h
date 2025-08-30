#ifndef GAME_RESULT_H
#define GAME_RESULT_H

#include <vector>
#include <memory>
#include "SatelliteView.h"

struct GameResult {
	int winner; // 0 = tie
	enum Reason { ALL_TANKS_DEAD, MAX_STEPS, ZERO_SHELLS };
	Reason reason;
	std::vector<size_t> remaining_tanks; 
	std::unique_ptr<SatelliteView> gameState; 
	size_t rounds; 
};

#endif // GAME_RESULT_H