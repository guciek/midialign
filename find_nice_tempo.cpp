/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
using namespace std;

#include "find_nice_tempo.hpp"

double avg(const vector<double> & v) {
	double sum = 0.0;
	if (v.size() < 1) return 0.0;
	for (unsigned int i = 0; i < v.size(); i++) sum += v[i];
	return sum/v.size();
}

double stddev(const vector<double> & v) {
	double a = avg(v);
	double sum = 0.0;
	for (unsigned int i = 0; i < v.size(); i++) sum += (v[i]-a)*(v[i]-a);
	return sqrt(sum/v.size());
}

double median(vector<double> & v) {
	if (v.size() < 1) return 0.0;
	sort(v.begin(), v.end());
	return v[v.size()/2];
}

double maxoccur(vector<double> & v) {
	if (v.size() < 1) return 0.0;
	sort(v.begin(), v.end());
	#ifdef DEBUG_MAX
		cerr << "# Maxoccur:";
		for (unsigned int i = 0; i < v.size(); i++)
			cerr << " " << v[i];
		cerr << endl;
	#endif
	double dist = 0.01;
	unsigned int afterdist = 0;
	unsigned int max_occur = 0;
	double max_val = 0.0;
	for (unsigned int i = 0; i < v.size(); i++) {
		while
			(
				(afterdist < i+v.size()) &&
				(
					((afterdist < v.size()) ? 0.0 : 1.0) +
						v[afterdist%v.size()] < v[i]+dist
				)
			)
				afterdist++;
		#ifdef DEBUG_MAX
			cerr << "# " << v[i] << " x" << (afterdist-i) << endl;
		#endif
		if (afterdist-i > max_occur) {
			max_occur = afterdist-i;
			max_val = v[((afterdist+i)/2) % v.size()];
		}
	}
	return max_val;
}

double cyclic_median(vector<double> & v) {
	double md1 = median(v);
	double sd1 = stddev(v);
	for (unsigned int i = 0; i < v.size(); i++)
		if (v[i] < 0.5) v[i] += 1.0;
	double md2 = median(v);
	double sd2 = stddev(v);
	if (sd1 < sd2) return md1;
	return md2-floor(md2);
}

double find_nice_constant_bpm_sorted(const vector<double> & evs) {
	#ifdef DEBUG
		cerr << "# Find nice constant bpm" << endl;
		cerr << "# Event times:";
		for (unsigned int i = 0; i < evs.size(); i++)
			cerr << " " << evs[i];
		cerr << endl;
	#endif
	vector<double> logs;
	for (unsigned int i = 0; i+1 < evs.size(); i++) {
		for (unsigned int j = i+1; j < evs.size(); j++) {
			if (evs[j]-evs[i] < 0.001) continue;
			double a = log2(evs[j]-evs[i]);
			logs.push_back(a-floor(a));
			if (evs[j]-evs[i] > 1.0) break;
		}
		if ((i > 0) && (evs[i]-evs[i-1] < 0.001)) continue;
	}
	if (logs.size() < 1) return 60.0;
	double maxoccur_bpm = 120.0/pow(2.0, maxoccur(logs));
	#ifdef DEBUG
		double median_bpm = 120.0/pow(2.0, cyclic_median(logs));
		cerr << "# 'cyclic_median' returned " << median_bpm << " bpm" << endl;
		cerr << "# 'maxoccur' returned " << median_bpm << " bpm" << endl;
	#endif
	return maxoccur_bpm;
}

double find_nice_constant_bpm(const double * event_times,
		unsigned int event_count) {
	vector<double> evs;
	for (unsigned int i = 0; i < event_count; i++)
		evs.push_back(event_times[i]);
	sort(evs.begin(), evs.end());
	return find_nice_constant_bpm_sorted(evs);
}
