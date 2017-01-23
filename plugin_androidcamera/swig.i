%module(directors="1") androidcamera
%include "typemaps.i"
%include <stdint.i>

// Mapping void* (Java to C) as ByteBuffer
%typemap(jni) void * "jbyteArray"
%typemap(jtype) void * "java.nio.ByteBuffer"
%typemap(jstype) void * "java.nio.ByteBuffer"
%typemap(javain) void * "$javainput"
%typemap(javaout) void * { return $jnicall; }

// (From Java to C)
%typemap(in) void * %{ 
	$1 = jenv->GetDirectBufferAddress($input); 
%}

%{
#include "compv/camera/android/compv_camera_android_proxy.h"
%}
%nodefaultctor;
%include "../camera/include/compv/camera/android/compv_camera_android_proxy.h"
%clearnodefaultctor;
