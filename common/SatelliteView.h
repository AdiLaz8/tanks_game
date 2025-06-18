#ifndef SATELITEVIEW_H
#define SATELITEVIEW_H
#include <cstddef>

class SatelliteView {
public:
	virtual ~SatelliteView() {}
    virtual char getObjectAt(size_t x, size_t y) const = 0;
};
#endif