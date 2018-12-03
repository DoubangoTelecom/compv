/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#if __OPENCL_VERSION__ < 120 // Starting OpenCL 1.2 it's part of Core
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#define TS 16 // FIXME(dmi): must not be hard-coded (localsize)

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
#if 0 // TILED VERSION
	const int local_i = get_local_id(1);
    const int local_j = get_local_id(0);
	const int global_i = TS*get_group_id(1) + local_i;
	const int global_j = TS*get_group_id(0) + local_j;

	double sum = 0;

	__local double matVectors_sub[TS][TS];
    __local double matSVs_sub[TS][TS];
	
	const int numTiles = (matSVs_cols) / TS; // FIXME(dmi): not correct because not multiple of TS
	
	for (int t = 0; t< numTiles; ++t) {
		const int matVectors_i = TS*t + local_i;
		const int matSVs_j = TS*t + local_j;
		matVectors_sub[local_i][local_j] = matVectors[(matVectors_i * matVectors_cols) + global_i];
		matSVs_sub[local_i][local_j] = matSVs[(matSVs_j * matSVs_cols) + global_j];

		barrier(CLK_LOCAL_MEM_FENCE);

		for (int k = 0; k < TS; ++k) {
			/*if ((global_i + (TS*t) + k) < 408 && (global_j + (TS*t) + k) < 56958)*/ {
				const double diff = matVectors_sub[k][local_i] - matSVs_sub[local_j][k];
				 sum += (diff * diff);
				//sum = fma(diff, diff, sum);
			}
        }
 
        barrier(CLK_LOCAL_MEM_FENCE);		
	}

#else // OTHER VERSION
#define SVS 20
	const int global_i = get_global_id(1); // number of inputs (e.g. 408)
	const int global_j = get_global_id(0); // number of support vectors (e.g. 56958)

	double sum = 0;

	for (int k = 0; k < matSVs_cols; ++k) {
		const double diff = matVectors[(global_i * matVectors_cols) + k] - matSVs[(global_j * matSVs_cols) + k];
		sum += (diff * diff);
		//sum = fma(diff, diff, sum);
	}
#endif 

	matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];
}

