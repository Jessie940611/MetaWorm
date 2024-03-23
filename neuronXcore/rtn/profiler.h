#ifndef Stopwatch_h
#define Stopwatch_h

#include <numeric>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace utility
{

	// https://www.modernescpp.com/index.php/the-three-clocks
	template <typename T>
	void printRatio()
	{
		std::cout << "  precision: " << T::num << "/" << T::den << " second " << std::endl;
		typedef typename std::ratio_multiply<T, std::kilo>::type MillSec;
		typedef typename std::ratio_multiply<T, std::mega>::type MicroSec;
		std::cout << std::fixed;
		std::cout << "             " << static_cast<double>(MillSec::num) / MillSec::den << " milliseconds " << std::endl;
		std::cout << "             " << static_cast<double>(MicroSec::num) / MicroSec::den << " microseconds " << std::endl;
	}

	class Stopwatch
	{
	private:
		std::chrono::time_point<std::chrono::system_clock> start;
		std::chrono::time_point<std::chrono::system_clock>  _stopwatch() const
		{
			return std::chrono::system_clock::now();
		}

	public:
		Stopwatch() { reset(); }
		void reset() { start = _stopwatch(); }

		//< return second unit
		double read() const
		{
			std::chrono::duration<double> diff = _stopwatch() - start;
			return diff.count();
		}
	};

	class FrameRateDetector
	{
	private:
		double _frametime; //< in ms unit
		double _framerate; //< 1000/_frametime
		Stopwatch _stopwatch;
	public:
		FrameRateDetector() :_frametime(1e20), _stopwatch() { _framerate = 1000. / _frametime; }
		void start() {}
		void stop()
		{
			_frametime = _stopwatch.read() * 1000.; //< in ms
			_framerate = 1000. / _frametime;
		}
		double framerate() const { return _framerate; }
		double frametime() const { return _frametime; }
	};

	struct ProfilerEntry
	{
		std::string name; //< profiler entry name, for example "Update", "Render"
		std::vector<double> samples; //< run time samples for this entry

		ProfilerEntry()
		{ }

		ProfilerEntry(std::string _name, double v, double avg = 0.0)
			: name(_name)
			, samples(1, v)
			, avgTime(avg)
		{
		}

		std::string str()
		{
			auto & v = samples;
			auto lambda = [&v](double a, double b) {return a + b / v.size(); };
			avgTime = std::accumulate(v.begin(), v.end(), 0.0, lambda);

			std::ostringstream ss;
			ss << std::left << std::setw(24) << name << "\t" << std::setprecision(9) << std::fixed << avgTime * 1000.f << " ms";
			return ss.str();
		}
	private:
		double avgTime = 0.0;
	};

	class CPUProfiler
	{
	private:
		static std::unordered_map<std::string, ProfilerEntry> ProfilerData;

	public:
		static void begin() { ProfilerData.clear(); }

		static std::string end()
		{
			std::ostringstream ss;
			ss << "\n----------Profiler Info------------\n";
			for (auto it = ProfilerData.begin(); it != ProfilerData.end(); ++it)
			{
				ss << it->second.str() << std::endl;
			}
			return ss.str();
		}

	private:
		std::string _name;
		Stopwatch _stopWatch;

	public:
		CPUProfiler(std::string name)
			: _name(name)
			, _stopWatch()
		{ }

		~CPUProfiler()
		{
			double deta = _stopWatch.read();
			auto ret = ProfilerData.find(_name);
			if (ret == ProfilerData.end())
			{
				ProfilerEntry entry = { _name, deta, 0.0 };
				ProfilerData[_name] = entry;
			}
			else
			{
				ProfilerData[_name].samples.push_back(deta);
			}
		}
	};

}
#endif
