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
			static constexpr auto FormatText = "Profile: {}, CallCount: {}, Total Duration {}";

			//TODO: we should configure some options on where we write these logs. standard out, VS output window, file?

			std::puts(std::format(FormatText, ProfileName, CallCount, TotalDuration).c_str());
		}

		void Reset()
		{
			CallCount = 0;
			TotalDuration = {};
		}
	};

	struct StackFrameResult
	{
		std::string_view ProfileName;
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime{};
		std::chrono::time_point<std::chrono::high_resolution_clock> EndTime{};

		StackFrameResult() = default;
		StackFrameResult(std::string_view InProfileName, std::chrono::time_point<std::chrono::high_resolution_clock> InStartTime)
			: ProfileName{ InProfileName }, StartTime{ InStartTime }
		{}

		void Print() const
		{
			static constexpr auto FormatText = "Profile: {}, Total Duration {}";

			//TODO: we should configure some options on where we write these logs. standard out, VS output window, file?
			std::puts(std::format(FormatText, ProfileName, EndTime - StartTime).c_str());
		}
	};


	static struct FrameProfiler* Profiler = nullptr;

	struct FrameTreeNode
	{
		StackFrameResult Value;
		std::vector<FrameTreeNode> Children;

		FrameTreeNode() = default;
		FrameTreeNode(StackFrameResult InValue) : Value{ InValue } {}

		void Print() const
		{
			Value.Print();
			for (auto& Child : Children)
			{
				Child.Print();
			}
		}
	};

	export struct FrameProfiler
	{
		std::unordered_map<std::string, StackFrameResults> Results;

		//TODO: frame tree, stack of references?

		FrameTreeNode FrameTree;
		std::stack<FrameTreeNode*> StackFrames;

		FrameProfiler()
		{
			Profiler = this;
			
			FrameTree = { { "Root", std::chrono::high_resolution_clock::now() } };
			StackFrames.push(&FrameTree);


			//TODO: push something onto the frame tree and stack
		}

		void AddFrame(std::string_view ProfileName, auto StartTime)
		{
			auto& AddedFrame = StackFrames.top()->Children.emplace_back(FrameTreeNode{ StackFrameResult{ ProfileName, StartTime } });
			StackFrames.push(&AddedFrame);
		}

		void FinalizeFrame(std::string_view ProfileName, auto EndTime)
		{
			auto* EndedFrame = StackFrames.top();
			EndedFrame->Value.EndTime = EndTime;

			auto TimeElapsed = EndTime - EndedFrame->Value.StartTime;

// 			auto FoundIter = Results.find(ProfileName);
// 			if (FoundIter == Results.end())
// 			{
// 				//not found. add a new one
// 				Results[ProfileName] = { 1, TimeElapsed, ProfileName };
// 			}
// 			else
// 			{
// 				FoundIter->second.CallCount++;
// 				FoundIter->second.TotalDuration += TimeElapsed;
// 			}

			StackFrames.pop();
		}

		void Print() const
		{
			std::puts("Frame Profiler Results:");
			FrameTree.Print();
// 			for (auto& [Id, Calls] : Results)
// 			{
// 				Calls.Print();
// 			}
		}

		void Reset()
		{
// 			for (auto& [Id, Calls] : Results)
// 			{
// 				Calls.Reset();
// 			}
		}
	};


	export struct StackFrameProfile
	{
		StackFrameProfile(std::string_view InProfileName)
			: ProfileName(InProfileName)
		{
			Profiler->AddFrame(ProfileName, std::chrono::high_resolution_clock::now());
		}

		~StackFrameProfile()
		{
			Profiler->FinalizeFrame(ProfileName, std::chrono::high_resolution_clock::now());

// 			const auto SampleEnd = std::chrono::high_resolution_clock::now();
// 			const auto TimeElapsed = SampleEnd - SampleStart;
// 
// 			auto FoundIter = Profiler->Results.find(Id);
// 			if (FoundIter == Profiler->Results.end())
// 			{
// 				//not found. add a new one
// 				Profiler->Results[Id] = { 1, TimeElapsed, ProfileName };
// 			}
// 			else
// 			{
// 				FoundIter->second.CallCount++;
// 				FoundIter->second.TotalDuration += TimeElapsed;
// 			}
		}

		std::string_view ProfileName;
	};

}