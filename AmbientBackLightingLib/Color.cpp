#include "stdafx.h"
#include "Color.h"

//"Mid-morning daylight"
const float CIE::ReferenceX = 95.799f;
const float CIE::ReferenceY = 100.f;
const float CIE::ReferenceZ = 90.926f;

//equal energy: 100.f across the board

ColorXYZ ColorRGB::ToXYZ() const
{
	auto NormalizedR = R / 255.0f;
	auto NormalizedG = G / 255.0f;
	auto NormalizedB = B / 255.0f;

	if (NormalizedR > 0.04045f)
		NormalizedR = pow(((NormalizedR + 0.055f) / 1.055f), 2.4f);
	else
		NormalizedR = NormalizedR / 12.92f;

	if (NormalizedG > 0.04045f)
		NormalizedG = pow(((NormalizedG + 0.055f) / 1.055f), 2.4f);
	else
		NormalizedG = NormalizedG / 12.92f;

	if (NormalizedB > 0.04045f)
		NormalizedB = pow(((NormalizedB + 0.055f) / 1.055f), 2.4f);
	else
		NormalizedB = NormalizedB / 12.92f;

	NormalizedR *= 100.f;
	NormalizedG *= 100.f;
	NormalizedB *= 100.f;

	return
	{
		NormalizedR * 0.4124f + NormalizedG * 0.3576f + NormalizedB * 0.1805f,
		NormalizedR * 0.2126f + NormalizedG * 0.7152f + NormalizedB * 0.0722f,
		NormalizedR * 0.0193f + NormalizedG * 0.1192f + NormalizedB * 0.9505f
	};
}

ColorRGB ColorXYZ::ToRGB() const
{
	auto NormalizedX = X / 100.f;
	auto NormalizedY = Y / 100.f;
	auto NormalizedZ = Z / 100.f;

	auto R = NormalizedX * 3.2406f + NormalizedY * -1.5372f + NormalizedZ * -0.4986f;
	auto G = NormalizedX * -0.9689f + NormalizedY * 1.8758f + NormalizedZ * 0.0415f;
	auto B = NormalizedX * 0.0557f + NormalizedY * -0.2040f + NormalizedZ * 1.0570f;

	if (R > 0.0031308f)
		R = 1.055f * (pow(R, 1.f / 2.4f)) - 0.055f;
	else
		R = 12.92f * R;

	if (G > 0.0031308f)
		G = 1.055f * (pow(G, 1.f / 2.4f)) - 0.055f;
	else
		G = 12.92f * G;

	if (B > 0.0031308f)
		B = 1.055f * (pow(B, 1.f / 2.4f)) - 0.055f;
	else
		B = 12.92f * B;

	return
	{
		(ULONG)(R * 255.f),
		(ULONG)(G * 255.f),
		(ULONG)(B * 255.f)
	};
}

ColorLUV ColorXYZ::ToLUV() const
{
	auto U = (4.f * X) / (X + (15.f * Y) + (3.f * Z));
	auto V = (9.f * Y) / (X + (15.f * Y) + (3.f * Z));

	auto NormalizedY = Y / 100.f;
	if (NormalizedY > 0.008856f)
		NormalizedY = pow(NormalizedY, 1.f / 3.f);
	else
		NormalizedY = (7.787f * NormalizedY) + (16.f / 116.f);
	
	auto ReferenceU = (4.f * CIE::ReferenceX) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));
	auto ReferenceV = (9.f * CIE::ReferenceY) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));

	auto L = (116.f * NormalizedY) - 16.f;
	return
	{
		L,
		13.f * L * (U - ReferenceU),
		13.f * L * (V - ReferenceV)
	};
}

ColorXYZ ColorLUV::ToXYZ() const
{
	auto Y = (L + 16.f) / 116.f;
	auto YCubed = pow(Y, 3.f);
	if (YCubed > 0.008856)
		Y = YCubed;
	else
		Y = (Y - 16.f / 116.f) / 7.787f;

	auto ReferenceU = (4.f * CIE::ReferenceX) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));
	auto ReferenceV = (9.f * CIE::ReferenceY) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));

	auto ToU = U / (13.f * L) + ReferenceU;
	auto ToV = V / (13.f * L) + ReferenceV;

	Y *= 100.f;
	auto X = -(9.f * Y * ToU) / ((ToU - 4.f) * ToV - ToU * ToV);
	return
	{
		Y,
		X,
		(9.f * Y - (15.f * ToV * Y) - (ToV * X)) / (3.f * ToV)
	};
}

