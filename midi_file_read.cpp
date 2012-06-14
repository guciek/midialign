/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "midi_file_read.hpp"
#include "stream_utils.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;

class pmidi; // announce


class legacy_pevent {
	public:

	legacy_pevent() : len(0), pairVelocity(0), raw(NULL), rawlen(0), home(NULL)  {}
	legacy_pevent(tick_t start, track * home)
		: start(start), len(0), pairVelocity(0), raw(NULL), rawlen(0), home(home) {}
	legacy_pevent(const legacy_pevent& o) {
		//cerr << "BBB" << endl;
		start = o.start;
		len = o.len;
		rawlen = o.rawlen;
		pairVelocity = o.pairVelocity;
		home = o.home;
		if (o.rawlen > 0) {
			//cerr << "CCC " << (int) o.raw[0] << endl;
			raw = new uint8_t[rawlen];
			memcpy(raw, o.raw, rawlen);
			for (int i = 0; i < rawlen; ++i)
				raw[i] = o.raw[i];
			//copy(o.raw, o.raw + rawlen, raw);
		}
		else
			raw = NULL;
	}

	virtual ~legacy_pevent() {
		if (raw != NULL) delete [] raw;
	}

	virtual tick_t getStartTicks() const { return start; }

	virtual tick_t getDurationTicks() const {
		if (! isNote())
			throw "Unable to get duration ticks for a non-note event.";
		else
			return len;
	}

	virtual void setStartTicks(tick_t ticks) { start = ticks; }

	virtual void setDurationTicks(tick_t ticks) {
		if (! isNote())
			throw "Unable to get duration ticks for a non-note event.";
		else
			len = ticks;
	}

	virtual double getStart() const {
		return home->getTrackTempo().getTickTime(start);
	}

	virtual double getDuration() const {
		return home->getTrackTempo().getTickTime(start + len) -
		       home->getTrackTempo().getTickTime(start);
	}

	virtual bool isNote() const {
		if (rawlen == 0) return false;
		return (getCommand() == CMD_NOTE_ON);
	}

	uint8_t getCommand() const {
		return (raw[0] >> 4);
	}

	uint8_t getMetaCommand() const {
		if (getCommand() != CMD_META_EVENT)
			throw "getMetaCommand: Not a meta command!";
		return raw[1];
	}

	uint8_t getNoteVelocity() const {
		if (getCommand() != CMD_NOTE_ON && getCommand() != CMD_NOTE_OFF)
			throw "getNoteVelocity: velocity not defined for events of this type!";
		return raw[2];
	}

	/**
	 * Test if events are the same wrt. channel and first byte after
	 * a command byte.
	 * Particulary, if this and o are EVENT_NOTEs, tells if
	 * they are the same pitch and on the same channel.
	 **/
	bool equalChannelAndPitch(const legacy_pevent &o) {
		return (
			((raw[0] & EVENT_CHANNEL_MASK) == (o.raw[0] & EVENT_CHANNEL_MASK)) &&
			(raw[1] == o.raw[1])
			);
		/*if (rawlen != o.rawlen)
			return false;
		if (rawlen > 0) {
			if ((raw[0] & EVENT_CHANNEL_MASK) != (o.raw[0] & EVENT_CHANNEL_MASK))
				return false;
			if (strncmp((char *) raw + 1, (char *) o.raw + 1, rawlen - 1) != 0)
				return false;
		}
		return true;*/
	}

	virtual void getDescription(char * buffer, unsigned int length) const {
		stringstream ss(stringstream::in | stringstream::out);
		ss.setf(ios::showbase);
		ss << cmd2str[getCommand()];
		if (isNote()) {
			ss << ", pitch: " << hex << (int) raw[1];
		}
		if (getCommand() == CMD_META_EVENT) {
			ss << ", meta command: " << hex << (int) raw[1];
			if (getMetaCommand() == META_TEMPO_CHANGE) {
				ss << ", META_TEMPO_CHANGE";
			}
		}
		ss.getline(buffer, length);
	}

	template<class InputIterator>
	void setRaw(InputIterator begin, InputIterator end) {
		if (raw != NULL) delete [] raw;
		raw = new uint8_t[end - begin];
		rawlen = end - begin;
		copy(begin, end, raw);
	}

