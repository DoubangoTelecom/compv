/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

// Hysteresis implementation based on http://www.inf.ufpr.br/vri/alumni/2011-LuisLourenco/wscad2012-lourenco.pdf

#define PIXEL_DE			255 // Definitive Edges (DE)
#define PIXEL_PE			128 // Possible Edges (PE) 
#define PIXEL_NE			0	// Non-Edges (NE)

#define PIXEL_PE_MINUS_1	127

// Double thresholding
__kernel void clHysteresisProcess_thresholding_8u8u(
	__global unsigned char* input, // 0
	__global unsigned char* output, // 1
	const int width, // 2
	const int height, // 3
	const unsigned char tLow, // 4
	const unsigned char tHigh // 5
)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	if (col < width && row < height) {
		const unsigned char edge = input[(row * width) + col];
		output[(row * width) + col] = (PIXEL_PE_MINUS_1 * (edge > tHigh) + PIXEL_PE) * (edge > tLow);		
	}
}

// Zero border which means no need to check bounds
__kernel void clHysteresisProcess_scanning_8u8u(
	__global unsigned char* input, // 0
	const int width, // 1
	const int height, // 2
	__constant int* neighbors8, // 3
	__global int* modified_global // 4
)
{
	const int global_i = get_global_id(0);
	const int global_j = get_global_id(1);

	if (global_i >= width || global_j >= height) {
		return;
	}

	__local int modified_region;
	
	//__local unsigned char region_data[18][18]; // FIXME(dmi): use shared data

	do {
		barrier(CLK_LOCAL_MEM_FENCE); // FIXME(dmi): needed?
		//if (local_i == 0 && local_j == 0) {
			modified_region = 0;
		//}
		barrier(CLK_LOCAL_MEM_FENCE);

		const int position = (global_j * width) + global_i;
		
		// No need to check bounds as borders are set to zero
		// condition "input[position] == PIXEL_PE" is true only if the current position isn't at the border which 
		// means all neighbs are available
		
		if (input[position] == PIXEL_PE) {
			#pragma unroll 8
			for (int neighb = 0; neighb < 8; ++neighb) {
				if (input[position + neighbors8[neighb]] == PIXEL_DE) {
					input[position] = PIXEL_DE;
					modified_region = 1;
				}
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);

		if (modified_region) {
			*modified_global = 1;
		}
	}
	while (modified_region);
}


__kernel void clHysteresisProcess_refining_8u8u(
	__global unsigned char* input, // 0
	const int width, // 1
	const int height // 2
)
{
	const int global_i = get_global_id(0);
	const int global_j = get_global_id(1);

	if (global_i < width && global_j < height) {
		if (input[(global_j * width) + global_i] == PIXEL_PE) {
			input[(global_j * width) + global_i] = PIXEL_NE;
		}
	}
}