#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	#
# File author: Mamadou DIOP (Doubango Telecom, France).					#
# License: GPLv3. For commercial license please contact us.				#
# Source code: https://github.com/DoubangoTelecom/compv					#
# WebSite: http://compv.org												#
#########################################################################
.include "compv_common_arm.i"

.section .data

.section .text

.global CompVImageConvRgb24family_to_y_Asm_NEON32

.extern k16_i16
  
CompVImageConvRgb24family_to_y_Asm_NEON32:
    add r0, r0, r1
	bx lr


	