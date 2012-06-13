/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

#include <iostream>
#include <inttypes.h>

uint16_t getUint16_t(const uint8_t * ptr, int offset);

uint32_t getUint32_t(std::istream &in);

uint32_t getUint24_t(std::istream &in);

uint32_t getUint24_t(uint8_t * in);

void setUint24_t(uint32_t val, uint8_t * out);

void printUint32_t(uint32_t val, std::ostream& out);

void printUint16_t(uint16_t val, std::ostream& out);

#endif
