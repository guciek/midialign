/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "midi_file_read.hpp"

void midi_file_read(midi & m, const char * fn) {
	while (m.trackCount() > 0) m.delTrack(0);
	m.setTicksPerQuaterNote(100);
}
