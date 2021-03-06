/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "midi_file_write.hpp"
#include "midi_file_common.hpp"
#include "stream_utils.hpp"

#include <vector>
#include <cmath>
#include <tr1/memory>
#include <algorithm>
#include <fstream>

using namespace std;
using namespace tr1;

#define EVENT_COMMAND_MASK         0xF0
#define EVENT_CHANNEL_MASK         0xF

struct start_event {
	shared_ptr<event> e;
	tick_t start;
	start_event(shared_ptr<event> e, tick_t start)
		: e(e), start(start) {}
};

uint16_t getFileFormat(const midi& m) {
	if (m.trackCount() <= 1)
		return 0;
	uint16_t ff = 1;
	for (unsigned i = 1; i < m.trackCount(); ++i) {
		if (m.tracks(i).getTrackTempo() != m.tracks(0).getTrackTempo()) {
			ff = 2;
			break;
		}
	}
	return ff;
}
void printHeader(const midi& m, ostream &out) {
	out.write("MThd\0\0\0\6", 8);
	if (m.trackCount() == 0)
		throw "No tracks! Cannot save a file.";
	else if (m.trackCount() == 1)
		printUint16_t(1, out);
	else {
		printUint16_t(getFileFormat(m), out);
	}
	printUint16_t(m.trackCount(), out);
	printUint16_t((uint16_t) m.getTicksPerQuaterNote(), out);
}
bool startCmp(start_event a, start_event b) {
	return a.start < b.start;
}
void assignTempoMark(uint32_t msecPerQuarter, pevent& e) {
	if ((msecPerQuarter & ((uint32_t) 0xFF000000)) != 0) {
		cerr << "Warning: truncated tempo mark!" << endl;
	}
	int rawlen = 6;
	uint8_t raw[rawlen];
	raw[0] = (EVENT_COMMAND_MASK | CMD_META_EVENT);
	raw[1] = META_TEMPO_CHANGE;
	raw[2] = (uint8_t) 3;
	setUint24_t(msecPerQuarter, raw + 3);
	e.setBytes(raw, rawlen);
}
int serializeEvent(const event& e, ostream& out) {
	uint8_t buf[260];
	int buflen = e.getBytes(buf, 260);
	out.write((char *) buf, buflen);
	return buflen;
}
int printVLE(tick_t value, ostream& out) {
	if (value == 0) {
		out.put('\0');
		return 1;
	}
	vector<uint8_t> deltav;
	while (value > 0) {
		deltav.push_back(value & 0x7F);
		value >>= 7;
	}
	reverse(deltav.begin(), deltav.end());
	for (int i = 0; i < ((int) deltav.size()) - 1; ++i)
		deltav[i] = (deltav[i] | 0x80);
	for (unsigned i = 0; i < deltav.size(); ++i)
		out.put((char) deltav[i]);
	return deltav.size();
}

int writeTrack(const track& t, ostream& out, uint16_t tpq = 0, bool writeTempo = false) {
	const tracktempo& thisTrackTempo = t.getTrackTempo();
	// get a collection of <start_tick, event>
	vector<start_event> saveEvents;
	uint8_t buf[260];
	int buflen;
	for (unsigned i = 0; i < t.eventCount(); ++i) {
		pevent * e = new pevent;
		buflen = t.events(i).getBytes(buf, 260);
		e->setBytes(buf, buflen);
		if (! (e->getCommand() == CMD_META_EVENT && e->getMetaCommand() == META_TRACK_END))
			saveEvents.push_back(start_event(shared_ptr<event>(e), t.getEventTicks(i)));
		if (t.events(i).isNote()) {
			pevent * eOff = new pevent;
			buflen = dynamic_cast<const pnote&>(t.events(i)).getNoteOffBytes(buf, 260);
			eOff->setBytes(buf, buflen);
			saveEvents.push_back(start_event(
				shared_ptr<event>(eOff), t.getEventTicks(i) + t.getNoteDurationTicks(i)));
		}
	}

	if (writeTempo) {
		tick_t tempoMark = 0;
		do
		{
			double secPerTick = thisTrackTempo.readTempoMark(tempoMark);
			double secPerQuarter = secPerTick * ((double) tpq);
			uint32_t msecPerQuarterInt= (uint32_t) floor((1000000.0 * secPerQuarter) + 0.5);

			pevent * tempoChange = new pevent;
			assignTempoMark(msecPerQuarterInt, *tempoChange);
			saveEvents.push_back(start_event(shared_ptr<event>(tempoChange), tempoMark));

			tempoMark = thisTrackTempo.nextTempoMarkAfter(tempoMark);
		} while (tempoMark != 0);
	}

	stable_sort(saveEvents.begin(), saveEvents.end(), startCmp);


	uint32_t totalLen = 4;
	out.write("MTrk", 4);
	printUint32_t(0xdeadbeef, out); // print length placeholder
	totalLen += 4;

	for (__typeof__(saveEvents.begin()) it = saveEvents.begin(),
						prev = saveEvents.begin(); it != saveEvents.end(); it++) {
		tick_t delta_tick = it->start - prev->start;
		totalLen += printVLE(delta_tick, out);
		totalLen += serializeEvent(*(it->e), out);
		prev = it;
	}

	out.write("\x00\xFF\x2F\x00", 4);
	totalLen += 4;

	return totalLen;
}

bool midi_file_write(const midi & m, const char * fn) {
	ofstream out(fn, ios_base::out | ios_base::binary);
	if (out.fail()) return false;
	printHeader(m, out);
	int lengths[m.trackCount()];
	uint16_t ff = getFileFormat(m);
	if (m.trackCount()) {
		lengths[0] = writeTrack(m.tracks(0), out, m.getTicksPerQuaterNote(), true);
		for (unsigned i = 1; i < m.trackCount(); ++i) {
			if (ff < 2)
				lengths[i] = writeTrack(m.tracks(i), out);
			else
				lengths[i] = writeTrack(m.tracks(i), out, m.getTicksPerQuaterNote(), true);
		}
	}
	out.seekp(14, ios_base::beg);
	for (unsigned i = 0; i < m.trackCount(); ++i) {
		out.seekp(4, ios_base::cur);
		printUint32_t(lengths[i] - 8, out);
		out.seekp(lengths[i] - 8, ios_base::cur);
	}

	out.close();
	return out.fail();
}