	void assignNoteOff(const legacy_pevent &noteOn) {
		if (! noteOn.isNote())
			throw "assignNoteOff: given event is not a NOTE_ON event.";
		if (raw != NULL) delete [] raw;
		home = noteOn.home;
		start = noteOn.start + noteOn.len;
		len = pairVelocity = 0;
		raw = new uint8_t[3];
		rawlen = 3;
		raw[0] = (noteOn.raw[0] & EVENT_CHANNEL_MASK);
		raw[0] |= 0x80;
		raw[1] = noteOn.raw[1];
		raw[2] = noteOn.pairVelocity;
	}

	void assignTempoMark(uint32_t msecPerQuarter) {
		if ((msecPerQuarter & ((uint32_t) 0xFF000000)) != 0) {
			cerr << "Warning: truncated tempo mark!" << endl;
		}
		if (raw != NULL) delete [] raw;
		rawlen = 6;
		raw = new uint8_t[rawlen];
		raw[0] = (EVENT_COMMAND_MASK | CMD_META_EVENT);
		raw[1] = META_TEMPO_CHANGE;
		raw[2] = (uint8_t) 3;
		setUint24_t(msecPerQuarter, raw + 3);
	}

	size_t getRawLen() const { return rawlen; }

	virtual unsigned int getBytes(uint8_t * buf, unsigned int sz) const {
		memcpy(buf, raw, min(sz, (unsigned) rawlen));
		return min(sz, (unsigned) rawlen);
	}

	legacy_pevent& operator=(const legacy_pevent& rhs) {
		if (this == &rhs) return *this;
		//cerr << "AAA" << endl;
		start = rhs.start;
		len = rhs.len;
		rawlen = rhs.rawlen;
		pairVelocity = rhs.pairVelocity;
		home = rhs.home;
		if (raw != NULL)
			delete [] raw;
		if (rhs.raw != NULL) {
			raw = new uint8_t[rawlen];
			memcpy(raw, rhs.raw, rawlen);
		}
		return *this;
	}

	bool operator==(const legacy_pevent& o) const {
		if (start != o.start) return false;
		if (len != o.len) return false;
		if (pairVelocity != o.pairVelocity) return false;
		if (rawlen != o.rawlen) return false;
		if (home != o.home) return false;
		if (strncmp((char *) raw, (char *) o.raw, rawlen) != 0) return false;
		return true;
	}

	friend ostream& operator<<(ostream&, const legacy_pevent&);



	tick_t start;
	tick_t len;
	uint8_t pairVelocity;

#ifdef DEBUG
	uint8_t * raw;
	int rawlen;
#endif

	private:

#ifndef DEBUG
	uint8_t * raw;
	int rawlen;
#endif

	track * home;
};

ostream& operator<<(ostream& out, const legacy_pevent& ev) {
	out.write((char *) ev.raw, ev.rawlen);
	return out;
}

bool startCmp(legacy_pevent a, legacy_pevent b) {
	// first things first
	if (a.start != b.start) return a.start < b.start;
	if (a.getCommand() == CMD_META_EVENT && a.getMetaCommand() == META_TRACK_END)
		return false;
	if (b.getCommand() == CMD_META_EVENT && b.getMetaCommand() == META_TRACK_END)
		return true;
	return a.start < b.start;
}

class ptrack : public track {
	public :

	ptrack() : thisTracktempo(tracktempo(0.0)), p_hasTempoMarkAt0(false) {}

