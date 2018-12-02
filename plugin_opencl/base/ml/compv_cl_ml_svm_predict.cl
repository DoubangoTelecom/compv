/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#if __OPENCL_VERSION__ < 120 // Starting OpenCL 1.2 it's part of Core
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

/*
	
*/
__kernel void clCompVMachineLearningSVMPredictBinaryRBF_Part1(
	__global const double* matVectors, // 0
	__global const double* matSVs, // 1
	__global const double* matCoeffs, // 2
	__global double* matResult, // 3
	const double gammaMinus, // 4
	const int matVectors_cols, // 5
	const int matSVs_cols, // 6
	const int matResult_cols // 7
)
{
	int i = get_global_id(0);
	int j = get_global_id(1);

	double sum = 0;
	for (int k = 0; k < matSVs_cols; ++k) {
		const double diff = matVectors[(i * matVectors_cols) + k] - matSVs[(j * matSVs_cols) + k];
		sum += (diff * diff);
	}
	matResult[(i * matResult_cols) + j] = exp(sum * gammaMinus) * matCoeffs[j];
}

