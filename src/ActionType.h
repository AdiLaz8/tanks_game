#ifndef ACTION_TYPE_H
#define ACTION_TYPE_H

enum class ActionType {
    MoveForward,
    MoveBackward,
    RotateLeft8,
    RotateRight8,
    RotateLeft4,
    RotateRight4,
    Shoot,
    None  // This can be used to represent no action or an invalid action
};

#endif // ACTION_TYPE_H