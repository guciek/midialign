/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

#include "midi.hpp"
#include "dump_midi.hpp"
#include "align_midi_events.hpp"
#include "find_nice_tempo.hpp"
#include "midi_file_read.hpp"
#include "midi_file_write.hpp"

void option_auto(midi & m) {
	tracktempo t = find_nice_tempo(0, 0);
	align_midi_events(m, t);
}

void option_bpm(midi & m, const char * param1) {
	int bpm = atoi(param1);
	int mult = 1;
	int x = string(param1).find('x');
	if (x >= 0) mult = atoi(param1+x+1);
	if ((bpm*mult < 1) || (bpm*mult > 1000000))
		throw "Invalid parameter for '-bpm'!";
	#ifdef DEBUG
		cerr << "# Aligning to a constant grid of "
			<< bpm*mult << " ticks/second..." << endl;
	#endif
	align_midi_events(m, tracktempo(60.0/(bpm*mult)));
	if (mult*bpm < 8*60) {
		while (mult*bpm < 8*60) mult *= 2;
		align_midi_events(m, tracktempo(60.0/(bpm*mult)));
	}
	m.setTicksPerQuaterNote(mult);
}

bool run(char ** params) {
	if ((!params[0]) || (!params[0][0]) || (params[0][0] == '-')) {
		cerr << "\nUsage:\n" <<
			"\tmidialign <input.midi> [ ... options ... ] <output.midi>\n" <<
			"\nOptions:\n" <<
			"\t-auto                         find a matching tempo and align events\n" <<
			"\t-bpm <bpm>[x<multiplier>]     align all events to this tempo\n" <<
			"\n";
		return false;
	}
	midi m;
	bool read = false;
	bool saved = false;
	while (params[0]) {
		string p = params[0];
		if (p[0] == '-') {
			if (p == "-auto") {
				option_auto(m);
				saved = false;
			} else if (p == "-bpm") {
				params++;
				if (!params[0])
					throw "Option '-bpm' requires a parameter!";
				option_bpm(m, params[0]);
				saved = false;
			} else {
				cerr << "Unknown option '" << p << "'!" << endl;
				return false;
			}
		} else if (read) {
			#ifdef DEBUG
				cerr << "# Saving output file '" << p << "'..." << endl;
			#endif
			midi_file_write(m, p.c_str());
			saved = true;
		} else {
			#ifdef DEBUG
				cerr << "# Reading file'" << p << "'..." << endl;
			#endif
			try {
				midi_file_read(m, p.c_str());
				read = true;
			} catch(const char * e) {
				cerr << "Could not open '" << p << "': " << e << endl;
				return false;
			}
		}
		params++;
	}
	if (!saved) dump_midi_stderr(m);
	return true;
}

int main(int argc, char ** argv) {
	#ifdef DEBUG
		cerr << "# Warning! This is a debug build." << endl;
	#endif
	try {
		return run(argv+1) ? 0 : 1;
	} catch(const char * e) {
		cerr << "Error! " << e << endl;
	} catch(...) {
		cerr << "Error! (unknown reason)" << endl;
	}
}
