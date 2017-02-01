/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
// Requires OpenCL 1.1
__kernel void clConvlt1VtHz_8u8u32f(
	__global const unsigned char* inPtr
	__global unsigned char* outPtr
	unsigned int width
	unsigned int height
	unsigned int step
	unsigned int pad
	__global const float* vthzKernPtr // FIXME: make local
	unsigned int kernSize
)
{
	
}