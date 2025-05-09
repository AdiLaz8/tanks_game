#ifndef ALGO2_H
#define ALGO2_H

#include "MyTankAlgorithm.h"

class Algo2 : public MyTankAlgorithm {
public:
    using MyTankAlgorithm::MyTankAlgorithm; // ירושה של הקונסטרקטור
    inline virtual Algo2::~Algo2() override {}
    ActionRequest getAction() override;
};

#endif // ALGO2_H
