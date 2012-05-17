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

void dump_tracktempo_stderr(const tracktempo & tt) {
	cerr << "\ttempo:" << endl;
	tick_t i = 0;
	do {
		double v = tt.readTempoMark(i);
		cerr << "\t\ttick " << i << ": " << v << "s/tick ("
			<< (1/v) << " ticks/s)" << endl;
		i = tt.nextTempoMarkAfter(i);
	} while (i != 0);
}

void dump_track_stderr(const track & t, int tnum) {
	cerr << "Track " << tnum << endl;
	dump_tracktempo_stderr(t.getTrackTempo());
	cerr << "\tevents:" << endl;
	char buf[256];
	for (unsigned int i = 0; i < t.eventCount(); i++) {
		event const  & e = t.events(i);
		e.getDescription(buf, 256);
		double s = t.getTrackTempo().getTickTime(e.getStartTicks());
		if (e.isNote()) {
			double d = t.getTrackTempo().getTickTime(
				e.getStartTicks()+e.getDurationTicks())-s;
			cerr << "\t\tnote:  ticks [" << e.getStartTicks() << "-" <<
				(e.getStartTicks()+e.getDurationTicks()) <<
				"], time [" << s << "s-" << s+d <<
				"s]\n\t\t       " << buf << endl;
		} else {
			cerr << "\t\tevent: tick [" << e.getStartTicks() <<
				"], time [" << s <<
				"s]\n\t\t       " << buf << endl;
		}
	}
}

void dump_midi_stderr(const midi & m) {
	cerr << "MIDI File (" << m.trackCount() << " tracks)" << endl;
	for (unsigned int i = 0; i < m.trackCount(); i++)
		dump_track_stderr(m.tracks(i), i+1);
}
