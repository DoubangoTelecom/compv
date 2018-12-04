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
#if 0 // SHARED_MEMORY

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
	matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];

#elif 0 // OCCUPANCY MAXIM

	#define SVS 7
	const int global_jsvs = get_global_id(0) * SVS; // number of support vectors (e.g. 56958) / SVS
	const int global_i = get_global_id(1); // number of inputs (e.g. 408)

	for (int j = 0; j < SVS; ++j) {
		const int global_j = global_jsvs + j;
		if (global_j < 56958) { // FIXME(dmi): hard-coded
			double sum = 0;

			for (int k = 0; k < matSVs_cols; ++k) {
				const double diff = matVectors[(global_i * matVectors_cols) + k] - matSVs[(global_j * matSVs_cols) + k];
				//sum += (diff * diff);
				sum = fma(diff, diff, sum);
			}

			matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];
		}
	}

#elif 1 // CACHED
	
	int global_j = get_global_id(0); // number of support vectors (e.g. 56958)
	int global_i = get_global_id(1); // number of inputs (e.g. 408)
	int group_j = get_group_id(0);
	int group_i = get_group_id(1);
	int local_j = get_local_id(0);
	int local_i = get_local_id(1);

	// "matVectors" contains the features to classify which means it will be short (N * 63) -> no need for caching
	
	__local double matSVs_sub[16][63]; // strange, 64 slow, 63 fast, 31 fast and 32 slow
	if (global_j < 56958) {
		int m = (local_i * 4);
		for (int k = 0; k < 4 && m < 63; ++k, ++m) {
			matSVs_sub[local_j][m] = matSVs[(((group_j * 16) + local_j) * matSVs_cols) + m];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	
	if (global_i < 408 && global_j < 56958) {
		
		double sum = 0;
		for (int k = 0; k < matSVs_cols; ++k) {
			double diff = matVectors[(global_i * matVectors_cols) + k] - matSVs_sub[local_j][k];
			sum = fma(diff, diff, sum); // fma instruction is faster
		}
		
		matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];
		
		/* // Next unrolled version is faster -> Add support for T-HOG version using hard-coded values
		double diff0, diff1, diff2, diff3;
		double sum = 0;
		#define ROUND(a, b, c, d) \
			diff0 = matVectors[(global_i * matVectors_cols) + a] - matSVs_sub[local_j][a]; \
			diff1 = matVectors[(global_i * matVectors_cols) + b] - matSVs_sub[local_j][b]; \
			diff2 = matVectors[(global_i * matVectors_cols) + c] - matSVs_sub[local_j][c]; \
			diff3 = matVectors[(global_i * matVectors_cols) + d] - matSVs_sub[local_j][d]; \
			sum = fma(diff0, diff0, sum); \
			sum = fma(diff1, diff1, sum); \
			sum = fma(diff2, diff2, sum); \
			sum = fma(diff3, diff3, sum);

		ROUND(0, 1, 2, 3); ROUND(4, 5, 6, 7); ROUND(8, 9, 10, 11); ROUND(12, 13, 14, 15);
		ROUND(16, 17, 18, 19); ROUND(20, 21, 22, 23); ROUND(24, 25, 26, 27); ROUND(28, 29, 30, 31);
		ROUND(32, 33, 34, 35); ROUND(36, 37, 38, 39); ROUND(40, 41, 42, 43); ROUND(44, 45, 46, 47);
		ROUND(48, 49, 50, 51); ROUND(52, 53, 54, 55); ROUND(56, 57, 58, 59); 
		diff0 = matVectors[(global_i * matVectors_cols) + 60] - matSVs_sub[local_j][60];
		diff1 = matVectors[(global_i * matVectors_cols) + 61] - matSVs_sub[local_j][61];
		diff2 = matVectors[(global_i * matVectors_cols) + 62] - matSVs_sub[local_j][62];
		sum = fma(diff0, diff0, sum);
		sum = fma(diff1, diff1, sum);
		sum = fma(diff2, diff2, sum);
		matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];
		*/
	}

#elif 0 // CACHED + OCCUPANCY MAXIM

	#define SVS 7
	int global_j = get_global_id(0) * SVS; // number of support vectors (e.g. 56958)
	int global_i = get_global_id(1); // number of inputs (e.g. 408)
	int group_j = get_group_id(0) * SVS;
	int group_i = get_group_id(1);
	int local_j = get_local_id(0);
	int local_i = get_local_id(1);

	for (int j = 0; j < SVS; ++j) {

		// "matVectors" contains the features to classify which means it will be short (N * 63) -> no need for caching
		/*__local double matVectors_sub[16][63];
		int m = (local_j * 4);
		for (int k = 0; k < 4 && m < 63; ++k, ++m) {
			matVectors_sub[local_i][m] = 0;//matVectors[(((group_i * 16) + local_i) * matVectors_cols) + m];
		}*/

		__local double matSVs_sub[16][63];
		/*if (global_i < 408 && global_j < 56958)*/ {
			int m = (local_i * 4);
			for (int k = 0; k < 4 && m < 63; ++k, ++m) {
				matSVs_sub[local_j][m] = matSVs[(((group_j * 16) + local_j) * matSVs_cols) + m];
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);

		if (global_i >= 408 || global_j >= 56958) {
			return;
		}

		double sum = 0;

		for (int k = 0; k < matSVs_cols; ++k) {
			double diff = /*matVectors_sub[local_i][k]*/matVectors[(global_i * matVectors_cols) + k] - matSVs_sub[local_j][k]/*matSVs[(global_j * matSVs_cols) + k]*/;
			//sum += (diff * diff);
			sum = fma(diff, diff, sum); // fma instruction is faster
		}

		matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];

	} // for (int j = 0; j < SVS; ++j)
	
#elif 0 // TILED
#define TILE_SIZE 16
	const int global_j = get_global_id(0); // number of support vectors (e.g. 56958)
	const int global_i = get_global_id(1); // number of inputs (e.g. 408)
	const int group_j = get_group_id(0);
	const int group_i = get_group_id(1);
	const int local_j = get_local_id(0);
	const int local_i = get_local_id(1);

	__local double matVectors_sub[TILE_SIZE][TILE_SIZE];
	__local double matSVs_sub[TILE_SIZE][TILE_SIZE];

	if (local_i == 0 && local_j == 0) {
		for (int j = 0; j < TILE_SIZE; ++j) {
			for (int i = 0; i < TILE_SIZE; ++i) {
				matVectors_sub[local_i][local_j] = matVectors[(global_i * matVectors_cols) + local_j];
				matSVs_sub[local_j][local_i] = matSVs[(global_j * matSVs_cols) + local_i];
			}
		}
	}

	double sum = 0;

	const int numTiles = (matSVs_cols + (TILE_SIZE - 1)) / TILE_SIZE;
	for (int t = 0; t < numTiles; ++t) {
		matVectors_sub[local_i][local_j] = matVectors[(global_i * matVectors_cols) + local_j];
		matSVs_sub[local_j][local_i] = matSVs[(global_j * matSVs_cols) + local_i];
		barrier(CLK_LOCAL_MEM_FENCE);

		for (int k = 0; k < TILE_SIZE; ++k) {
			const double diff = matVectors_sub[local_i][k] * matSVs_sub[local_j][k];
			sum += (diff * diff);
			//sum = fma(diff, diff, sum);
		}
	}

	matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];

