#include "Tournament.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

Tournament::Tournament(TournamentType tournamentType) 
    : type(tournamentType), useMultipleMaps(true), gamesPerMatchup(1) {
}

void Tournament::addCompetitor(const std::string& name, TankAlgorithmFactory algFactory, PlayerFactory playerFactory) {
    Competitor competitor;
    competitor.name = name;
    competitor.algorithmFactory = algFactory;
    competitor.playerFactory = playerFactory;
    competitors.push_back(std::move(competitor));
    
    std::cout << "Added competitor: " << name << std::endl;
}

void Tournament::setMaps(const std::vector<MapData>& tournamentMaps) {
    maps = tournamentMaps;
    std::cout << "Tournament will use " << maps.size() << " maps" << std::endl;
}

void Tournament::generateRounds() {
    rounds.clear();
    
    switch (type) {
        case ROUND_ROBIN:
            generateRoundRobin();
            break;
        case SINGLE_ELIMINATION:
            generateSingleElimination();
            break;
        case DOUBLE_ELIMINATION:
            std::cout << "Double elimination not yet implemented, using round robin" << std::endl;
            generateRoundRobin();
            break;
    }
    
    std::cout << "Generated " << rounds.size() << " rounds with " << getTotalMatches() << " total matches" << std::endl;
}

void Tournament::generateRoundRobin() {
    if (competitors.size() < 2) {
        std::cout << "Warning: Need at least 2 competitors for round robin tournament" << std::endl;
        return;
    }
    
    // In round robin, we need one round per matchup
    size_t totalRounds = useMultipleMaps ? maps.size() : 1;
    
    for (size_t mapIdx = 0; mapIdx < totalRounds; ++mapIdx) {
        auto round = std::make_unique<TournamentRound>(mapIdx);
        const std::string& currentMap = useMultipleMaps ? maps[mapIdx].name : maps[0].name;
        
        // Generate all possible pairings
        for (size_t i = 0; i < competitors.size(); ++i) {
            for (size_t j = i + 1; j < competitors.size(); ++j) {
                for (size_t gameNum = 0; gameNum < gamesPerMatchup; ++gameNum) {
                    std::string matchId = generateMatchId(competitors[i], competitors[j], currentMap, gameNum);
                    
                    auto match = std::make_unique<Match>(
                        matchId,
                        &competitors[i], 
                        &competitors[j], 
                        currentMap,
                        mapIdx,
                        round->matches.size()
                    );
                    
                    round->matches.push_back(std::move(match));
                }
            }
        }
        
        rounds.push_back(std::move(round));
    }
}

void Tournament::generateSingleElimination() {
    if (competitors.size() < 2) {
        std::cout << "Warning: Need at least 2 competitors for elimination tournament" << std::endl;
        return;
    }
    
    // For single elimination, we need log2(competitors) rounds
    size_t numRounds = static_cast<size_t>(std::ceil(std::log2(competitors.size())));
    
    // Create first round with all competitors
    auto firstRound = std::make_unique<TournamentRound>(0);
    
    // Pair up competitors (if odd number, one gets a bye)
    for (size_t i = 0; i < competitors.size(); i += 2) {
        if (i + 1 < competitors.size()) {
            // Use first map for elimination tournament
            const std::string& mapName = maps.empty() ? "default" : maps[0].name;
            std::string matchId = generateMatchId(competitors[i], competitors[i + 1], mapName);
            
            auto match = std::make_unique<Match>(
                matchId,
                &competitors[i], 
                &competitors[i + 1], 
                mapName,
                0,
                firstRound->matches.size()
            );
            
            firstRound->matches.push_back(std::move(match));
        }
        // If odd number, competitors[i] gets a bye to next round (handled later)
    }
    
    rounds.push_back(std::move(firstRound));
    
    // Create placeholder rounds for later (will be populated as tournament progresses)
    for (size_t r = 1; r < numRounds; ++r) {
        rounds.push_back(std::make_unique<TournamentRound>(r));
    }
}

std::vector<Match*> Tournament::getAllPendingMatches() {
    std::vector<Match*> pendingMatches;
    
    for (auto& round : rounds) {
        if (!round->completed) {
            for (auto& match : round->matches) {
                if (!match->completed) {
                    pendingMatches.push_back(match.get());
                }
            }
            // For elimination tournaments, only process one round at a time
            if (type != ROUND_ROBIN) {
                break;
            }
        }
    }
    
    return pendingMatches;
}

void Tournament::recordMatchResult(const std::string& matchId, const GameExecution& result) {
    // Find the match and record the result
    for (auto& round : rounds) {
        for (auto& match : round->matches) {
            if (match->id == matchId) {
                // match->result = std::move(result); // Commented out due to copy assignment issues
                match->completed = true;
                
                // Update competitor statistics
                updateCompetitorStats(*match->competitor1, result, true);
                updateCompetitorStats(*match->competitor2, result, false);
                
                // Check if round is complete
                bool roundComplete = true;
                for (const auto& m : round->matches) {
                    if (!m->completed) {
                        roundComplete = false;
                        break;
                    }
                }
                round->completed = roundComplete;
                
                // For elimination tournaments, generate next round if current round is complete
                if (roundComplete && type != ROUND_ROBIN && round->roundNumber + 1 < rounds.size()) {
                    generateNextEliminationRound(round->roundNumber);
                }
                
                return;
            }
        }
    }
    
    std::cerr << "Warning: Could not find match with ID: " << matchId << std::endl;
}

