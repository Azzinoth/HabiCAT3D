#pragma once

#include "MeasurementGrid.h"
using namespace FocalEngine;

namespace FocalEngine
{
#define DEFAULT_GRID_SIZE 1.25f
#define GRID_VARIANCE 25

	static std::vector<float> PseudoRandom64 = {
		0.300000f, 0.100000f, 0.020000f, 1.430000f,
		0.450000f, 0.130000f, 0.090000f, 1.410000f,
		0.410000f, -0.310000f, 0.300000f, 1.390000f,
		0.340000f, 0.130000f, 0.470000f, 1.330000f,
		-0.390000f, 0.060000f, -0.440000f, 1.400000f,
		-0.350000f, -0.020000f, 0.380000f, 1.340000f,
		-0.300000f, 0.480000f, -0.500000f, 1.280000f,
		-0.390000f, 0.060000f, 0.370000f, 1.290000f,
		-0.120000f, 0.270000f, 0.300000f, 1.300000f,
		0.030000f, -0.060000f, -0.290000f, 1.410000f,
		-0.240000f, -0.130000f, -0.130000f, 1.250000f,
		0.220000f, -0.320000f, 0.160000f, 1.300000f,
		-0.360000f, -0.010000f, -0.190000f, 1.270000f,
		-0.200000f, -0.220000f, -0.140000f, 1.440000f,
		0.340000f, -0.100000f, -0.060000f, 1.280000f,
		-0.210000f, 0.290000f, 0.090000f, 1.270000f,
		0.130000f, 0.010000f, 0.180000f, 1.250000f,
		0.090000f, 0.460000f, 0.350000f, 1.460000f,
		-0.500000f, -0.090000f, 0.320000f, 1.250000f,
		-0.200000f, 0.080000f, -0.320000f, 1.260000f,
		-0.390000f, -0.440000f, 0.420000f, 1.410000f,
		0.260000f, -0.040000f, 0.400000f, 1.440000f,
		-0.020000f, -0.010000f, 0.290000f, 1.400000f,
		-0.020000f, -0.370000f, 0.280000f, 1.390000f,
		0.240000f, -0.320000f, -0.290000f, 1.410000f,
		-0.100000f, 0.360000f, -0.110000f, 1.420000f,
		-0.270000f, -0.490000f, 0.470000f, 1.250000f,
		-0.450000f, 0.390000f, -0.310000f, 1.280000f,
		0.440000f, -0.290000f, -0.360000f, 1.460000f,
		0.000000f, -0.050000f, -0.370000f, 1.400000f,
		-0.060000f, 0.120000f, 0.430000f, 1.270000f,
		-0.490000f, 0.410000f, 0.150000f, 1.270000f,
		-0.500000f, 0.370000f, -0.040000f, 1.390000f,
		0.380000f, -0.210000f, 0.040000f, 1.320000f,
		0.280000f, 0.340000f, -0.430000f, 1.270000f,
		-0.410000f, -0.450000f, 0.330000f, 1.430000f,
		-0.180000f, -0.370000f, 0.070000f, 1.280000f,
		-0.110000f, -0.310000f, 0.400000f, 1.300000f,
		-0.060000f, 0.270000f, 0.240000f, 1.320000f,
		-0.390000f, 0.480000f, 0.180000f, 1.260000f,
		-0.390000f, 0.100000f, 0.070000f, 1.330000f,
		-0.260000f, -0.230000f, 0.450000f, 1.430000f,
		0.480000f, 0.080000f, -0.140000f, 1.470000f,
		0.330000f, -0.380000f, -0.350000f, 1.310000f,
		0.180000f, 0.200000f, -0.330000f, 1.490000f,
		-0.230000f, -0.110000f, 0.070000f, 1.250000f,
		0.190000f, -0.230000f, -0.330000f, 1.480000f,
		0.480000f, 0.150000f, 0.290000f, 1.410000f,
		-0.210000f, 0.240000f, 0.180000f, 1.320000f,
		0.450000f, -0.230000f, 0.240000f, 1.480000f,
		0.400000f, 0.370000f, -0.060000f, 1.450000f,
		0.080000f, -0.500000f, 0.200000f, 1.370000f,
		-0.240000f, -0.380000f, 0.440000f, 1.430000f,
		-0.230000f, -0.210000f, 0.240000f, 1.390000f,
		-0.320000f, -0.050000f, 0.330000f, 1.370000f,
		-0.030000f, -0.470000f, -0.320000f, 1.480000f,
		0.320000f, 0.160000f, 0.460000f, 1.460000f,
		0.270000f, 0.430000f, 0.410000f, 1.380000f,
		0.240000f, -0.470000f, -0.190000f, 1.430000f,
		0.480000f, -0.420000f, -0.240000f, 1.490000f,
		-0.400000f, -0.270000f, 0.060000f, 1.450000f,
		0.280000f, 0.430000f, -0.010000f, 1.490000f,
		-0.190000f, 0.110000f, -0.100000f, 1.450000f,
		-0.210000f, -0.040000f, 0.340000f, 1.440000f,
	};

