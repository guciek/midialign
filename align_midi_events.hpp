/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#ifndef ALIGN_MIDI_EVENTS_H
#define ALIGN_MIDI_EVENTS_H

#include "midi.hpp"

void align_midi_events(midi & m, tick_t tickPerQuarterNote,
	const tracktempo & newtempo);

#endif
