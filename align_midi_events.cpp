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
	while (t.getTickTime(b) <= s) b <<= 4;
	tick_t a = 0;
	while (a+1 < b) {
		tick_t m = (a+b) >> 1;
		if (t.getTickTime(m) > s) b = m;
		else a = m;
	}
	if (s >= (0.5*(t.getTickTime(b)+t.getTickTime(a)))) return b;
	return a;
}

void align_midi_event(track & t, unsigned int i, const tracktempo & newtempo) {
	double s = t.getEventSeconds(i);
	tick_t start_ticks = nearest_tick(newtempo, s);
	if (t.events(i).isNote()) {
		double d = t.getNoteDurationSeconds(i);
		tick_t duration_ticks = nearest_tick(newtempo,
			newtempo.getTickTime(start_ticks)+d) - start_ticks;
		if (duration_ticks < 1) duration_ticks = 1;
		t.setNoteDurationTicks(i, duration_ticks);
	}
	t.setEventTicks(i, start_ticks);
}

void align_midi_events(midi & m, tick_t tickPerQuarterNote,
	const tracktempo & newtempo) {
	for (unsigned int i = 0; i < m.trackCount(); i++) {
		tracktempo oldtempo = m.tracks(i).getTrackTempo();
		for (unsigned int j = 0; j < m.tracks(i).eventCount(); j++) {
			align_midi_event(m.tracks(i), j, newtempo);
		}
		m.tracks(i).setTrackTempo(newtempo);
	}
	m.setTicksPerQuaterNote(tickPerQuarterNote);
}
