export module AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Color;
import std.core;

//TODO: I think this is a good file to start considering C++20 concepts

export namespace ABL
{
	class IColorSampler
	{
	public:
		virtual unsigned int GetSampleCount() const = 0;

		virtual void AddSample(const ABL::ColorRGB& Sample) = 0;

		virtual void ClearSamples() = 0;

		virtual ABL::ColorRGB GetAverageColor() const = 0;
	};


	//TODO: for efficiency, let's stop using our color structs. why call ctors when we just want 3 channels?

	class RGBSampler : public IColorSampler
	{
		unsigned long R = 0;
		unsigned long G = 0;
		unsigned long B = 0;

		unsigned int SampleCount = 0;

	public:
		bool UseRootMeanSquare = false;
	
		unsigned int GetSampleCount() const override { return SampleCount; }

		void AddSample(const ABL::ColorRGB& Sample) override
		{
			if (UseRootMeanSquare)
			{
				R += Sample.R * Sample.R;
				G += Sample.G * Sample.G;
				B += Sample.B * Sample.B;
			}
			else
			{
				R += Sample.R;
				G += Sample.G;
				B += Sample.B;
			}

			++SampleCount;
		}

		void ClearSamples() override
		{
			R = 0;
			G = 0;
			B = 0;

			SampleCount = 0;
		}
	
		ABL::ColorRGB GetAverageColor() const override
		{
			if (UseRootMeanSquare)
			{
				return
				{
					static_cast<unsigned long>(std::sqrt(R / SampleCount)),
					static_cast<unsigned long>(std::sqrt(G / SampleCount)),
					static_cast<unsigned long>(std::sqrt(B / SampleCount))
				};
			}
			else
			{
				return
				{
					R / SampleCount,
					G / SampleCount,
					B / SampleCount
				};
			}
		}
	};

	//TODO: these conversions don't work for some reason.
	class LUVSampler : public IColorSampler
	{
		double L = 0;
		double U = 0;
		double V = 0;

		unsigned int SampleCount = 0;

	public:

		unsigned int GetSampleCount() const override { return SampleCount; }

		void AddSample(const ABL::ColorRGB& Sample) override
		{
// 			auto SampleLUV = XYZToLUV(RGBToXYZ(Sample));
// 			L += SampleLUV.L;
// 			U += SampleLUV.U;
// 			V += SampleLUV.V;

			++SampleCount;
		}

		void ClearSamples() override
		{
			L = 0;
			U = 0;
			V = 0;

			SampleCount = 0;
		}

		ABL::ColorRGB GetAverageColor() const override
		{
			ColorLUV Average =
			{
				(float)(L / SampleCount),
				(float)(U / SampleCount),
				(float)(V / SampleCount)
			};

			return ABL::ColorRGB{};//XYZToRGB(LUVToXYZ(Average));
		}
	};

	//TODO: this should probably be templated with something that has a distance method
	struct VoronoiCell
	{
		ABL::ColorRGB Seed;

		VoronoiCell(ABL::ColorRGB InSeed) { Seed = InSeed; }

		double GetDistance(const ABL::ColorRGB& Sample) const
		{
			return Seed.GetDistance(Sample);
		}
	};

	//this could probably be templated with a subclass of IColorSampler
	class IImageSummarizer
	{
	public:
		virtual void AddSample(const ABL::ColorRGB& Sample) = 0;

		virtual void ClearSamples() = 0;
	
		virtual ABL::ColorRGB GetColor() const = 0;
	};

	//TODO: the Voronoi one could probably be expressed as a collection of other image samplers.
	class VoronoiImageSummarizer : public IImageSummarizer
	{
		std::vector<std::pair<VoronoiCell, RGBSampler>> Cells;

	public:
		VoronoiImageSummarizer(const std::vector<ABL::ColorRGB>& Colors)
		{
			//TODO: I feel like this isn't really what we want to do here.
			auto NumRemoved = Cells.empty();
			Cells.reserve(Colors.size());

			for (auto& Color : Colors)
			{
				auto Cell = VoronoiCell{ Color };
				RGBSampler Sampler;
				Cells.push_back(std::make_pair(Cell, Sampler));
			}
		}

		void AddSample(const ABL::ColorRGB& Sample) override
		{
			double ClosestDistance = std::numeric_limits<double>::max();
			RGBSampler* BestSampleAverager = nullptr;

			//find the closest cell's sampler.
			for (auto& Pair : Cells)
			{
				auto SampleDistance = Pair.first.GetDistance(Sample);
				if (SampleDistance < ClosestDistance)
				{
					ClosestDistance = SampleDistance;
					BestSampleAverager = &Pair.second;
				}
			}

			if (BestSampleAverager == nullptr)
				return; //TODO: assert here?

			BestSampleAverager->AddSample(Sample);
		}

		void ClearSamples() override
		{
			for (auto& Pair : Cells)
			{
				Pair.second.ClearSamples();
			}
		}

		ABL::ColorRGB GetColor() const override
		{
			unsigned int HighestSampleCount = 0;
			const RGBSampler* HighestSampleAverager = nullptr;

			for (auto& Pair : Cells)
			{
				auto SampleCount = Pair.second.GetSampleCount();
				if (SampleCount > HighestSampleCount)
				{
					HighestSampleCount = SampleCount;
					HighestSampleAverager = &Pair.second;
				}
			}

			if (HighestSampleAverager == nullptr)
				return {}; //TODO: assert here?

			return HighestSampleAverager->GetAverageColor();
		}
	};

	class AverageImageSummarizer : public IImageSummarizer
	{
		RGBSampler Sampler;

	public:
		AverageImageSummarizer(bool UseRootMeanSquare) { Sampler.UseRootMeanSquare = UseRootMeanSquare; }

		void AddSample(const ABL::ColorRGB& Sample) override
		{
			Sampler.AddSample(Sample);
		}

		void ClearSamples() override
		{
			Sampler.ClearSamples();
		}

		ABL::ColorRGB GetColor() const override
		{
			return Sampler.GetAverageColor();
		}
	};

	class AverageLUVImageSummarizer : public IImageSummarizer
	{
		LUVSampler Sampler;

	public:

		void AddSample(const ABL::ColorRGB& Sample) override
		{
			Sampler.AddSample(Sample);
		}

		void ClearSamples() override
		{
			Sampler.ClearSamples();
		}

		ABL::ColorRGB GetColor() const override
		{
			return Sampler.GetAverageColor();
		}
	};

}

