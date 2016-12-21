/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
package org.doubango.java.androidcamera;

import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.doubango.jni.CompVCameraAndroidProxy;

public class CompVCamera {
    private static final String TAG = CompVCamera.class.getCanonicalName();
    private static final int CALLABACK_BUFFERS_COUNT = 3;
    private final CompVCameraAndroidProxy mProxy;
    private final PreviewCallback mPreviewCallback;
    private CompVCameraCaps mCapsPref;
    private CompVCameraCaps mCapsNeg;
    private boolean mStarted = false;
    private ByteBuffer mVideoFrame;
    private int mVideoFrameSize;
    private byte[] mVideoCallbackBytes;
    private Camera mCamera;
    private int mCameraId;
    private SurfaceTexture mSurfaceTexture;
    private SurfaceTexture mSurfaceTextureDummy; // to avoid garbage collection

    // Next fields are used in C++ code using reflexion
    public static final int PIXEL_FORMAT_NV21 = PixelFormat.YCbCr_420_SP; // all android devices are required to support this format
    public static final int PIXEL_FORMAT_YUY2 = PixelFormat.YCbCr_422_I;
    public static final int PIXEL_FORMAT_NV16 = PixelFormat.YCbCr_422_SP; // TODO(dmi): add support for NV16
    public static final int PIXEL_FORMAT_RGB565 = PixelFormat.RGB_565;
    public static final int PIXEL_FORMAT_RGBA = PixelFormat.RGBA_8888;
    public static final int PIXEL_FORMAT_RGB = PixelFormat.RGB_888;

    static {
        System.loadLibrary("androidcamera");
    }

