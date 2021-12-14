export module Profiler;

import std.core;

namespace Profiler
{
	struct StackFrameResults
	{
		uint32_t CallCount = 0;
		std::chrono::duration<double> TotalDuration = {};
		std::string_view ProfileName;

		void Print() const
		{
			std::cout << "Profile Name: " << ProfileName << ", CallCount: " << CallCount << ", Total Duration: " << TotalDuration << '\n';
		}

		void Reset()
		{
			CallCount = 0;
			TotalDuration = {};
		}
	};


	static struct FrameProfiler* Profiler = nullptr;

	export struct FrameProfiler
	{
		std::unordered_map<std::size_t, StackFrameResults> Results;

		FrameProfiler()
		{
			Profiler = this;
		}

		void Print() const
		{
			std::cout << "Frame Profiler Results:" << '\n';
			for (auto& Pair : Results)
			{
				Pair.second.Print();
			}
		}

		void Reset()
		{
			for (auto& Pair : Results)
			{
				Pair.second.Reset();
			}
		}
	};


	export struct StackFrameProfile
	{
		StackFrameProfile(std::string_view InProfileName, std::size_t InId)
			: ProfileName(InProfileName)
			, SampleStart(std::chrono::high_resolution_clock::now())
			, Id(InId)
		{}

		~StackFrameProfile()
		{
			const auto SampleEnd = std::chrono::high_resolution_clock::now();
			const auto TimeElapsed = SampleEnd - SampleStart;

			auto FoundIter = Profiler->Results.find(Id);
			if (FoundIter == Profiler->Results.end())
			{
				//not found. add a new one
				Profiler->Results[Id] = { 1, TimeElapsed, ProfileName };
			}
			else
			{
				FoundIter->second.CallCount++;
				FoundIter->second.TotalDuration += TimeElapsed;
			}
		}

		std::string_view ProfileName;
		std::chrono::time_point<std::chrono::high_resolution_clock> SampleStart = {};
		std::size_t Id;

	};

}