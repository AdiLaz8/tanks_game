#include "Shell.h"

Shell::Shell(Position p, Direction d, char ownerId)
    : p(p), d(d), ownerId(ownerId) {}

char Shell::getSymbol() const {
    return '*';
}

const Position& Shell::getPosition() const {
    return p;
}
Shell::~Shell() {
    // כרגע כנראה לא צריך לעשות כלום, אבל כדאי שיהיה דיסטרקטור
}




void Shell::move(int width, int height) {
    p.move(d, width, height);
}
std::string Shell::toString() const {
    std::stringstream ss;
    ss << "Shell(owner=" << ownerId
       << ", dir=" << d.getDirection()
       << ", pos=(" << p.x << "," << p.y << "))";
    return ss.str();
}


Direction Shell::getDirection() const {
    return d;
}

char Shell::getOwnerId() const {
    return ownerId;
}
