#pragma once
#include "stdafx.h"
#include <vector>
#include "Color.h"

class EXPORT IColorSampler
{
public:
	virtual unsigned int GetSampleCount() const = 0;

	virtual void AddSample(const ColorRGB& Sample) = 0;

	virtual void ClearSamples() = 0;

	virtual ColorRGB GetAverageColor() const = 0;
};

class EXPORT RGBSampler : public IColorSampler
{
	ULONG R = 0;
	ULONG G = 0;
	ULONG B = 0;

	unsigned int SampleCount = 0;

public:
	bool UseRootMeanSquare = false;
	
	unsigned int GetSampleCount() const override { return SampleCount; }

	void AddSample(const ColorRGB& Sample) override;

	void ClearSamples() override;
	
	ColorRGB GetAverageColor() const override;
};

class EXPORT LUVSampler : public IColorSampler
{
	double L = 0;
	double U = 0;
	double V = 0;

	unsigned int SampleCount = 0;

public:

	unsigned int GetSampleCount() const override { return SampleCount; }

	void AddSample(const ColorRGB& Sample) override;

	void ClearSamples() override;

	ColorRGB GetAverageColor() const override;
};

//TODO: this should probably be templated with something that has a distance method
struct EXPORT VoronoiCell
{
	ColorRGB Seed;

	VoronoiCell(ColorRGB InSeed) { Seed = InSeed; }

	double GetDistance(const ColorRGB& Sample) const
	{
		return Seed.GetDistance(Sample);
	}
};

//this could probably be templated with a subclass of IColorSampler
class EXPORT IImageSummarizer
{
public:
	virtual void AddSample(const ColorRGB& Sample) = 0;

	virtual void ClearSamples() = 0;
	
	virtual ColorRGB GetColor() const = 0;
};

//TODO: the voronoi one could probably be expressed as a collection of other image samplers.
class EXPORT VoronoiImageSummarizer : public IImageSummarizer
{
	std::vector<std::pair<VoronoiCell, RGBSampler>> Cells;

public:
	VoronoiImageSummarizer(const std::vector<ColorRGB>& Colors);

	void AddSample(const ColorRGB& Sample) override;

	void ClearSamples() override;

	ColorRGB GetColor() const override;
};

class EXPORT AverageImageSummarizer : public IImageSummarizer
{
	RGBSampler Sampler;

public:
	AverageImageSummarizer(bool UseRootMeanSquare) { Sampler.UseRootMeanSquare = UseRootMeanSquare; }

	void AddSample(const ColorRGB& Sample) override;

	void ClearSamples() override;

	ColorRGB GetColor() const override;
};

class EXPORT AverageLUVImageSummarizer : public IImageSummarizer
{
	LUVSampler Sampler;

public:

	void AddSample(const ColorRGB& Sample) override;

	void ClearSamples() override;

	ColorRGB GetColor() const override;
};

