/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
package org.doubango.java.androidcamera;

public class CompVCameraCaps {
    public int mWidth;
    public int mHeight;
    public int mFps;
    public int mFormat;

    public CompVCameraCaps() {
        this(640, 480, 25, CompVCamera.PIXEL_FORMAT_NV21);
    }

    public CompVCameraCaps(int width, int height, int fps, int format) {
        mWidth = width;
        mHeight = height;
        mFps = fps;
        mFormat = format;
    }

    @Override
    public String toString() {
        return String.format("width=%d, height=%d, fps=%d, format=%s", mWidth, mHeight, mFps, CompVCameraUtils.getFormatName(mFormat));
    }
}
