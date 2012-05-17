/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Lupus Nocawy                                                          *
**************************************************************************/

#include "tracktempo.hpp"

#include <vector>
#include <list>

#ifdef DEBUG
	#include <cstdio>
#endif

using namespace std;

typedef pair<tick_t, double> tTempoMark;

class ptracktempo {
	public:

	ptracktempo(double seconds_per_tick) {
		tempoList.push_back(tTempoMark(0,seconds_per_tick));		// tick 0
	}

	void addTempoMark(tick_t tick, double seconds_per_tick) {
		list<tTempoMark>::iterator it = tempoList.end();
		it--;
		while(it->first > tick) {
			it--;
		}
		if(it->first != tick) {
			it++;
			tempoList.insert(it,tTempoMark(tick,seconds_per_tick));	//~ add element
		}
		else {
			it->second = seconds_per_tick;
		}
	#ifdef DEBUG
		//~ for(it = tempoList.begin(); it!=tempoList.end(); it++){
			//~ printf("%llu %lf\n", it->first, it->second);
		//~ } printf("\n");
	#endif	
	}

	void delTempoMark(tick_t tick) {
		if (tick == 0) {
			return;
		}
		list<tTempoMark>::iterator it = tempoList.end();
		it--;
		while(it->first > tick) {
			it--;
		}
		if(it->first == tick) {
			tempoList.erase(it);	//~ remove element
		}
	#ifdef DEBUG
		//~ for(it = tempoList.begin(); it!=tempoList.end(); it++){
			//~ printf("%llu %lf\n", it->first, it->second);
		//~ } printf("\n");
	#endif
	}

	double getTickTime(tick_t tick) {
		double total = 0.0;
		list<tTempoMark>::iterator it = tempoList.begin();
		tick_t prev_tick = 0;
		double prev_tempo = it->second;
		it++;
		while (it != tempoList.end() && it->first < tick) {
			total += (it->first - prev_tick)*(prev_tempo);
			prev_tick = it->first;
			prev_tempo = it->second;
			it++;
		}
		total += (tick - prev_tick)*(prev_tempo);
	#ifdef DEBUG
		//~ printf("%3llu %10lf\n", tick, total);
	#endif
		return total;
	}
	
	tick_t nextTempoMarkAfter(tick_t tick) const {
		//~ list<tTempoMark>::const_iterator it = tempoList.begin();
		__typeof__(tempoList.begin()) it = tempoList.begin();
		while(it != tempoList.end() && it->first <= tick) {
			it++;
		}
		if(it != tempoList.end()) {
			return it->first;
		}
		else return 0;
	}
	
	double readTempoMark(tick_t tick) const {
		//~ list<tTempoMark>::const_iterator it = tempoList.end();
		__typeof__(tempoList.end()) it = tempoList.end();
		do {
			it--;
		} while (it->first > tick);
		return it->second;
	}

	bool operator==(const ptracktempo& b) const {
		#ifdef DEBUG
			//~ printf("#ptracktempo::operator== tempoList.size(): %d\n", (int)b.tempoList.size());
			//~ printf("
		#endif
		return 1;
	}

	bool operator!=(const ptracktempo& b) const {
		return ( !(*this==b));
		return 1;
	}

	//~ TODO destructor if needed
	//~ ~ptracktempo(){};

	private:

	//~ TODO replace list with vector, for linear insertions, but getTickTime() in O(lg)
	list<tTempoMark> tempoList;
};

tracktempo::tracktempo(double seconds_per_tick)
	{ p = new ptracktempo(seconds_per_tick); }
tracktempo::tracktempo(const tracktempo & rhs)
	{ p = new ptracktempo(*((ptracktempo *)rhs.p)); }
tracktempo::~tracktempo() { delete ((ptracktempo *)p); }
tracktempo & tracktempo::operator=(const tracktempo & rhs) {
	if (this == &rhs) return *this;
	delete ((ptracktempo *)p);
	p = new ptracktempo(*((ptracktempo *)rhs.p));
	return *this;
}
void tracktempo::addTempoMark(tick_t tick, double seconds_per_tick)
	{ return ((ptracktempo *)p)->addTempoMark(tick, seconds_per_tick); }
void tracktempo::delTempoMark(tick_t tick)
	{ return ((ptracktempo *)p)->delTempoMark(tick); }
double tracktempo::getTickTime(tick_t tick) const
	{ return ((ptracktempo *)p)->getTickTime(tick); }
tick_t tracktempo::nextTempoMarkAfter(tick_t tick) const
	{ return ((ptracktempo *)p)->nextTempoMarkAfter(tick); }
double tracktempo::readTempoMark(tick_t tick) const
	{ return ((ptracktempo *)p)->readTempoMark(tick); }

bool tracktempo::operator==(const tracktempo& b) const
	{ return ((ptracktempo *)p)->operator==( (*(ptracktempo *)b.p) ); }
bool tracktempo::operator!=(const tracktempo& b) const
	{ return ((ptracktempo *)p)->operator!=( (ptracktempo &)b.p ); }
