#include "stdafx.h"
#include "ImageSummarizer.h"

void RGBSampler::AddSample(const ColorRGB& Sample)
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

void RGBSampler::ClearSamples()
{
	R = 0;
	G = 0;
	B = 0;

	SampleCount = 0;
}

ColorRGB RGBSampler::GetAverageColor() const
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

void LUVSampler::AddSample(const ColorRGB& Sample)
{
	auto SampleLUV = Sample.ToXYZ().ToLUV();
	L += SampleLUV.L;
	U += SampleLUV.U;
	V += SampleLUV.V;

	++SampleCount;
}

void LUVSampler::ClearSamples()
{
	L = 0;
	U = 0;
	V = 0;

	SampleCount = 0;
}

ColorRGB LUVSampler::GetAverageColor() const
{
	ColorLUV Average = 
	{
		(float)(L / SampleCount),
		(float)(U / SampleCount),
		(float)(V / SampleCount)
	};
	
	return Average.ToXYZ().ToRGB();
}

VoronoiImageSummarizer::VoronoiImageSummarizer(const std::vector<ColorRGB>& Colors)
{
	Cells.empty();
	Cells.reserve(Colors.size());

	for (auto& Color : Colors)
	{
		auto Cell = VoronoiCell{ Color };
		RGBSampler Sampler;
		Cells.push_back(std::make_pair(Cell, Sampler));
	}
}

void VoronoiImageSummarizer::AddSample(const ColorRGB& Sample)
{
	double ClosestDistance = DBL_MAX;
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

void VoronoiImageSummarizer::ClearSamples()
{
	for (auto& Pair : Cells)
	{
		Pair.second.ClearSamples();
	}
}

ColorRGB VoronoiImageSummarizer::GetColor() const
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

void AverageImageSummarizer::AddSample(const ColorRGB& Sample)
{
	Sampler.AddSample(Sample);
}

void AverageImageSummarizer::ClearSamples()
{
	Sampler.ClearSamples();
}

ColorRGB AverageImageSummarizer::GetColor() const
{
	return Sampler.GetAverageColor();
}

void AverageLUVImageSummarizer::AddSample(const ColorRGB& Sample)
{
	Sampler.AddSample(Sample);
}

void AverageLUVImageSummarizer::ClearSamples()
{
	Sampler.ClearSamples();
}

ColorRGB AverageLUVImageSummarizer::GetColor() const
{
	return Sampler.GetAverageColor();
}

