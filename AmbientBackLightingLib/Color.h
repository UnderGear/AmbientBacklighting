#pragma once

#include "stdafx.h"
#include <cmath>

struct EXPORT Color
{
	ULONG R = 0;
	ULONG G = 0;
	ULONG B = 0;

	Color() {}

	Color(ULONG InR, ULONG InG, ULONG InB)
	{
		R = InR;
		G = InG;
		B = InB;
	}

	double GetDistance(const Color& Other) const
	{
		auto DeltaR = Other.R - R;
		auto DeltaG = Other.G - G;
		auto DeltaB = Other.B - B;

		return sqrt(DeltaR * DeltaR + DeltaG * DeltaG + DeltaB * DeltaB);
	}
};

