/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

__local static const unsigned short kCompVFast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
__local static const unsigned short kCompVFast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

#define _opencl_fast_check(a, b, c, d) \
		t0 = circle[a][idx], t1 = circle[b][idx]; \
		sd = (t0 < darker) + (t1 < darker), sb = (t0 > brighter) + (t1 > brighter); \
		if (!(sd || sb)) { strengths[idx] = 0; return; } \
		sumd += sd, sumb += sb; \
		t0 = circle[c][idx], t1 = circle[d][idx]; \
		sd = (t0 < darker) + (t1 < darker), sb = (t0 > brighter) + (t1 > brighter); \
		if (!(sd || sb)) { strengths[idx] = 0; return; } \
		sumd += sd, sumb += sb; \
		if (sumd < minsum && sumb < minsum) { strengths[idx] = 0; return; } \

#define _opencl_fast_strenght() \
	unsigned flags = \
		(neighborhoods16[0] ? (1 << 0) : 0) \
		| (neighborhoods16[1] ? (1 << 1) : 0) \
		| (neighborhoods16[2] ? (1 << 2) : 0) \
		| (neighborhoods16[3] ? (1 << 3) : 0) \
		| (neighborhoods16[4] ? (1 << 4) : 0) \
		| (neighborhoods16[5] ? (1 << 5) : 0) \
		| (neighborhoods16[6] ? (1 << 6) : 0) \
		| (neighborhoods16[7] ? (1 << 7) : 0) \
		| (neighborhoods16[8] ? (1 << 8) : 0) \
		| (neighborhoods16[9] ? (1 << 9) : 0) \
		| (neighborhoods16[10] ? (1 << 10) : 0) \
		| (neighborhoods16[11] ? (1 << 11) : 0) \
		| (neighborhoods16[12] ? (1 << 12) : 0) \
		| (neighborhoods16[13] ? (1 << 13) : 0) \
		| (neighborhoods16[14] ? (1 << 14) : 0) \
		| (neighborhoods16[15] ? (1 << 15) : 0); \
		for (unsigned arcStart = 0; arcStart < 16; ++arcStart) { \
			if ((flags & FastXFlags[arcStart]) == FastXFlags[arcStart]) { \
				t0 = 0xff; \
				for (unsigned j = arcStart, k = 0; k < N; ++j, ++k) { \
					t0 = min(neighborhoods16[j & 15], t0); \
				} \
				strength = max(strength, t0); \
			} \
		} \


// Requires OpenCL 1.1
__kernel void clFAST(
	__global const unsigned char* IP,
	unsigned int width,
	unsigned int height,
	unsigned int stride,
	__global const int* pixels16,
	unsigned char N,
	unsigned char threshold,
	__global unsigned char* strengths
)
{
	/* Initializes local memory shared by group items */
#if 0 // FIXME: why local mem not working?
	__local const unsigned char* circle[16];
	if (get_local_id(0) == 0) {
		for (int i = 0; i < 16; ++i) {
			circle[i] = &IP[pixels16[i]];
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
#else
	__global const unsigned char* circle[16] = {
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};
#endif

	/* Performs FAST feature detection */
	int x = get_global_id(0);
	if (x >= 3 && x < width - 3) {
		int y = get_global_id(1);
		if (y >= 3 && y < height - 3) {
			int idx = x + (y*stride);
			const unsigned char minsum = (N == 12 ? 3 : 2); // FIXME: make param
			unsigned char strength, sumb, sumd, sb, sd, brighter, darker, t0, t1;
			strength = sumb = sumd = 0;
			brighter = add_sat(IP[idx], threshold);
			darker = sub_sat(IP[idx], threshold);

			_opencl_fast_check(0, 8, 4, 12);
			_opencl_fast_check(1, 9, 5, 13);
			_opencl_fast_check(2, 10, 6, 14);
			_opencl_fast_check(3, 11, 7, 15);

			unsigned char neighborhoods16[16];
			const unsigned short *FastXFlags = N == 9 ? kCompVFast9Flags : kCompVFast12Flags;

			if (sumd >= N) {
				neighborhoods16[0] = sub_sat(darker, circle[0][idx]);
				neighborhoods16[1] = sub_sat(darker, circle[1][idx]);
				neighborhoods16[2] = sub_sat(darker, circle[2][idx]);
				neighborhoods16[3] = sub_sat(darker, circle[3][idx]);
				neighborhoods16[4] = sub_sat(darker, circle[4][idx]);
				neighborhoods16[5] = sub_sat(darker, circle[5][idx]);
				neighborhoods16[6] = sub_sat(darker, circle[6][idx]);
				neighborhoods16[7] = sub_sat(darker, circle[7][idx]);
				neighborhoods16[8] = sub_sat(darker, circle[8][idx]);
				neighborhoods16[9] = sub_sat(darker, circle[9][idx]);
				neighborhoods16[10] = sub_sat(darker, circle[10][idx]);
				neighborhoods16[11] = sub_sat(darker, circle[11][idx]);
				neighborhoods16[12] = sub_sat(darker, circle[12][idx]);
				neighborhoods16[13] = sub_sat(darker, circle[13][idx]);
				neighborhoods16[14] = sub_sat(darker, circle[14][idx]);
				neighborhoods16[15] = sub_sat(darker, circle[15][idx]);
				_opencl_fast_strenght();
			}
			else if (sumb >= N) {
				neighborhoods16[0] = sub_sat(circle[0][idx], brighter);
				neighborhoods16[1] = sub_sat(circle[1][idx], brighter);
				neighborhoods16[2] = sub_sat(circle[2][idx], brighter);
				neighborhoods16[3] = sub_sat(circle[3][idx], brighter);
				neighborhoods16[4] = sub_sat(circle[4][idx], brighter);
				neighborhoods16[5] = sub_sat(circle[5][idx], brighter);
				neighborhoods16[6] = sub_sat(circle[6][idx], brighter);
				neighborhoods16[7] = sub_sat(circle[7][idx], brighter);
				neighborhoods16[8] = sub_sat(circle[8][idx], brighter);
				neighborhoods16[9] = sub_sat(circle[9][idx], brighter);
				neighborhoods16[10] = sub_sat(circle[10][idx], brighter);
				neighborhoods16[11] = sub_sat(circle[11][idx], brighter);
				neighborhoods16[12] = sub_sat(circle[12][idx], brighter);
				neighborhoods16[13] = sub_sat(circle[13][idx], brighter);
				neighborhoods16[14] = sub_sat(circle[14][idx], brighter);
				neighborhoods16[15] = sub_sat(circle[15][idx], brighter);
				_opencl_fast_strenght();
			}
			strengths[idx] = strength;
		}
		else {
			strengths[x + (y*stride)] = 0;
		}
	}
	else {
		strengths[x + (get_global_id(1)*stride)] = 0;
	}
}