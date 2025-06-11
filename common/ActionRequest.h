#ifndef ACTION_REQUEST_H
#define ACTION_REQUEST_H

#include <string>  

enum class ActionRequest {
    MoveForward,
    MoveBackward,
    RotateLeft90,
    RotateRight90,
    RotateLeft45,
    RotateRight45,
    Shoot,
    GetBattleInfo,
    DoNothing  
};

#endif