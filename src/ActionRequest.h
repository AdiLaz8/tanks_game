#ifndef ACTION_REQUEST_H
#define ACTION_REQUEST_H

#include <string>  // חשוב: נדרש בשביל std::string

enum class ActionRequest {
    MoveForward,
    MoveBackward,
    RotateLeft90,
    RotateRight90,
    RotateLeft45,
    RotateRight45,
    Shoot,
    GetBattleInfo,
    DoNothing  // This can be used to represent no action or an invalid action
};

// פונקציה שממירה enum למחרוזת
inline std::string actionToString(ActionRequest req) {
    switch (req) {
        case ActionRequest::MoveForward: return "MoveForward";
        case ActionRequest::MoveBackward: return "MoveBackward";
        case ActionRequest::RotateLeft90: return "RotateLeft90";
        case ActionRequest::RotateRight90: return "RotateRight90";
        case ActionRequest::RotateLeft45: return "RotateLeft45";
        case ActionRequest::RotateRight45: return "RotateRight45";
        case ActionRequest::Shoot: return "Shoot";
        case ActionRequest::GetBattleInfo: return "GetBattleInfo";
        case ActionRequest::DoNothing: return "DoNothing";
        default: return "DoNothing";
    }
}

#endif