#elif 0 // mulAB instead of mulABt

	const int k = get_global_id(0); // Brows - 63 - k
	const int i = get_global_id(1); // Arows - 408 - i
	
	//double sum = 0;

	const double r = matVectors[(i * matVectors_cols) + k];
	for (size_t j = 0; j < 56958; ++j) {
		const double diff = r - matSVs[(k * matSVs_cols) + j];
		matResult[(i * matResult_cols) + j] += (diff * diff);
	}

	//for (int k = 0; k < matSVs_cols; ++k) {
	//	const double diff = matVectors[(global_i * matVectors_cols) + k] - matSVs[(global_j * matSVs_cols) + k];
		//sum += (diff * diff);
	//	sum = fma(diff, diff, sum);
	//}

	//matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];

#else // DEFAULT
	
	const int global_j = get_global_id(0); // number of support vectors (e.g. 56958)
	const int global_i = get_global_id(1); // number of inputs (e.g. 408)
	
	double sum = 0;

	for (int k = 0; k < matSVs_cols; ++k) {
		const double diff = matVectors[(global_i * matVectors_cols) + k] - matSVs[(global_j * matSVs_cols) + k];
		sum += (diff * diff);
		//sum = fma(diff, diff, sum);
	}

	matResult[(global_i * matResult_cols) + global_j] = exp(sum * gammaMinus) * matCoeffs[global_j];
	
#endif /* SHARED_MEMORY */
	
}