	void load(ifstream &in, uint16_t tpq) {
		this->tpq = tpq;
		// assume default 120 bpm => 2 quarter notes per second
		thisTracktempo = tracktempo(1.0/(2.0 * ((double) tpq)));
		char sig[4];
		in.read(sig, 4);
		if (strncmp(sig, "MTrk", 4) != 0)
			throw "Not a valid MIDI track: \"MTrk\" signature not found.";
		eventsCol.clear();
		long long remaining = getUint32_t(in);
#ifdef DEBUG
		cerr << "# Remaining bytes: " << remaining << endl;
#endif
		tick_t globalTicks = 0;
		uint8_t rawbuf[260];
		rawbuf[0] = 0xFF;
		while (remaining > 0) {
			uint8_t x;

			/* Read delta ticks. */
			vector<uint8_t> deltav;
			do
			{
				in.read((char*) &x, 1);
				deltav.push_back(x);
				--remaining;
			} while ((deltav.back() & (1u << 7)) != 0);
			tick_t deltaTicks = 0;
			for (unsigned i = 0; i < deltav.size(); ++i) {
				deltaTicks <<= 7;
				deltaTicks |= (deltav[i] & 0x7F);
			}
			globalTicks += deltaTicks;

			/* Read event description: command and data. */
			uint8_t previousCommand = rawbuf[0];
			int runningMode = 0;
			in.read((char *) rawbuf, 1);
			--remaining;
			// Check if we have a running event.
			if ((rawbuf[0] & 0x80) == (uint8_t) 0) {
				//rawbuf[0] = (rawbuf[0] & 0xF);
				//rawbuf[0] = (rawbuf[0] | (previousCommand & 0xF0));
				rawbuf[1] = rawbuf[0]; // pass this byte as command's argument
				rawbuf[0] = previousCommand; // treat this command the same as previous one
				runningMode = 1;
			}

			legacy_pevent ev(globalTicks, this);
			bool skip = false;
			ev.start = globalTicks;
			switch (rawbuf[0] >> 4) {
				case CMD_NOTE_OFF:
					in.read((char *) rawbuf + 1 + runningMode, 2 - runningMode);
					ev.setRaw(rawbuf, rawbuf + 3);
					remaining -= 2 - runningMode;

					{
						__typeof__(eventsCol.rbegin()) noteOn = findCorrespondingNoteOn(ev);
						if (noteOn == eventsCol.rend())
							cerr << "Warning: NOTE_OFF event without corresponding NOTE_ON." << endl;
						else {
							noteOn->pairVelocity = rawbuf[2];
							noteOn->len = ev.start - noteOn->start;
						}
					}
					break;
				case CMD_NOTE_ON:
					in.read((char *) rawbuf + 1 + runningMode, 2 - runningMode);
					ev.setRaw(rawbuf, rawbuf + 3);
					remaining -= 2 - runningMode;
					break;
				case CMD_KEY_AFTER_TOUCH:
				case CMD_CONTROL_CHANGE:
				case CMD_PITCH_WHEEL_CHANGE:
					in.read((char *) rawbuf + 1 + runningMode, 2 - runningMode);
					ev.setRaw(rawbuf, rawbuf + 3);
					remaining -= 2 - runningMode;
					break;
				case CMD_PROGRAM_CHANGE:
				case CMD_CHANNEL_AFTER_TOUCH:
					in.read((char *) rawbuf + 1 + runningMode, 1 - runningMode);
					ev.setRaw(rawbuf, rawbuf + 2);
					remaining -= 1 - runningMode;
					break;
				case CMD_META_EVENT:
					in.read((char *) rawbuf + 1 + runningMode, 2 - runningMode);
					remaining -= 2 - runningMode;
					in.read((char *) rawbuf + 3, rawbuf[2]);
					if (rawbuf[1] == META_TEMPO_CHANGE) {
						// Handle tempo change.
						uint32_t mspq = getUint24_t(rawbuf + 3); // microsec per quarter note
						if (ev.start == 0) {
							p_hasTempoMarkAt0 = true;
						}
						thisTracktempo.addTempoMark(ev.start, ((double) mspq) / ((double) tpq) * 0.000001);
					}
					remaining -= rawbuf[2];
					ev.setRaw(rawbuf, rawbuf + 3 + rawbuf[2]);
					break;
				default:
					cerr << "Warning: skipping unknown MIDI command type: " << (rawbuf[0] >> 4) << endl;
					in.read((char *) rawbuf + 1, 1);
					remaining -= 1;
					skip = true;
					break;
			}
#ifdef DEBUG
			if (! skip) {
				char buf[256];
				ev.getDescription(buf, 256);
				cerr << "# " << buf << endl;
				cerr << "remaining bytes: " << remaining << endl;
			}
#endif
			if (! skip) {
				/* Check for NOTE_ON events with velocity = 0 which are actually NOTE_OFF events. */
				if (ev.getCommand() == CMD_NOTE_ON && ev.getNoteVelocity() == 0) {
					__typeof__(eventsCol.rbegin()) noteOn = findCorrespondingNoteOn(ev);
					if (noteOn == eventsCol.rend())
						cerr << "Warning: NOTE_ON (velocity = 0) event without corresponding NOTE_ON."
						     << endl;
					else {
						noteOn->pairVelocity = DEFAULT_VELOCITY;
						noteOn->len = ev.start - noteOn->start;
					}
				} else {
					if (ev.getCommand() != CMD_NOTE_OFF)
					{
						if (! (ev.getCommand() == CMD_META_EVENT &&
								ev.getMetaCommand() == META_TEMPO_CHANGE)) {
							eventsCol.push_back(ev);
							assert (eventsCol.back() == ev);
						}
					}
				}
			}
		}
#ifdef DEBUG
		cerr << "# Debug: Finished reading a track. Here are the events:" << endl;
		for (__typeof__(eventsCol.begin()) it = eventsCol.begin(); it != eventsCol.end(); it++) {
			assert (it->getCommand() != CMD_NOTE_OFF);
			cerr << "# Event type: " << cmd2str[(int) it->getCommand()] << "(" << (int) it->getCommand() << ")";
			cerr << ", start: " << it->start;
			if (it->getCommand() == CMD_NOTE_ON)
					cerr << ", duration: " << it->len
					     << ", velocity of a paired NOTE_OFF: " << (int) it->pairVelocity;
			cerr << endl;
		}
#endif
	}


