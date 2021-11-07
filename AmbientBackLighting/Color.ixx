export module AmbientBackLighting.Color;
import std.core;

export namespace ABL
{
	//https://www.easyrgb.com/en/math.php
	namespace CIE
	{
		//"Mid-morning daylight"
		constexpr float ReferenceX = 95.799f;
		constexpr float ReferenceY = 100.f;
		constexpr float ReferenceZ = 90.926f;
	};

	//sRGB
	struct ColorRGB
	{
		unsigned long R = 0;
		unsigned long G = 0;
		unsigned long B = 0;

		ColorRGB() {}
		ColorRGB(unsigned long InR, unsigned long InG, unsigned long InB) : R{ InR }, G{ InG }, B{ InB } { }

		double GetDistance(const ColorRGB& Other) const
		{
			auto DeltaR = Other.R - R;
			auto DeltaG = Other.G - G;
			auto DeltaB = Other.B - B;

			return std::sqrt(DeltaR * DeltaR + DeltaG * DeltaG + DeltaB * DeltaB);
		}
	};

	//CIE XYZ
	struct ColorXYZ
	{
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;

		ColorXYZ() {}
		ColorXYZ(float InX, float InY, float InZ) : X{ InX }, Y{ InY }, Z{ InZ } { }
	};

	//CIE L*uv, or more accurately CIELCH, CIE LChuv, CIE HLCuv
	struct ColorLUV
	{
		float L = 0.f;
		float U = 0.f;
		float V = 0.f;

		ColorLUV() {}
		ColorLUV(float InL, float InU, float InV) : L{ InL }, U{ InU }, V { InV } { }
	};

	static ColorXYZ RGBToXYZ(const ColorRGB& RGB)
	{
		auto NormalizedR = RGB.R / 255.0f;
		auto NormalizedG = RGB.G / 255.0f;
		auto NormalizedB = RGB.B / 255.0f;

		if (NormalizedR > 0.04045f)
			NormalizedR = std::pow(((NormalizedR + 0.055f) / 1.055f), 2.4f);
		else
			NormalizedR = NormalizedR / 12.92f;

		if (NormalizedG > 0.04045f)
			NormalizedG = std::pow(((NormalizedG + 0.055f) / 1.055f), 2.4f);
		else
			NormalizedG = NormalizedG / 12.92f;

		if (NormalizedB > 0.04045f)
			NormalizedB = std::pow(((NormalizedB + 0.055f) / 1.055f), 2.4f);
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

	static ColorRGB XYZToRGB(const ColorXYZ& XYZ)
	{
		auto NormalizedX = XYZ.X / 100.f;
		auto NormalizedY = XYZ.Y / 100.f;
		auto NormalizedZ = XYZ.Z / 100.f;

		auto R = NormalizedX * 3.2406f + NormalizedY * -1.5372f + NormalizedZ * -0.4986f;
		auto G = NormalizedX * -0.9689f + NormalizedY * 1.8758f + NormalizedZ * 0.0415f;
		auto B = NormalizedX * 0.0557f + NormalizedY * -0.2040f + NormalizedZ * 1.0570f;

		if (R > 0.0031308f)
			R = 1.055f * (std::pow(R, 1.f / 2.4f)) - 0.055f;
		else
			R = 12.92f * R;

		if (G > 0.0031308f)
			G = 1.055f * (std::pow(G, 1.f / 2.4f)) - 0.055f;
		else
			G = 12.92f * G;

		if (B > 0.0031308f)
			B = 1.055f * (std::pow(B, 1.f / 2.4f)) - 0.055f;
		else
			B = 12.92f * B;

		return
		{
			(unsigned long)(R * 255.f),
			(unsigned long)(G * 255.f),
			(unsigned long)(B * 255.f)
		};
	}

	static ColorLUV XYZToLUV(const ColorXYZ& XYZ)
	{
		auto U = (4.f * XYZ.X) / (XYZ.X + (15.f * XYZ.Y) + (3.f * XYZ.Z));
		auto V = (9.f * XYZ.Y) / (XYZ.X + (15.f * XYZ.Y) + (3.f * XYZ.Z));

		auto NormalizedY = XYZ.Y / 100.f;
		if (NormalizedY > 0.008856f)
			NormalizedY = std::pow(NormalizedY, 1.f / 3.f);
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

	static ColorXYZ LUVToXYZ(const ColorLUV& LUV)
	{
		auto Y = (LUV.L + 16.f) / 116.f;
		auto YCubed = std::pow(Y, 3.f);
		if (YCubed > 0.008856)
			Y = YCubed;
		else
			Y = (Y - 16.f / 116.f) / 7.787f;

		auto ReferenceU = (4.f * CIE::ReferenceX) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));
		auto ReferenceV = (9.f * CIE::ReferenceY) / (CIE::ReferenceX + (15.f * CIE::ReferenceY) + (3.f * CIE::ReferenceZ));

		auto ToU = LUV.U / (13.f * LUV.L) + ReferenceU;
		auto ToV = LUV.V / (13.f * LUV.L) + ReferenceV;

		Y *= 100.f;
		auto X = -(9.f * Y * ToU) / ((ToU - 4.f) * ToV - ToU * ToV);
		return
		{
			Y,
			X,
			(9.f * Y - (15.f * ToV * Y) - (ToV * X)) / (3.f * ToV)
		};
	}
}
