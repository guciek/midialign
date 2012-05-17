/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
* - Lupus Nocawy                                                          *
**************************************************************************/

#include <cstdio>
#include "../tracktempo.hpp"

#define FAIL printf("Test failed (%s, line %d)\n", __FILE__, __LINE__)

int test1() {
	tracktempo T(0.0);
	T.addTempoMark(4, 1.0);
	T.addTempoMark(2, 1.0);
	T.addTempoMark(0, 2.0);
	T.addTempoMark(3, 3.0);
	T.delTempoMark(2);

	if ( T.getTickTime(0) != 0.0) { FAIL; return 1; }
	if ( T.getTickTime(1) != 2.0) { FAIL; return 1; }
	if ( T.getTickTime(2) != 4.0) { FAIL; return 1; }
	if ( T.getTickTime(3) != 6.0) { FAIL; return 1; }
	if ( T.getTickTime(4) != 9.0) { FAIL; return 1; }
	if ( T.getTickTime(5) != 10.0) { FAIL; return 1; }
	if ( T.getTickTime(6) != 11.0) { FAIL; return 1; }

	if ( T.nextTempoMarkAfter(0) != 3) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(1) != 3) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(2) != 3) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(3) != 4) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(4) != 0) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(5) != 0) { FAIL; return 1; }
	if ( T.nextTempoMarkAfter(6) != 0) { FAIL; return 1; }
	
	if ( T.readTempoMark(0) != 2.0) { FAIL; return 1; }
	if ( T.readTempoMark(1) != 2.0) { FAIL; return 1; }
	if ( T.readTempoMark(2) != 2.0) { FAIL; return 1; }
	if ( T.readTempoMark(3) != 3.0) { FAIL; return 1; }
	if ( T.readTempoMark(4) != 1.0) { FAIL; return 1; }
	if ( T.readTempoMark(5) != 1.0) { FAIL; return 1; }
	if ( T.readTempoMark(6) != 1.0) { FAIL; return 1; }

	return 0;
}

int test2() {
	tracktempo A(1.0);
	tracktempo B(1.0);
	if (A != B) { FAIL; return 1; }
	if (!(A == B)) { FAIL; return 1; }
	B.addTempoMark(1, 1.0);
	if (A != B) { FAIL; return 1; }
	A.addTempoMark(10, 2.0);
	if (A == B) { FAIL; return 1; }
	if (!(A != B)) { FAIL; return 1; }

	return 0;
}

int main(void) {
	if (test1()) return 1;
	if (test2()) return 1;

	return 0;
}
