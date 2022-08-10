#include "Convolution.h"

#include <cstdio>
#include <iostream>

#include "Math/Math.h"

static Hermes::Vec3 PointOnUnitSphereFromSphericalCoordinates(float Phi, float Theta)
{
	Hermes::Vec3 Result;

	Result.X = Hermes::Math::Cos(Phi) * Hermes::Math::Sin(Theta);
	Result.Y = Hermes::Math::Sin(Phi) * Hermes::Math::Sin(Theta);
	Result.Z = Hermes::Math::Cos(Theta);

	return Result;
}

/*
 * NOTE : this is really really *really* immature and unoptimized code. It needs a lot of work to be
 *        actually useful. Multithreading is the very least that needs to be done as it takes ~10 minutes
 *		  to create 128x64 convoluted envmap on my machine at the moment. Obviously, we can always lower
 *		  image dimensions or sample count, but this might cause too much of an accuracy loss
 */
std::unique_ptr<Image> GenerateDiffuseConvolution(const Image& Source)
{
	// TODO : expose dimensions via command line arguments
	auto Result = std::make_unique<Image>(128, 64, Source.GetFormat(), Source.GetBytesPerChannel());

	constexpr float SampleDelta = Hermes::Math::Pi / 180.0f / 4.0f;
	for (uint16_t Y = 0; Y < Result->GetHeight(); Y++)
	{

		for (uint16_t X = 0; X < Result->GetWidth(); X++)
		{
			/*
			 * NOTE : https://learnopengl.com/PBR/IBL/Diffuse-irradiance
			 */

			auto DestPhi = static_cast<float>(X) / static_cast<float>(Result->GetWidth()) * 2.0f * Hermes::Math::Pi;
			auto DestTheta = (static_cast<float>(Y) / static_cast<float>(Result->GetHeight()) * 2.0f - 1.0f) *
				Hermes::Math::HalfPi;

			auto Normal = PointOnUnitSphereFromSphericalCoordinates(DestPhi, DestTheta).Normalize();

			auto Up = Hermes::Vec3(0.0f, 1.0f, 0.0f);
			auto Right = (Up ^ Normal).Normalize();
			Up = (Normal ^ Right).Normalize();

			Image::Pixel AccumulatedIrradiance = { 0.0f, 0.0f, 0.0f, 0.0f };
			float NumberOfSamples = 0.0f;

			for (float Phi = 0.0f; Phi <= 2.0f * Hermes::Math::Pi; Phi += SampleDelta)  // NOLINT(cert-flp30-c)
			{
				for (float Theta = 0.0f; Theta <= Hermes::Math::HalfPi; Theta += SampleDelta)  // NOLINT(cert-flp30-c)
				{
					/*
					 * Spherical to cartesian in tangent space
					 */
					Hermes::Vec3 TangentSample(Hermes::Math::Sin(Theta) * Hermes::Math::Cos(Phi),
					                           Hermes::Math::Sin(Theta) * Hermes::Math::Sin(Phi),
					                           Hermes::Math::Cos(Theta));

					auto WorldSample = Right * TangentSample.X + Up * TangentSample.Y + Normal * TangentSample.Z;
					
					float SamplePhi = Hermes::Math::Atan(WorldSample.Z, WorldSample.X);
					float SampleTheta = Hermes::Math::Acos(WorldSample.Y);

					float NormalizedPhi = (SamplePhi+ Hermes::Math::Pi) / (2.0f * Hermes::Math::Pi);
					float NormalizedTheta = SampleTheta / Hermes::Math::Pi;

					auto SampledValue = Source.Sample(NormalizedPhi, NormalizedTheta);

					AccumulatedIrradiance = AccumulatedIrradiance + SampledValue * Hermes::Math::Sin(Theta) *
						Hermes::Math::Cos(Theta);
					NumberOfSamples += 1.0f;
				}
			}

			AccumulatedIrradiance = AccumulatedIrradiance * (1.0f / NumberOfSamples);
			Result->Store(X, Y, AccumulatedIrradiance);
		}

		printf("\rDiffuse convolution: %.1f%% done",
		       (100.0 * static_cast<double>(Y + 1) / static_cast<double>(Result->GetHeight())));
	}
	std::cout << std::endl;

	return Result;
}
