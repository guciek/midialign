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

//~ typedef pair<tick_t, double> tTempoMark;

class tTempoMark{
	public:
	tick_t tick;
	double tempo;
	double time;
	tTempoMark(tick_t t,double s): tick(t), tempo(s), time(0) {}
};

class ptracktempo {
	public:

	ptracktempo(double seconds_per_tick) {
		tempoList.push_back(tTempoMark(0,seconds_per_tick));		// tick 0
		lastUpdatedTick = 0;
	}

	void addTempoMark(tick_t tick, double seconds_per_tick) {
		__typeof__(tempoList.end()) it = tempoList.end();
		it--;
		while(it->tick > tick) {
			it--;
		}
		if(it->tick != tick) {
			it++;
			tempoList.insert(it,tTempoMark(tick,seconds_per_tick));	//~ add element
		}
		else {
			it->tempo = seconds_per_tick;
		}
		// update time
		if (tick-1 <= lastUpdatedTick) {
			//~ time =
			lastUpdatedTick = tick;
		}
	#ifdef DEBUG
		//~ for(it = tempoList.begin(); it!=tempoList.end(); it++){
			//~ printf("%llu %lf\n", it->tick, it->tempo);
		//~ } printf("\n");
	#endif
	}

	void delTempoMark(tick_t tick) {
		if (tick == 0) {
			return;
		}
		__typeof__(tempoList.end()) it = tempoList.end();
		it--;
		while(it->tick > tick) {
			it--;
		}
		if(it->tick == tick) {
			tempoList.erase(it);	//~ remove element
		}
	#ifdef DEBUG
		//~ for(it = tempoList.begin(); it!=tempoList.end(); it++){
			//~ printf("%llu %lf\n", it->tick, it->tempo);
		//~ } printf("\n");
	#endif
	}

	double getTickTime(tick_t tick) const {
		// TODO O(n) -> (lgn)
		double total = 0.0;
		__typeof__(tempoList.begin()) it = tempoList.begin();
		tick_t prev_tick = 0;
		double prev_tempo = it->tempo;
		it++;
		while (it != tempoList.end() && it->tick < tick) {
			total += (it->tick - prev_tick)*(prev_tempo);
			prev_tick = it->tick;
			prev_tempo = it->tempo;
			it++;
		}
		total += (tick - prev_tick)*(prev_tempo);
	#ifdef DEBUG
		//~ printf("%3llu %10lf\n", tick, total);
	#endif
		return total;
	}

	tick_t nextTempoMarkAfter(tick_t tick) const {
		__typeof__(tempoList.begin()) it = tempoList.begin();
		while(it != tempoList.end() && it->tick <= tick) {
			it++;
		}
		if(it != tempoList.end()) {
			return it->tick;
		}
		else return 0;
	}

	double readTempoMark(tick_t tick) const {
		__typeof__(tempoList.end()) it = tempoList.end();
		do {
			it--;
		} while (it->tick > tick);
		return it->tempo;
	}

	tick_t findNearestTick(double time_seconds) const {
		tick_t b = 256;
		while (getTickTime(b) <= time_seconds) b <<= 4;
		tick_t a = 0;
		while (a+1 < b) {
			tick_t m = (a+b) >> 1;
			if (getTickTime(m) > time_seconds) b = m;
			else a = m;
		}
		if (time_seconds >= (0.5*(getTickTime(b)+getTickTime(a)))) return b;
		return a;
	}

	bool operator==(const ptracktempo& b) const {
		#ifdef DEBUG
			//~ printf("#ptracktempo::operator== tempoList.size(): %d\n", (int)b.tempoList.size());
			//~ printf("
		#endif
		__typeof__(tempoList.begin()) ita = tempoList.begin();
		__typeof__(b.tempoList.begin()) itb = b.tempoList.begin();
		double da=0.0, db=0.0;
		while (ita != this->tempoList.end() || itb != b.tempoList.end() ) {
			if ( da!=db ) return 0;
			if ( ita == this->tempoList.end() ) {
				db = itb->tempo;
				itb++;
			}
			else if ( itb == b.tempoList.end() ) {
				da = ita->tempo;
				ita++;
			}
			else if( ita->tick > itb->tick ) {
				db = itb->tempo;
				itb++;
			}
			else if( ita->tick < itb->tick ) {
				da = ita->tempo;
				ita++;
			}
			else {
				da = ita->tempo;
				ita++;
				db = itb->tempo;
				itb++;
			}
		}
		if ( da!=db ) return 0;
		return 1;
	}

	bool operator!=(const ptracktempo& b) const {
		return ( !(*this==b));
	}

	//~ TODO destructor if needed
	//~ ~ptracktempo(){};

	private:
	vector<tTempoMark> tempoList;
	tick_t lastUpdatedTick;
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

tick_t tracktempo::findNearestTick(double time_seconds) const
	{ return ((ptracktempo *)p)->findNearestTick(time_seconds); }

bool tracktempo::operator==(const tracktempo& b) const
	{ return ((ptracktempo *)p)->operator==( (*(ptracktempo *)b.p) ); }
bool tracktempo::operator!=(const tracktempo& b) const
	{ return ((ptracktempo *)p)->operator!=( (*(ptracktempo *)b.p) ); }
