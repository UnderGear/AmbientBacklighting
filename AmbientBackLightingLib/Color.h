#pragma once

#include "stdafx.h"
#include <cmath>

//https://www.easyrgb.com/en/math.php
class EXPORT CIE
{
public:
	static const float ReferenceX;
	static const float ReferenceY;
	static const float ReferenceZ;
};

//sRGB
struct EXPORT ColorRGB
{
	ULONG R = 0;
	ULONG G = 0;
	ULONG B = 0;

	ColorRGB() {}
	ColorRGB(ULONG InR, ULONG InG, ULONG InB) : R(InR), G(InG), B(InB) { }

	double GetDistance(const ColorRGB& Other) const
	{
		auto DeltaR = Other.R - R;
		auto DeltaG = Other.G - G;
		auto DeltaB = Other.B - B;

		return sqrt(DeltaR * DeltaR + DeltaG * DeltaG + DeltaB * DeltaB);
	}

	struct ColorXYZ ToXYZ() const;
};

//CIE XYZ
struct EXPORT ColorXYZ
{
	float X = 0;
	float Y = 0;
	float Z = 0;

	ColorXYZ() {}
	ColorXYZ(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) { }

	ColorRGB ToRGB() const;
	struct ColorLUV ToLUV() const;
};

//CIE L*uv, or more accurately CIELCH, CIE LChuv, CIE HLCuv
struct EXPORT ColorLUV
{
	float L = 0;
	float U = 0;
	float V = 0;

	ColorLUV() {}
	ColorLUV(float InL, float InU, float InV) : L(InL), U(InU), V(InV) { }

	ColorXYZ ToXYZ() const;
};

