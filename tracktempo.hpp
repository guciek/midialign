/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
* - Lupus Nocawy                                                          *
**************************************************************************/

#ifndef TRACKTEMPO_H
#define TRACKTEMPO_H

typedef unsigned long long tick_t;

class tracktempo {
	public:
		tracktempo(double seconds_per_tick);
		tracktempo(const tracktempo &);
		~tracktempo();
		tracktempo & operator=(const tracktempo &);
		void addTempoMark(tick_t tick, double seconds_per_tick);
		void delTempoMark(tick_t tick);
		double getTickTime(tick_t tick) const;
		tick_t nextTempoMarkAfter(tick_t tick) const;
		double readTempoMark(tick_t tick) const;
		bool operator==(const tracktempo& b) const;
		bool operator!=(const tracktempo& b) const;
	private:
		void * p;
};

#endif
