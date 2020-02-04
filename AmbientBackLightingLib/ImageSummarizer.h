#pragma once
#include "stdafx.h"
#include <vector>
#include "Color.h"

class EXPORT ColorSampler
{
	ULONG R = 0;
	ULONG G = 0;
	ULONG B = 0;

	unsigned int SampleCount = 0;

public:
	bool UseRootMeanSquare = false;
	
	unsigned int GetSampleCount() const { return SampleCount; }

	void AddSample(const Color& Sample);

	void ClearSamples();
	
	Color GetAverageColor() const;
};

//TODO: this should probably be templated with something that has a distance method
struct EXPORT VoronoiCell
{
	Color Seed;

	VoronoiCell(Color InSeed) { Seed = InSeed; }

	double GetDistance(const Color& Sample) const
	{
		return Seed.GetDistance(Sample);
	}
};

//TODO: we may need to do RMS here. just change out add sample and get average color implementations
class EXPORT IImageSummarizer
{
public:
	virtual void AddSample(const Color& Sample) = 0;

	virtual void ClearSamples() = 0;
	
	virtual Color GetColor() const = 0;
};

class EXPORT VoronoiImageSummarizer : public IImageSummarizer
{
	std::vector<std::pair<VoronoiCell, ColorSampler>> Cells;

public:
	VoronoiImageSummarizer(const std::vector<Color>& Colors);

	void AddSample(const Color& Sample) override;

	void ClearSamples() override;

	Color GetColor() const override;
};

class EXPORT AverageImageSummarizer : public IImageSummarizer
{
	ColorSampler Sampler;

public:
	AverageImageSummarizer(bool UseRootMeanSquare) { Sampler.UseRootMeanSquare = UseRootMeanSquare; }

	void AddSample(const Color& Sample) override;

	void ClearSamples() override;

	Color GetColor() const override;
};