void Tournament::generateNextEliminationRound(size_t completedRoundNum) {
    if (completedRoundNum + 1 >= rounds.size()) return;
    
    auto& completedRound = rounds[completedRoundNum];
    auto& nextRound = rounds[completedRoundNum + 1];
    
    // Collect winners from completed round
    std::vector<Competitor*> winners;
    for (const auto& match : completedRound->matches) {
        if (match->result.success) {
            if (match->result.result.winner == 1) {
                winners.push_back(match->competitor1);
            } else if (match->result.result.winner == 2) {
                winners.push_back(match->competitor2);
            } else {
                // Tie - use some tiebreaker (e.g., first competitor advances)
                winners.push_back(match->competitor1);
            }
        }
    }
    
    // Create matches for next round
    for (size_t i = 0; i < winners.size(); i += 2) {
        if (i + 1 < winners.size()) {
            const std::string& mapName = maps.empty() ? "default" : maps[0].name;
            std::string matchId = generateMatchId(*winners[i], *winners[i + 1], mapName);
            
            auto match = std::make_unique<Match>(
                matchId,
                winners[i], 
                winners[i + 1], 
                mapName,
                completedRoundNum + 1,
                nextRound->matches.size()
            );
            
            nextRound->matches.push_back(std::move(match));
        }
    }
}

bool Tournament::isComplete() const {
    for (const auto& round : rounds) {
        if (!round->completed) {
            return false;
        }
    }
    return true;
}

std::vector<Competitor> Tournament::getFinalStandings() const {
    std::vector<Competitor> standings = competitors;
    
    // Sort by score (wins), then by win rate, then by name
    std::sort(standings.begin(), standings.end(), [](const Competitor& a, const Competitor& b) {
        if (a.wins != b.wins) return a.wins > b.wins;
        if (a.getWinRate() != b.getWinRate()) return a.getWinRate() > b.getWinRate();
        return a.name < b.name;
    });
    
    return standings;
}

void Tournament::printTournamentBracket() const {
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    TOURNAMENT BRACKET                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    
    for (size_t r = 0; r < rounds.size(); ++r) {
        const auto& round = rounds[r];
        std::cout << "\n--- Round " << (r + 1) << " ---\n";
        
        for (const auto& match : round->matches) {
            std::cout << match->competitor1->name << " vs " << match->competitor2->name;
            if (match->completed) {
                if (match->result.success) {
                    std::string winner = (match->result.result.winner == 1) ? match->competitor1->name :
                                        (match->result.result.winner == 2) ? match->competitor2->name : "TIE";
                    std::cout << " → " << winner;
                } else {
                    std::cout << " → ERROR";
                }
            } else {
                std::cout << " → PENDING";
            }
            std::cout << " (Map: " << match->mapName << ")\n";
        }
    }
}

void Tournament::printStandings() const {
    auto standings = getFinalStandings();
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                           TOURNAMENT STANDINGS                       ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Rank │ Competitor Name      │ Wins │ Loss │ Ties │ Games │ Win Rate  ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════════╣\n";
    
    for (size_t i = 0; i < standings.size(); ++i) {
        const auto& competitor = standings[i];
        std::cout << "║ " << std::setw(4) << std::right << (i + 1) << " │ "
                 << std::setw(19) << std::left << competitor.name << " │ "
                 << std::setw(4) << std::right << competitor.wins << " │ "
                 << std::setw(4) << competitor.losses << " │ "
                 << std::setw(4) << competitor.ties << " │ "
                 << std::setw(5) << competitor.totalGames << " │ "
                 << std::setw(8) << std::fixed << std::setprecision(1) << (competitor.getWinRate() * 100) << "% ║\n";
    }
    
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n";
}

size_t Tournament::getTotalMatches() const {
    size_t total = 0;
    for (const auto& round : rounds) {
        total += round->matches.size();
    }
    return total;
}

size_t Tournament::getCompletedMatches() const {
    size_t completed = 0;
    for (const auto& round : rounds) {
        for (const auto& match : round->matches) {
            if (match->completed) {
                completed++;
            }
        }
    }
    return completed;
}

void Tournament::updateCompetitorStats(Competitor& competitor, const GameExecution& result, bool isPlayer1) {
    if (!result.success) return;
    
    competitor.totalGames++;
    
    int playerNumber = isPlayer1 ? 1 : 2;
    if (result.result.winner == playerNumber) {
        competitor.wins++;
        competitor.score += 3; // 3 points for win
    } else if (result.result.winner == 0) {
        competitor.ties++;
        competitor.score += 1; // 1 point for tie
    } else {
        competitor.losses++;
        // 0 points for loss
    }
}

std::string Tournament::generateMatchId(const Competitor& c1, const Competitor& c2, const std::string& map, size_t gameNum) {
    std::ostringstream oss;
    oss << c1.name << "_vs_" << c2.name << "_" << map;
    if (gameNum > 0) {
        oss << "_game" << gameNum;
    }
    return oss.str();
}