	static std::vector<float> Tetrahedron1Jitter = {
		0.0000f, 0.0000f, 0.0000f, 1.0000f,
	};

	static std::vector<float> Tetrahedron7Jitter = {
		0.0000f, 0.0000f, 0.0000f, 1.0000f,
		0.0000f, 1.0000f, -0.0000f, 1.0000f,
		1.0000f, 0.0000f, -0.0000f, 1.0000f,
		0.0000f, 0.0000f, -1.0000f, 1.0000f,
		-1.0000f, 0.0000f, -0.0000f, 1.0000f,
		0.0000f, 0.0000f, 1.0000f, 1.0000f,
		0.0000f, -1.0000f, -0.0000f, 1.0000f
	};

	static std::vector<float> Tetrahedron13Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.50000f, -0.50000f, 1.0000f,
		-0.70711f, 0.50000f, -0.50000f, 1.0000f,
		-0.70711f, -0.50000f, 0.50000f, 1.0000f,
		0.70711f, -0.50000f, 0.50000f, 1.0000f,
		-0.00000f, -0.70711f, -0.70711f, 1.0000f
	};

	static std::vector<float> Tetrahedron19Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, -0.70711f, 1.0000f,
		-0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, -0.70711f, 1.0000f,
		-0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, 0.70711f, 1.0000f
	};

	static std::vector<float> Tetrahedron25Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, -0.70711f, 1.0000f,
		-0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, -0.70711f, 1.0000f,
		-0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, 0.70711f, 1.0000f,
		-0.00537f, 0.45959f, 0.45962f, 1.0000f,
		0.45579f, 0.33035f, -0.32500f, 1.0000f,
		-0.46338f, 0.31961f, -0.32500f, 1.0000f,
		-0.45579f, -0.33035f, 0.32500f, 1.0000f,
		0.46338f, -0.31961f, 0.32500f, 1.0000f,
		0.00537f, -0.45959f, -0.45962f, 1.0000f
	};

	static std::vector<float> Tetrahedron37Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, -0.70711f, 1.0000f,
		-0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, -0.70711f, 1.0000f,
		-0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.04654f, 0.37805f, 0.32391f, 1.0000f,
		0.25692f, 0.26041f, -0.34085f, 1.0000f,
		-0.42641f, 0.19816f, -0.17001f, 1.0000f,
		-0.25692f, -0.26041f, 0.34085f, 1.0000f,
		0.42641f, -0.19816f, 0.17001f, 1.0000f,
		-0.04654f, -0.37805f, -0.32391f, 1.0000f,
		0.21458f, 0.45146f, -0.01198f, 1.0000f,
		-0.26861f, 0.40744f, 0.10882f, 1.0000f,
		-0.14876f, 0.08318f, 0.47005f, 1.0000f,
		0.33443f, 0.12720f, 0.34926f, 1.0000f,
		0.14876f, -0.08318f, -0.47005f, 1.0000f,
		-0.33443f, -0.12720f, -0.34926f, 1.0000f,
		-0.21458f, -0.45146f, 0.01198f, 1.0000f,
		0.26861f, -0.40744f, -0.10882f, 1.0000f,
		-0.11985f, 0.32426f, -0.36124f, 1.0000f,
		-0.48319f, -0.04401f, 0.12080f, 1.0000f,
		0.11985f, -0.32426f, 0.36124f, 1.0000f,
		0.48319f, 0.04401f, -0.12080f, 1.0000f
	};

	static std::vector<float> Tetrahedron55Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, -0.70711f, 1.0000f,
		-0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, -0.70711f, 1.0000f,
		-0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.06143f, 0.49902f, 0.42756f, 1.0000f,
		0.33913f, 0.34374f, -0.44992f, 1.0000f,
		-0.56286f, 0.26158f, -0.22442f, 1.0000f,
		-0.33913f, -0.34374f, 0.44992f, 1.0000f,
		0.56286f, -0.26158f, 0.22442f, 1.0000f,
		-0.06143f, -0.49902f, -0.42756f, 1.0000f,
		0.28324f, 0.59592f, -0.01582f, 1.0000f,
		-0.35456f, 0.53782f, 0.14364f, 1.0000f,
		-0.19636f, 0.10980f, 0.62047f, 1.0000f,
		0.44144f, 0.16790f, 0.46102f, 1.0000f,
		0.19636f, -0.10980f, -0.62047f, 1.0000f,
		-0.44144f, -0.16790f, -0.46102f, 1.0000f,
		-0.28324f, -0.59592f, 0.01582f, 1.0000f,
		0.35456f, -0.53782f, -0.14364f, 1.0000f,
		-0.15820f, 0.42802f, -0.47683f, 1.0000f,
		-0.63781f, -0.05810f, 0.15946f, 1.0000f,
		0.15820f, -0.42802f, 0.47683f, 1.0000f,
		0.63781f, 0.05810f, -0.15946f, 1.0000f,
		0.00360f, 0.23247f, 0.23419f, 1.0000f,
		0.25790f, 0.14413f, -0.14702f, 1.0000f,
		-0.20586f, 0.18463f, -0.18010f, 1.0000f,
		-0.25790f, -0.14413f, 0.14702f, 1.0000f,
		0.20586f, -0.18463f, 0.18010f, 1.0000f,
		-0.00360f, -0.23247f, -0.23419f, 1.0000f,
		0.18490f, 0.26629f, 0.06164f, 1.0000f,
		-0.14302f, 0.29493f, 0.03825f, 1.0000f,
		-0.17982f, 0.06246f, 0.26956f, 1.0000f,
		0.14810f, 0.03383f, 0.29295f, 1.0000f,
		0.17982f, -0.06246f, -0.26956f, 1.0000f,
		-0.14810f, -0.03383f, -0.29295f, 1.0000f,
		-0.18490f, -0.26629f, -0.06164f, 1.0000f,
		0.14302f, -0.29493f, -0.03825f, 1.0000f,
		0.03680f, 0.23247f, -0.23131f, 1.0000f,
		-0.32792f, 0.02864f, -0.02339f, 1.0000f,
		-0.03680f, -0.23247f, 0.23131f, 1.0000f,
		0.32792f, -0.02864f, 0.02339f, 1.0000f
	};

	static std::vector<float> Tetrahedron73Jitter = {
		0.00000f, 0.00000f, 0.00000f, 1.0000f,
		0.00000f, 1.00000f, -0.00000f, 1.0000f,
		1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, -1.00000f, 1.0000f,
		-1.00000f, 0.00000f, -0.00000f, 1.0000f,
		0.00000f, 0.00000f, 1.00000f, 1.0000f,
		0.00000f, -1.00000f, -0.00000f, 1.0000f,
		0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, -0.70711f, 1.0000f,
		-0.70711f, 0.70711f, -0.00000f, 1.0000f,
		0.00000f, 0.70711f, 0.70711f, 1.0000f,
		0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, -0.70711f, 1.0000f,
		-0.70711f, -0.70711f, -0.00000f, 1.0000f,
		0.00000f, -0.70711f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, -0.70711f, 1.0000f,
		-0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.70711f, 0.00000f, 0.70711f, 1.0000f,
		0.06981f, 0.56707f, 0.48586f, 1.0000f,
		0.38538f, 0.39061f, -0.51128f, 1.0000f,
		-0.63962f, 0.29724f, -0.25502f, 1.0000f,
		-0.38538f, -0.39061f, 0.51128f, 1.0000f,
		0.63962f, -0.29724f, 0.25502f, 1.0000f,
		-0.06981f, -0.56707f, -0.48586f, 1.0000f,
		0.32187f, 0.67718f, -0.01797f, 1.0000f,
		-0.40291f, 0.61116f, 0.16323f, 1.0000f,
		-0.22314f, 0.12477f, 0.70508f, 1.0000f,
		0.50164f, 0.19079f, 0.52388f, 1.0000f,
		0.22314f, -0.12477f, -0.70508f, 1.0000f,
		-0.50164f, -0.19079f, -0.52388f, 1.0000f,
		-0.32187f, -0.67718f, 0.01797f, 1.0000f,
		0.40291f, -0.61116f, -0.16323f, 1.0000f,
		-0.17977f, 0.48639f, -0.54185f, 1.0000f,
		-0.72478f, -0.06602f, 0.18120f, 1.0000f,
		0.17977f, -0.48639f, 0.54185f, 1.0000f,
		0.72478f, 0.06602f, -0.18120f, 1.0000f,
		0.00545f, 0.35222f, 0.35484f, 1.0000f,
		0.39075f, 0.21838f, -0.22276f, 1.0000f,
		-0.31190f, 0.27974f, -0.27288f, 1.0000f,
		-0.39075f, -0.21838f, 0.22276f, 1.0000f,
		0.31190f, -0.27974f, 0.27288f, 1.0000f,
		-0.00545f, -0.35222f, -0.35484f, 1.0000f,
		0.28016f, 0.40347f, 0.09339f, 1.0000f,
		-0.21670f, 0.44686f, 0.05795f, 1.0000f,
		-0.27245f, 0.09464f, 0.40843f, 1.0000f,
		0.22440f, 0.05125f, 0.44387f, 1.0000f,
		0.27245f, -0.09464f, -0.40843f, 1.0000f,
		-0.22440f, -0.05125f, -0.44387f, 1.0000f,
		-0.28016f, -0.40347f, -0.09339f, 1.0000f,
		0.21670f, -0.44686f, -0.05795f, 1.0000f,
		0.05576f, 0.35222f, -0.35047f, 1.0000f,
		-0.49685f, 0.04339f, -0.03544f, 1.0000f,
		-0.05576f, -0.35222f, 0.35047f, 1.0000f,
		0.49685f, -0.04339f, 0.03544f, 1.0000f,
		0.04625f, 0.08012f, 0.23225f, 1.0000f,
		0.18067f, 0.14906f, -0.08740f, 1.0000f,
		-0.16649f, 0.18401f, -0.03033f, 1.0000f,
		-0.18067f, -0.14906f, 0.08740f, 1.0000f,
		0.16649f, -0.18401f, 0.03033f, 1.0000f,
		-0.04625f, -0.08012f, -0.23225f, 1.0000f,
		0.16046f, 0.16206f, 0.10243f, 1.0000f,
		-0.08502f, 0.18677f, 0.14278f, 1.0000f,
		-0.09505f, -0.04875f, 0.22603f, 1.0000f,
		0.15043f, -0.07346f, 0.18567f, 1.0000f,
		0.09505f, 0.04875f, -0.22603f, 1.0000f,
		-0.15043f, 0.07346f, -0.18567f, 1.0000f,
		-0.16046f, -0.16206f, -0.10243f, 1.0000f,
		0.08502f, -0.18677f, -0.14278f, 1.0000f,
		0.01003f, 0.23552f, -0.08324f, 1.0000f,
		-0.24548f, 0.02472f, 0.04036f, 1.0000f,
		-0.01003f, -0.23552f, 0.08324f, 1.0000f,
		0.24548f, -0.02472f, -0.04036f, 1.0000f
	};

	static std::unordered_map<std::string, std::vector<float>> TetrahedronJitterOrientationsOptions = {
		{"1",		Tetrahedron1Jitter},
		{"7",		Tetrahedron7Jitter},
		{"13",		Tetrahedron13Jitter},
		//{"19",	Tetrahedron19Jitter},
		{"25",		Tetrahedron25Jitter},
		{"37",		Tetrahedron37Jitter},
		{"55",		Tetrahedron55Jitter},
		{"73",		Tetrahedron73Jitter}
	};

	static std::vector<float> SphereJitter = {
		0.0000f, 1.0000f, -0.0000f, 1.430000f,
		1.0000f, 0.0000f, -0.0000f, 1.410000f,
		0.0000f, 0.0000f, -1.0000f, 1.390000f,
		-1.0000f, 0.0000f, -0.0000f, 1.330000f,
		0.0000f, 0.0000f, 1.0000f, 1.400000f,
		0.0000f, -1.0000f, -0.0000f, 1.340000f,
		0.5000f, 0.8660f, -0.0000f, 1.280000f,
		0.8660f, 0.5000f, -0.0000f, 1.290000f,
		0.0000f, 0.8660f, -0.5000f, 1.300000f,
		0.0000f, 0.5000f, -0.8660f, 1.410000f,
		-0.5000f, 0.8660f, -0.0000f, 1.250000f,
		-0.8660f, 0.5000f, -0.0000f, 1.300000f,
		0.0000f, 0.8660f, 0.5000f, 1.270000f,
		0.0000f, 0.5000f, 0.8660f, 1.440000f,
		0.5000f, -0.8660f, -0.0000f, 1.280000f,
		0.8660f, -0.5000f, -0.0000f, 1.270000f,
		0.0000f, -0.8660f, -0.5000f, 1.250000f,
		0.0000f, -0.5000f, -0.8660f, 1.460000f,
		-0.5000f, -0.8660f, -0.0000f, 1.250000f,
		-0.8660f, -0.5000f, -0.0000f, 1.260000f,
		0.0000f, -0.8660f, 0.5000f, 1.410000f,
		0.0000f, -0.5000f, 0.8660f, 1.440000f,
		0.8660f, 0.0000f, -0.5000f, 1.400000f,
		0.5000f, 0.0000f, -0.8660f, 1.390000f,
		-0.5000f, 0.0000f, -0.8660f, 1.410000f,
		-0.8660f, 0.0000f, -0.5000f, 1.420000f,
		-0.8660f, 0.0000f, 0.5000f, 1.250000f,
		-0.5000f, 0.0000f, 0.8660f, 1.280000f,
		0.5000f, 0.0000f, 0.8660f, 1.460000f,
		0.8660f, 0.0000f, 0.5000f, 1.400000f,
		0.5477f, 0.6325f, -0.5477f, 1.270000f,
		-0.5477f, 0.6325f, -0.5477f, 1.270000f,
		-0.5477f, 0.6325f, 0.5477f, 1.390000f,
		0.5477f, 0.6325f, 0.5477f, 1.320000f,
		0.5477f, -0.6325f, -0.5477f, 1.270000f,
		-0.5477f, -0.6325f, -0.5477f, 1.430000f,
		-0.5477f, -0.6325f, 0.5477f, 1.280000f,
		0.5477f, -0.6325f, 0.5477f, 1.300000f,
		0.0000f, 0.5000f, -0.0000f, 1.320000f,
		0.4714f, -0.1667f, -0.0000f, 1.260000f,
		-0.2357f, -0.1667f, -0.4082f, 1.330000f,
		-0.2357f, -0.1667f, 0.4082f, 1.430000f,
		0.2973f, 0.4020f, -0.0000f, 1.470000f,
		0.4781f, 0.1463f, -0.0000f, 1.310000f,
		-0.1487f, 0.4020f, -0.2575f, 1.490000f,
		-0.2391f, 0.1463f, -0.4140f, 1.250000f,
		-0.1487f, 0.4020f, 0.2575f, 1.480000f,
		-0.2391f, 0.1463f, 0.4140f, 1.410000f,
		0.3294f, -0.2742f, -0.2575f, 1.320000f,
		0.0583f, -0.2742f, -0.4140f, 1.480000f,
		0.3294f, -0.2742f, 0.2575f, 1.450000f,
		0.0583f, -0.2742f, 0.4140f, 1.370000f,
		-0.3877f, -0.2742f, -0.1565f, 1.430000f,
		-0.3877f, -0.2742f, 0.1565f, 1.390000f,
		0.2132f, 0.2611f, -0.3693f, 1.370000f,
		-0.4264f, 0.2611f, -0.0000f, 1.480000f,
		0.2132f, 0.2611f, 0.3693f, 1.460000f,
		0.1040f, -0.4891f, -0.0000f, 1.380000f,
	};

	struct GridInitData_Jitter
	{
		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 2.5f;
	};

	class JitterManager
	{
		friend class UIManager;
		friend class LayerManager;
	public:
		SINGLETON_PUBLIC_PART(JitterManager)

		static void OnMeshUpdate();

		void CalculateWithGridJitterAsync(std::function<void(GridNode* currentNode)> funcf, bool bSmootherResult = false);
		void CalculateOnWholeModel(std::function<void(GridNode* currentNode)> func);
		void SetOnCalculationsStartCallback(std::function<void()> func);
		void SetOnCalculationsEndCallback(std::function<void(MeshLayer CurrentMeshLayer)> func);

		float GetResolutonInM();
		void SetResolutonInM(float NewResolutonInM);

		float GetLowestPossibleResolution();
		float GetHigestPossibleResolution();

		int GetJitterDoneCount();
		int GetJitterToDoCount();

		std::vector<std::vector<float>> GetPerJitterResult();

		MeasurementGrid* GetLastUsedGrid();
		std::vector<GridInitData_Jitter> GetLastUsedJitterSettings();

		/**
		* @brief Sets the function that determines whether a calculated value should be ignored.
		*
		* This function is called for each calculated value in each jitter. If it returns truef, the value is ignored.
		* If it returns falsef, the value is used to calculate all jitter combined result array.
		* 
		* After each calculationf, this function will reset to ensure the default behavior is applied.
		*
		* @param func The desired function.
		*/
		void SetIgnoreValueFunction(std::function<bool(float Value)> func);

		/**
		* @brief Sets the fallback value for the result array.
		*
		* This value is used for an element in the result array only if all its
		* calculated values are ignored. By providing a fallbackf, we ensure that
		* the result array has a consistent value in such scenariosf, preventing
		* potential issues from uninitialized or unpredictable data.
		* 
		* After each calculationf, this value will be reset to 1.0f to ensure that the default behavior is used.
		*
		* @param NewValue The desired fallback value.
		*/
		void SetFallbackValue(float NewValue);

		std::string GetCurrentJitterVectorSetName();
		void SetCurrentJitterVectorSetName(std::string name);
		std::vector<std::string> GetJitterVectorSetNames();

		void AdjustOutliers(std::vector<float>& Dataf, float LowerPercentilef, float UpperPercentile);

		// Produces a data layer of standard deviation values for all jitters from last run.
		std::vector<float> ProduceStandardDeviationData();

		FEAABB GetAABBForJitteredGrid(GridInitData_Jitter* Settings, float CurrentResolutionInM);
	private:
		SINGLETON_PRIVATE_PART(JitterManager)

		MeasurementGrid* LastUsedGrid = nullptr;
		std::vector<GridInitData_Jitter> LastUsedJitterSettings;
		std::function<void(GridNode* currentNode)> CurrentFunc;
		
		int JitterDoneCount = 0;
		int JitterToDoCount = 4;
		int DebugJitterToDoCount = -1;
		int GetDebugJitterToDoCount();
		void SetDebugJitterToDoCount(int NewValue);
		std::vector<std::string> JitterVectorSetNames;
		std::string CurrentJitterVectorSetName = "55";

		float ResolutonInM = 1.0f;
		float LowestPossibleResolution = -1.0f;
		float HigestPossibleResolution = -1.0f;

		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 1.25f;

		std::vector<float> Result;
		std::vector<std::vector<float>> PerJitterResult;
		std::vector<int> CorrectValuesCounters;
		float FallbackValue = 1.0f;
		std::function<bool(float Value)> IgnoreValueFunc = nullptr;

		void RunCreationOfGridAsync();
		static void RunCalculationOnGridAsync(void* InputDataf, void* OutputData);
		static void AfterCalculationFinishGridCallback(void* OutputData);
		void MoveResultDataFromGrid(MeasurementGrid* Grid);
		void RunCalculationOnWholeModel(MeasurementGrid* ResultGrid);

		static void OnCalculationsStart();
		static void OnCalculationsEnd();

		std::vector <std::function<void()>> OnCalculationsStartCallbacks;
		std::vector<std::function<void(MeshLayer CurrentMeshLayer)>> OnCalculationsEndCallbacks;

		float LastTimeTookForCalculation;
		uint64_t StartTime;

		float FindStandardDeviation(std::vector<float> DataPoints);
	};

	#define JITTER_MANAGER JitterManager::getInstance()
}