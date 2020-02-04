#include "stdafx.h"
#include "ImageSummarizer.h"

void ColorSampler::AddSample(const Color& Sample)
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

void ColorSampler::ClearSamples()
{
	R = 0;
	G = 0;
	B = 0;

	SampleCount = 0;
}

Color ColorSampler::GetAverageColor() const
{
	if (UseRootMeanSquare)
	{
		return
		{
			(ULONG)sqrt(R / SampleCount),
			(ULONG)sqrt(G / SampleCount),
			(ULONG)sqrt(B / SampleCount)
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

VoronoiImageSummarizer::VoronoiImageSummarizer(const std::vector<Color>& Colors)
{
	Cells.empty();
	Cells.reserve(Colors.size());

	for (auto& Color : Colors)
	{
		auto Cell = VoronoiCell{ Color };
		ColorSampler Sampler;
		Cells.push_back(std::make_pair(Cell, Sampler));
	}
}

void VoronoiImageSummarizer::AddSample(const Color& Sample)
{
	double ClosestDistance = DBL_MAX;
	ColorSampler* BestSampleAverager = nullptr;

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

void VoronoiImageSummarizer::ClearSamples()
{
	for (auto& Pair : Cells)
	{
		Pair.second.ClearSamples();
	}
}

Color VoronoiImageSummarizer::GetColor() const
{
	unsigned int HighestSampleCount = 0;
	const ColorSampler* HighestSampleAverager = nullptr;

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

void AverageImageSummarizer::AddSample(const Color& Sample)
{
	Sampler.AddSample(Sample);
}

void AverageImageSummarizer::ClearSamples()
{
	Sampler.ClearSamples();
}

Color AverageImageSummarizer::GetColor() const
{
	return Sampler.GetAverageColor();
}