    public CompVCamera() {
        mProxy = new CompVCameraAndroidProxy();
        mCapsPref = new CompVCameraCaps();
        mCapsNeg = new CompVCameraCaps();
        mCameraId = 0;

        mPreviewCallback = new PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] _data, Camera _camera) {
                if (mStarted) {
                    final int _dataLength = _data == null ? 0 : _data.length;
                    if (mVideoFrameSize != _dataLength) {
                        Log.e(TAG, "Video frame size mismatch: " + mVideoFrameSize + "<>" + _dataLength);
                    }
                    else {
                        mVideoFrame.put(_data);
                        mProxy.pushFrame(mVideoFrame, mVideoFrameSize, mCapsNeg.mWidth, mCapsNeg.mHeight, mCapsNeg.mFps, mCapsNeg.mFormat);
                        mVideoFrame.rewind();
                    }
                    // do not use "_data" which could be null (e.g. on GSII)
                    mCamera.addCallbackBuffer(_data == null ? mVideoCallbackBytes : _data);
                } else {
                    Log.w(TAG, "onPreviewFrame called while the camera is stopped");
                }
            }
        };
    }

    @Override
    protected void finalize() throws Throwable {
        Log.d(TAG, "finalize()");
        close();
        super.finalize();
    }

    public static int getNumberOfCameras() {
        return Camera.getNumberOfCameras();
    }

    public static String getCameraInfo(int cameraId) {
        if (cameraId < 0 || cameraId >= getNumberOfCameras()) {
            throw new ArrayIndexOutOfBoundsException();
        }
        final int numCamera = getNumberOfCameras();
        final Camera.CameraInfo info = new Camera.CameraInfo();
        for (int id = 0; id < numCamera; ++id) {
            if (id == cameraId) {
                Camera.getCameraInfo(cameraId, info);
                return String.format("%d %d %d", cameraId, info.orientation, info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT ? 1 : 0);
            }
        }
        throw new ArrayIndexOutOfBoundsException();
    }

    public boolean open(int cameraId) {
        Log.d(TAG, "open("+cameraId+")");
        if (mCameraId != cameraId) {
            close();
        }
        if (mCamera == null) {
            mCamera = Camera.open(cameraId);
            if (mCamera == null) {
                Log.e(TAG, "Camera.open() failed");
                return false;
            }
            mCameraId = cameraId;
        }
        return (mCamera != null);
    }

    public boolean close() {
        Log.d(TAG, "close()");
        if (mCamera != null) {
            stop();
            mCamera.release();
            mCamera = null;
        }
        return true;
    }

    public boolean start(int cameraId) {
        Log.d(TAG, "start(" + cameraId + ")");
        if (mStarted) {
            Log.d(TAG, "camera already started");
            return true;
        }
        if (!open(cameraId)) {
            Log.e(TAG, "Failed to open the camera for start");
            return false;
        }

        /* apply caps */
        Log.d(TAG, "Trying to apply preferred caps: " + mCapsPref);
        boolean retBool = CompVCameraUtils.applyCaps(mCamera, mCapsPref);
        if (!retBool) {
            Log.w(TAG, "Failed to apply preferred caps: " + mCapsPref);
            final CompVCameraCaps capsBest = CompVCameraUtils.getBestCaps(mCamera, mCapsPref);
            if (capsBest == null) {
                Log.e(TAG, "Failed to get best caps");
                close();
                return false;
            }
            Log.d(TAG, "Trying to apply best caps: " + capsBest);
            retBool = CompVCameraUtils.applyCaps(mCamera, capsBest);
            if (!retBool) {
                Log.e(TAG, "Failed to apply best caps:" + capsBest);
                close();
                return false;
            }
        }
        mCapsNeg = CompVCameraUtils.getNegCaps(mCamera);
        Log.d(TAG, "Negotiated caps: " + mCapsNeg);

        /* Compute buffer size */
        switch (mCapsNeg.mFormat) {
            case PIXEL_FORMAT_NV21:
                mVideoFrameSize = ((mCapsNeg.mWidth * mCapsNeg.mHeight) * 3) >> 1;
                break;
            case PIXEL_FORMAT_YUY2:
            case PIXEL_FORMAT_RGB565:
                mVideoFrameSize = (mCapsNeg.mWidth * mCapsNeg.mHeight) << 1;
                break;
            case PIXEL_FORMAT_RGBA:
                mVideoFrameSize = (mCapsNeg.mWidth * mCapsNeg.mHeight) << 2;
                break;
            case PIXEL_FORMAT_RGB:
                mVideoFrameSize = (mCapsNeg.mWidth * mCapsNeg.mHeight) * 3;
                break;
            default:
                Log.e(TAG, "Invalid pixel format: " + mCapsNeg.mFormat);
                close();
                return false;
        }
        Log.d(TAG, "Frame buffer size = " + mVideoFrameSize);
        try {
            mVideoFrame = ByteBuffer.allocateDirect(mVideoFrameSize);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Failed to allocate buffer: " + mVideoFrameSize);
            close();
            return false;
        }

        mCamera.setPreviewCallbackWithBuffer(mPreviewCallback);

        mVideoCallbackBytes = new byte[mVideoFrame.capacity()];
        for (int i = 0; i < CALLABACK_BUFFERS_COUNT; ++i) {
            mCamera.addCallbackBuffer(new byte[mVideoFrame.capacity()]);
        }

        /* Set surface texture */
        try {
            if (mSurfaceTexture == null) {
                Log.w(TAG, "setPreviewTexture will be called with null texture");
                if (mSurfaceTextureDummy == null) {
                    mSurfaceTextureDummy = new SurfaceTexture(0);
                }
            }
            final SurfaceTexture surfaceTexture = (mSurfaceTexture == null ? mSurfaceTextureDummy : mSurfaceTexture);
            Log.d(TAG, "setPreviewTexture(" + surfaceTexture + ")");
            mCamera.setPreviewTexture(surfaceTexture);
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "setPreviewTexture() failed" + e);
            close();
            return false;
        }

        mStarted = true;
        mCamera.startPreview();

        return true;
    }

    public boolean start() {
        return start(Math.max(getNumberOfCameras() - 1, 0));
    }

    public boolean stop() {
        Log.d(TAG, "stop()");
        if (mCamera != null) {
            mCamera.setPreviewCallbackWithBuffer(null);
            mCamera.stopPreview();
        }
        mStarted = false;
        return true;
    }

    public boolean setSurfaceTextureName(int textureName) {
        Log.d(TAG, "setUseFront(" + textureName + ")");
        final SurfaceTexture surfaceTexture = new SurfaceTexture(textureName);
        if (mCamera != null) {
            try {
                mCamera.setPreviewTexture(surfaceTexture);
            } catch (IOException e) {
                e.printStackTrace();
                Log.e(TAG, "setPreviewTexture(" + mSurfaceTexture + ") failed" + e);
                return false;
            }
        }
        mSurfaceTexture = surfaceTexture;
        return true;
    }

    public boolean setSurfaceTexture(SurfaceTexture surfaceTexture) {
        Log.d(TAG, "setSurfaceTexture(" + surfaceTexture + ")");
        if (mCamera != null) {
            try {
                mCamera.setPreviewTexture(surfaceTexture);
            } catch (IOException e) {
                e.printStackTrace();
                Log.e(TAG, "setPreviewTexture(" + surfaceTexture + ") failed" + e);
                return false;
            }
        }
        mSurfaceTexture = surfaceTexture;
        return true;
    }

    public void setCaps(CompVCameraCaps caps) { mCapsPref = caps; }
    public void setCaps(int width, int height, int fps, int format) { mCapsPref = new CompVCameraCaps(width, height, fps, format); }
    public CompVCameraCaps getCapsNeg() { return mCapsNeg; }
    public CompVCameraCaps getCapsPref() { return mCapsPref; }

    public CompVCameraAndroidProxy getProxy() {
        return mProxy;
    }

    public void setCallbackFunc(long funcptr, long userData) {
        mProxy.setCallbackFunc(funcptr, userData);
    }
}
