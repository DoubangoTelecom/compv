echo "--->[Starting process]<---"
swig -noexcept -c++ -java -D__ANDROID__ -package org.doubango.jni -outdir app/src/main/java/org/doubango/jni -o app/src/main/cpp/androidcamera_swig.cxx swig.i
sed -i 's/dynamic_cast/static_cast/g' app/src/main/cpp/androidcamera_swig.cxx
echo "--->[!! Process done !!]<---"