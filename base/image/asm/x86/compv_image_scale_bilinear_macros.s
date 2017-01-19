;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%define _mm_bilinear_insert_x86_sse41(vecDst, val32, index)  pinsrd vecDst, val32, index
%define _mm_bilinear_insert_at_0_x86_sse41(vecDst, val32)    movd vecDst, val32
%define _mm_bilinear_insert_at_1_x86_sse41(vecDst, val32)    _mm_bilinear_insert_x86_sse41(vecDst, val32, 1)
%define _mm_bilinear_insert_at_2_x86_sse41(vecDst, val32)    _mm_bilinear_insert_x86_sse41(vecDst, val32, 2)
%define _mm_bilinear_insert_at_3_x86_sse41(vecDst, val32)    _mm_bilinear_insert_x86_sse41(vecDst, val32, 3)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This macro overrides rdx, rdi, rax and rcx
%macro _mm_bilinear_extract_then_insert_x86_sse41 7
	%define vecNeareastX       %1
	%define neareastIndex0     %2
	%define neareastIndex1     %3
	%define vecNeighbA         %4
	%define vecNeighbB         %5
	%define neighbIndex        %6
	%define inPtr_             %7

	;;; Extract indices(neareastIndex0, neareastIndex1) ;;;
	pextrd eax, vecNeareastX, neareastIndex0 ; rax = nearestX0
	pextrd ecx, vecNeareastX, neareastIndex1 ; rcx = nearestX1
	;; Insert in vecNeighbA(neighbIndex)
	movzx rdx, word [inPtr_ + rax] ; rdx = inPtr_[nearestX0]
	movzx rdi, word [inPtr_ + rcx] ; rdi = inPtr_[nearestX1]
	shl rdi, 16 ; rdi = (inPtr_[nearestX1] << 16)
	or rdx, rdi ; rdx = inPtr_[nearestX0] | (inPtr_[nearestX1] << 16)
	_mm_bilinear_insert_at_ %+ neighbIndex %+ _x86_sse41(vecNeighbA, edx)
	;; Insert in vecNeighbB(neighbIndex)
	add rax, arg_inStride ; rax = (nearestX0 + inStride)
	add rcx, arg_inStride ; rcx = (nearestX1 + inStride)
	movzx rdx, word [inPtr_ + rax] ; rdx = inPtr_[nearestX0 + inStride]
	movzx rdi, word [inPtr_ + rcx] ; rdi = inPtr_[nearestX1 + inStride]
	shl rdi, 16 ; rdi = (inPtr_[nearestX1 + inStride] << 16)
	or rdx, rdi ; rdx = inPtr_[nearestX0 + inStride] | (inPtr_[nearestX1 + inStride] << 16)
	_mm_bilinear_insert_at_ %+ neighbIndex %+ _x86_sse41(vecNeighbB, edx)

	%undef vecNeareastX
	%undef neareastIndex0
	%undef neareastIndex1
	%undef vecNeighbA
	%undef vecNeighbB
	%undef neighbIndex
	%undef inPtr_
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This macro overrides rdx, rdi, rax and rcx
%macro _mm_bilinear_set_neighbs_x86_sse41 6
	;%define vecNeareastX       %1
	;%define vecNeighbA         %2
	;%define vecNeighbB         %3
	;%define neighbIndex0       %4
	;%define neighbIndex1       %5
	;%define inPtr_             %6
	
	_mm_bilinear_extract_then_insert_x86_sse41  %1, 0, 1, %2, %3, %4, %6
	_mm_bilinear_extract_then_insert_x86_sse41  %1, 2, 3, %2, %3, %5, %6
				
	;%undef vecNeareastX
	;%undef vecNeighbA
	;%undef vecNeighbB
	;%undef neighbIndex0
	;%undef neighbIndex1
	;%undef inPtr_
%endmacro
