#include "Timestamp.h"
#include <chrono>
#include <cstdio>
#include <cstring>

using namespace std;
using namespace chrono;

namespace
{
	// http://mingw.5.n7.nabble.com/Porting-localtime-r-and-gmtime-r-td12634.html
	// Windows variants of gmtime and localtime are thread-safe because they use thread-local storage
#if !HAVE_TIME_R

	struct tm* gmtime_r(time_t* _clock, struct tm* _result) {
		struct tm* p = gmtime(_clock);

		if (p)
			*(_result) = *p;

		return p;
	}

#endif

	void populateTimestamp(char* _out) {
		auto now = system_clock::now();
		auto ms = duration_cast<milliseconds>(now.time_since_epoch());
		auto fractional_seconds = ms.count() % 1000;

		time_t tt = system_clock::to_time_t(now);
		tm utc_tm;
		gmtime_r(&tt, &utc_tm);
		strftime(_out, Timestamp::MAX_LENGTH, "%Y-%m-%dT%H:%M:%S.000", &utc_tm);

		static const char* FRACTAL_FORMAT = "%03d";
		char fractals[4] = {0};
		sprintf(fractals, FRACTAL_FORMAT, fractional_seconds);
		memcpy(&_out[Timestamp::FRACTAL_POS], fractals, 3);
	}
}

Timestamp::Timestamp() {
	populateTimestamp(value);
}

Timestamp::~Timestamp() {
}
