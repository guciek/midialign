/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/
 
#include <vector>
#include <iostream>
using namespace std;

#include "align_midi_events.hpp"

tick_t nearest_tick(const tracktempo & t, double s) {
	tick_t b = 256;
	while (t.getTickTime(b) < s) b <<= 4;
	tick_t a = 0;
	while (a+1 < b) {
		tick_t m = (a+b) >> 1;
		if (t.getTickTime(m) > s) b = m;
		else a = m;
	}
	return a;
}

void align_midi_event(event & e, const tracktempo & oldtempo,
		const tracktempo & newtempo) {
	double s = oldtempo.getTickTime(e.getStartTicks());
	tick_t start_ticks = nearest_tick(newtempo, s);
	if (e.isNote()) {
		double d = oldtempo.getTickTime(e.getStartTicks()+
			e.getDurationTicks()) - s;
		tick_t duration_ticks = nearest_tick(newtempo,
			newtempo.getTickTime(start_ticks)+d) - start_ticks;
		if (duration_ticks < 1) duration_ticks = 1;
		e.setDurationTicks(duration_ticks);
	}
	e.setStartTicks(start_ticks);
}

void align_midi_events(midi & m, const tracktempo & newtempo) {
	for (unsigned int i = 0; i < m.trackCount(); i++) {
		tracktempo oldtempo = m.tracks(i).getTrackTempo();
		for (unsigned int j = 0; j < m.tracks(i).eventCount(); j++) {
			align_midi_event(m.tracks(i).events(j),
				oldtempo, newtempo);
			m.tracks(i).setTrackTempo(newtempo);
		}
	}
}
