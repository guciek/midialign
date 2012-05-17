/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
* - Szymon Kałasz                                                         *
**************************************************************************/

#ifndef MIDI_H
#define MIDI_H

#include <inttypes.h>

#include "tracktempo.hpp"

class event {
	public:
		virtual ~event() = 0;
		virtual bool isNote() const { return false; };
		virtual unsigned int getBytes(uint8_t *, unsigned int) const = 0;
		virtual void getDescription(char *, unsigned int) const = 0;
};

class note : public event {
	public:
		virtual ~note() = 0;
		virtual bool isNote() const { return true; };
		virtual unsigned int getNoteOffBytes(uint8_t *, unsigned int) const;
		virtual void getDescription(char *, unsigned int) const = 0;
};

class track {
	public:
		track();
		~track();

		unsigned int addEvent(event *, tick_t start);
		unsigned int addNote(note *, tick_t start, tick_t duration);
		void delEvent(unsigned int);

		unsigned int eventCount() const;
		event & events(unsigned int);
		const event & events(unsigned int) const;

		tick_t getEventTicks(unsigned int) const;
		double getEventSeconds(unsigned int) const;
		tick_t getNoteDurationTicks(unsigned int) const;
		double getNoteDurationSeconds(unsigned int) const;
		void setEventTicks(unsigned int, tick_t);
		void setNoteDurationTicks(unsigned int, tick_t);

		const tracktempo & getTrackTempo() const;
		void setTrackTempo(const tracktempo &);
	private:
		void * a;
		void * b;
};

class midi {
	public:
		midi();
		~midi();

		unsigned addTrack(const track &);
		void delTrack(unsigned int);

		unsigned int trackCount() const;
		track & tracks(unsigned int);
		const track & tracks(unsigned int) const;

		void setTicksPerQuaterNote(tick_t);
		tick_t getTicksPerQuaterNote() const;
	private:
		uint16_t a;
		void * b;
};

#endif
