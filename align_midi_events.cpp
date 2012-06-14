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

void align_midi_event(track & t, unsigned int i, const tracktempo & newtempo) {
	double s = t.getEventSeconds(i);
	tick_t start_ticks = newtempo.findNearestTick(s);
	if (t.events(i).isNote()) {
		double d = t.getNoteDurationSeconds(i);
		tick_t duration_ticks = newtempo.findNearestTick
			(newtempo.getTickTime(start_ticks)+d) - start_ticks;
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