	int save(ostream &out) {
		vector<legacy_pevent> saveEvents(eventsCol.begin(), eventsCol.end());

		for (__typeof__(eventsCol.begin()) it = eventsCol.begin(); it != eventsCol.end(); it++) {
			if (it->isNote()) {
				legacy_pevent noteOff;
				noteOff.assignNoteOff(*it);
				saveEvents.push_back(noteOff);
			}
		}

		tick_t tempoMark = 0;
		do
		{
			double secPerTick = thisTracktempo.readTempoMark(tempoMark);
			double secPerQuarter = secPerTick * ((double) tpq);
			uint32_t msecPerQuarterInt= (uint32_t) floor((1000000 * secPerQuarter) + 0.5);

			legacy_pevent tempoChange(tempoMark, this);
			tempoChange.assignTempoMark(msecPerQuarterInt);
			saveEvents.push_back(tempoChange);

			tempoMark = thisTracktempo.nextTempoMarkAfter(tempoMark);
		} while (tempoMark != 0);

		stable_sort(saveEvents.begin(), saveEvents.end(), startCmp);

		uint32_t totalLen = 4;
		out.write("MTrk", 4);
		printUint32_t(0xdeadbeef, out); // print length placeholder
		totalLen += 4;

		for (__typeof__(saveEvents.begin()) it = saveEvents.begin(),
			                prev = saveEvents.begin(); it != saveEvents.end(); it++) {
			tick_t delta_tick = it->start - prev->start;
			uint32_t delta = (uint32_t) delta_tick;
			vector<uint8_t> deltav;
			do
			{
				if (delta >= 0x80) {
					int msb = 31 - __builtin_clz(delta);
					deltav.push_back((delta >> (msb - 7)) | 0x80);
					delta = (delta & ((1 << (msb - 7 + 1)) - 1));
				}
				else {
					deltav.push_back(delta);
					delta = 0;
				}
			} while (delta != 0);
			for (unsigned i = 0; i < deltav.size(); ++i)
				out.put((char) deltav[i]);

			totalLen += deltav.size();
			totalLen += it->getRawLen();
			out << (*it);
		}
		return totalLen;
	}

	virtual legacy_pevent & events(unsigned int i) {
		return eventsCol[i];
	}

	virtual const legacy_pevent & events(unsigned int i) const {
		return eventsCol[i];
	}

	virtual const tracktempo & getTrackTempo() const {
		return thisTracktempo;
	}

	virtual void setTrackTempo(const tracktempo &t) {
		thisTracktempo = t;
	}

	virtual unsigned int eventCount() const {
		return eventsCol.size();
	}

	bool hasTempoMarkAt0() const {
		return p_hasTempoMarkAt0;
	}

	friend class pmidi;

	private:

	vector<legacy_pevent> eventsCol;
	tracktempo thisTracktempo;
	uint16_t tpq;
	bool p_hasTempoMarkAt0;

	__typeof__(eventsCol.rbegin()) findCorrespondingNoteOn(const legacy_pevent& ev) {
		__typeof__(eventsCol.rbegin()) noteOn = eventsCol.rbegin();
		for (; noteOn != eventsCol.rend(); noteOn++)
			if (noteOn->isNote() && noteOn->equalChannelAndPitch(ev)) break;
		return noteOn;
	}
};

class pmidi {
	public:

