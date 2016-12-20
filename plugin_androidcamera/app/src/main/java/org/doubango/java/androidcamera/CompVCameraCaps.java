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
