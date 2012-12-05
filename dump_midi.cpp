/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#include "dump_midi.hpp"

#include <iostream>
using namespace std;

void dump_tracktempo_stdout(const tracktempo & tt) {
	tick_t i = 0;
	do {
		double v = tt.readTempoMark(i);
		cout << "Tempo at tick " << i << ": " << v
			<< "s/tick (" << (1/v) << " ticks/s)" << endl;
		i = tt.nextTempoMarkAfter(i);
	} while (i != 0);
}

void dump_track_stdout(const track & t, int tnum) {
	cout << "Track " << tnum << endl;
	dump_tracktempo_stdout(t.getTrackTempo());
	char buf[256];
	for (unsigned int i = 0; i < t.eventCount(); i++) {
		event const & e = t.events(i);
		e.getDescription(buf, 256);
		tick_t st = t.getEventTicks(i);
		double sd = t.getEventSeconds(i);
		if (e.isNote()) {
			cout << "Note [" << st << "-" <<
				(st+t.getNoteDurationTicks(i)) <<
				"] [" << sd << "s-" <<
				sd+t.getNoteDurationSeconds(i) <<
				"s]: " << buf << endl;
		} else {
			cout << "Event [" << st <<
				"] [" << sd <<
				"s]: " << buf << endl;
		}
	}
}

void dump_midi_stdout(const midi & m) {
	cout << "MIDI File (" << m.trackCount() << " tracks, " <<
		m.getTicksPerQuaterNote() << " ticks/quater note)" << endl;
	for (unsigned int i = 0; i < m.trackCount(); i++)
		dump_track_stdout(m.tracks(i), i+1);
}
