/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "stream_utils.hpp"

using namespace std;

uint16_t getUint16_t(const uint8_t * ptr, int offset) {
	ptr += offset;
	uint16_t v = *((uint16_t *) ptr);
	return ((v << 8) | (v >> 8));
}

uint32_t getUint32_t(istream &in) {
	uint32_t result = 0;
	uint8_t tmp;
	for (int i = 0; i < 4; ++i) {
		in.read((char *) &tmp, 1);
		result <<= 8;
		result |= tmp;
	}
	return result;
}

uint32_t getUint24_t(istream &in) {
	uint32_t result = 0;
	uint8_t tmp;
	for (int i = 0; i < 3; ++i) {
		in.read((char *) &tmp, 1);
		result <<= 8;
		result |= tmp;
	}
	return result;
}

uint32_t getUint24_t(uint8_t * in) {
	uint32_t result = 0;
	for (int i = 0; i < 3; ++i) {
		result <<= 8;
		result |= in[i];
	}
	return result;
}

void setUint24_t(uint32_t val, uint8_t * out) {
	for (int i = 2; i >= 0; --i) {
		out[i] = (val & 0x000000FF);
		val >>= 8;
	}
}

void printUint32_t(uint32_t val, ostream& out) {
	for (int i = 24; i >= 0; i -= 8) {
		out.put((char) ((val >> i) & 0x000000FF));
	}
}

void printUint16_t(uint16_t val, ostream& out) {
	for (int i = 8; i >= 0; i -= 8) {
		out.put((char) ((val >> i) & 0x000000FF));
	}
}
