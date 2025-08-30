#ifndef SIMULATOR_TOURNAMENT_H
#define SIMULATOR_TOURNAMENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "SimulationResults.h"
#include "GameRunner.h"

struct Competitor {
    std::string name;
    TankAlgorithmFactory algorithmFactory;
    PlayerFactory playerFactory;
    
    // Tournament statistics
    size_t wins = 0;
    size_t losses = 0;
    size_t ties = 0;
    size_t totalGames = 0;
    double score = 0.0;
    
    double getWinRate() const {
        return totalGames > 0 ? static_cast<double>(wins) / totalGames : 0.0;
    }
};

struct Match {
    std::string id;
    Competitor* competitor1;
    Competitor* competitor2;
    std::string mapName;
    GameExecution result;
    bool completed = false;
    
    // For tournament brackets
    size_t round = 0;
    size_t matchInRound = 0;
    
    Match(const std::string& matchId, Competitor* c1, Competitor* c2, const std::string& map, size_t r = 0, size_t m = 0)
        : id(matchId), competitor1(c1), competitor2(c2), mapName(map), round(r), matchInRound(m) {}
};

struct TournamentRound {
    size_t roundNumber;
    std::vector<std::unique_ptr<Match>> matches;
    bool completed = false;
    
    TournamentRound(size_t num) : roundNumber(num) {}
};

class Tournament {
public:
    // Tournament settings
    enum TournamentType {
        ROUND_ROBIN,    // Everyone plays everyone
        SINGLE_ELIMINATION,
        DOUBLE_ELIMINATION
    };

private:
    std::vector<Competitor> competitors;
    std::vector<std::unique_ptr<TournamentRound>> rounds;
    std::vector<MapData> maps;
    
    TournamentType type;
    
    bool useMultipleMaps; // If true, play on all maps for each matchup
    size_t gamesPerMatchup; // Number of games per competitor pair
    
public:
    Tournament(TournamentType tournamentType = ROUND_ROBIN);
    
    // Setup
    void addCompetitor(const std::string& name, TankAlgorithmFactory algFactory, PlayerFactory playerFactory);
    void setMaps(const std::vector<MapData>& tournamentMaps);
    void setGamesPerMatchup(size_t games) { gamesPerMatchup = games; }
    void setUseMultipleMaps(bool use) { useMultipleMaps = use; }
    
    // Tournament generation
    void generateRounds();
    void generateRoundRobin();
    void generateSingleElimination();
    
    // Execution
    std::vector<Match*> getAllPendingMatches();
    void recordMatchResult(const std::string& matchId, const GameExecution& result);
    bool isComplete() const;
    
    // Results
    std::vector<Competitor> getFinalStandings() const;
    void printTournamentBracket() const;
    void printStandings() const;
    
    // Getters
    const std::vector<Competitor>& getCompetitors() const { return competitors; }
    const std::vector<std::unique_ptr<TournamentRound>>& getRounds() const { return rounds; }
    size_t getTotalMatches() const;
    size_t getCompletedMatches() const;
    
private:
    void updateCompetitorStats(Competitor& competitor, const GameExecution& result, bool isPlayer1);
    std::string generateMatchId(const Competitor& c1, const Competitor& c2, const std::string& map, size_t gameNum = 0);
    void generateNextEliminationRound(size_t completedRoundNum);
};

#endif // SIMULATOR_TOURNAMENT_H