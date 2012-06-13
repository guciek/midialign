/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#ifndef FIND_NICE_TEMPO_H
#define FIND_NICE_TEMPO_H

#include "tracktempo.hpp"

double find_nice_constant_bpm(const double * event_times,
		unsigned int event_count);

#endif