	pmidi(const char * fn) {
		uint8_t header[14];
		ifstream in(fn, ifstream::in);

		in.read((char *) header, 14);
		if (strncmp((char *) header, "MThd", 4) != 0)
			throw "Not a MIDI file!";
		if (strncmp((char *) header + 4, "\0\0\0\6", 4) != 0)
			throw "Incorrect header length.";

#ifdef DEBUG
		uint16_t ff = getFileFormat();
		cerr << "# File format: " << FILE_FORMAT[ff] << endl;
#endif

		uint16_t tracksCount = getUint16_t(header, 10);
		t.resize(tracksCount);

		tpq = getUint16_t(header, 12);

#ifdef DEBUG
		cerr << "# Tracks count: " << tracksCount << endl;
		cerr << "# Ticks per quarter note: " << tpq << endl;
#endif

		for (unsigned i = 0; i < tracksCount; ++i)
			t[i].load(in, tpq);

		/* Check declared file format. If it is 1, the file is synchronous. */
		if (getUint16_t(header, 8) == 1) {
			tracktempo mergedTracktempo(0.0);
			for (unsigned i = 0; i < tracksCount; ++i) {
				tick_t tempoMark = 0;
				do
				{
					double secPerTick = t[i].getTrackTempo().readTempoMark(tempoMark);
					if (t[i].hasTempoMarkAt0() || (tempoMark != 0))
						mergedTracktempo.addTempoMark(tempoMark, secPerTick);
					tempoMark = t[i].getTrackTempo().nextTempoMarkAfter(tempoMark);
				} while (tempoMark != 0);
			}
			for (unsigned i = 0; i < tracksCount; ++i)
				t[i].thisTracktempo = mergedTracktempo;
		}

		in.close();
	}

	~pmidi() {
	}

	bool save(const char * fn) {
		ofstream out(fn, ios_base::out | ios_base::binary);
		if (out.fail()) return false;
		printHeader(out);
		int lengths[trackCount()];
		for (unsigned i = 0; i < trackCount(); ++i)
			lengths[i] = t[i].save(out);
		out.seekp(14, ios_base::beg);
		for (unsigned i = 0; i < trackCount(); ++i) {
			out.seekp(4, ios_base::cur);
			printUint32_t(lengths[i] - 8, out);
			out.seekp(lengths[i] - 8, ios_base::cur);
		}

		out.close();
		return out.fail();
	}

	ptrack & tracks(unsigned int i) {
		return t[i];
	}

	const ptrack & tracks(unsigned int i) const {
		return t[i];
	}

	unsigned int trackCount() const {
		return t.size();
	}

	uint16_t getFileFormat() const {
		if (trackCount() <= 1)
			return 0;
		uint16_t ff = 1;
		for (unsigned i = 1; i < trackCount(); ++i) {
			if (t[i].getTrackTempo() != t[0].getTrackTempo()) {
				ff = 2;
				break;
			}
		}
		return ff;
	}

	/* Ticks per quarter note. */
	uint16_t tpq;

	private:

	vector<ptrack> t;
	void printHeader(ostream &out) {
		out.write("MThd\0\0\0\6", 8);
		if (trackCount() == 0)
			throw "No tracks! Cannot save a file.";
		else if (trackCount() == 1)
			printUint16_t(1, out);
		else {
			printUint16_t(getFileFormat(), out);
		}
		printUint16_t(trackCount(), out);
		printUint16_t(tpq, out);
	}

};


void midi_file_read(midi & m, const char * fn) {
	while (m.trackCount() > 0) m.delTrack(0);
	pmidi mymidi(fn);
	m.setTicksPerQuaterNote(mymidi.tpq);
	for (unsigned i = 0; i < mymidi.trackCount(); ++i) {
		ptrack& currTrack = mymidi.tracks(i);
		track newTrack;
		newTrack.setTrackTempo(currTrack.getTrackTempo());
		for (unsigned j = 0; j < currTrack.eventCount(); ++j) {
			uint8_t buf[260];
			const legacy_pevent& srcEvent = currTrack.events(j);
			int buflen = srcEvent.getBytes(buf, 260);
			if (srcEvent.isNote()) {
				pnote * tmpNote = new pnote;
				tmpNote->setBytes(buf, buflen);
				// create temporary note-off event
				legacy_pevent noteOff;
				noteOff.assignNoteOff(srcEvent);
				// dump its bytes
				buflen = noteOff.getBytes(buf, 260);
				// and assign them to the new note event
				tmpNote->setNoteOffBytes(buf, buflen);

				newTrack.addNote((note*) tmpNote, srcEvent.getStartTicks(), srcEvent.getDurationTicks());
			} else {
				pevent * tmpEvent = new pevent;
				tmpEvent->setBytes(buf, buflen);

				newTrack.addEvent((event*) tmpEvent, srcEvent.getStartTicks());
			}
		}
		m.addTrack(newTrack);
	}
}
