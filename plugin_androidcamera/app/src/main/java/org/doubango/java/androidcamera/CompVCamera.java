package org.doubango.java.androidcamera;

import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import org.doubango.jni.CompVCameraAndroidProxy;

public class CompVCamera {
    private static final String TAG = CompVCamera.class.getCanonicalName();
    private static final int CALLABACK_BUFFERS_COUNT = 3;
    private static final int PREF_PIXEL_FORMAT = PixelFormat.YCbCr_420_SP;
    private final CompVCameraAndroidProxy mProxy;
    private final PreviewCallback mPreviewCallback;
    private boolean mStarted = false;
    private ByteBuffer mVideoFrame;
    private int mVideoFrameSize;
    private byte[] mVideoCallbackBytes;
    private Camera mCamera;
    private int mWidthPref = 640;
    private int mHeightPref = 480;
    private int mWidthFrame = mWidthPref;
    private int mHeightFrame = mHeightPref;
    private int mPixelFormat = PREF_PIXEL_FORMAT; // NV21, supported by all Android devices
    private SurfaceTexture mSurfaceTexture;
    private SurfaceTexture mSurfaceTextureDummy; // to avoid garbage collection
    private int mCameraFacingPref = Camera.CameraInfo.CAMERA_FACING_FRONT;

    static {
        System.loadLibrary("androidcamera");
    }

    public CompVCamera() {
        mProxy = new CompVCameraAndroidProxy();

        mPreviewCallback = new PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] _data, Camera _camera) {
                if (mStarted) {
                    if (mVideoFrameSize != _data.length) {
                        Log.e(TAG, "Video frame size mismatch: " + mVideoFrameSize + "<>" + _data.length);
                        return;
                    }
                    mVideoFrame.put(_data);
                    mProxy.pushFrame(mVideoFrame, mVideoFrameSize);
                    mVideoFrame.rewind();

                    // do not use "_data" which could be null (e.g. on GSII)
                    mCamera.addCallbackBuffer(_data == null ? mVideoCallbackBytes : _data);
                }
                else {
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

    public boolean open() {
        Log.d(TAG, "open()");
        if (mCamera == null) {
            int numCamera = Camera.getNumberOfCameras();
            if (numCamera <= 0) {
                Log.e(TAG, "numCamera="+numCamera);
                return false;
            }
            // Open preferred camera
            Camera.CameraInfo info = new Camera.CameraInfo();
            for (int c = 0; c < numCamera; ++c) {
                Camera.getCameraInfo(c, info);
                if (info.facing == mCameraFacingPref) {
                    mCamera = Camera.open(c);
                    break;
                }
            }
            // Open default camera
            if (mCamera == null) {
                mCamera = Camera.open(numCamera - 1);
            }
            if (mCamera == null) {
                Log.e(TAG, "Camera.open() failed");
                return false;
            }
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

    public boolean start() {
        Log.d(TAG, "start");
        if (mStarted) {
            Log.d(TAG, "camera already started");
            return true;
        }
        if (!open()) {
            Log.e(TAG, "Failed to open the camera for start");
            return false;
        }

        final Camera.Parameters parameters = mCamera.getParameters();

        /* Set pixel format */
        mPixelFormat = PREF_PIXEL_FORMAT;
        try {
            parameters.setPreviewFormat(mPixelFormat);
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "setPreviewFormat(" + mPixelFormat + ") failed" + e);
        }
        mPixelFormat = parameters.getPreviewFormat();
        Log.d(TAG, "mPixelFormat=" + mPixelFormat);

        /* Set preview size */
        final Camera.Size sizePref = getBestPreviewSize();
        try {
            parameters.setPreviewSize(sizePref.width, sizePref.height);
            mCamera.setParameters(parameters);
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "setPreviewSize(" + sizePref.width + ", sizePref.height) failed" + e);
        }
        mWidthFrame = parameters.getPreviewSize().width;
        mHeightFrame = parameters.getPreviewSize().height;
        Log.d(TAG, "mWidthPref=" + mWidthPref + ", mHeightPref=" + mHeightPref + ", mWidthFrame=" + mWidthFrame + ", mHeightFrame=" + mHeightFrame);

        /* Set framerate */
        // parameters.setPreviewFrameRate(NgnCameraProducer.fps);

        /* Compute buffer size */
        switch (mPixelFormat) {
            case PixelFormat.YCbCr_420_SP: // NV21
                mVideoFrameSize = ((mWidthFrame * mHeightFrame) * 3) >> 1;
                break;
            case PixelFormat.YCbCr_422_SP:
            case PixelFormat.YCbCr_422_I: // YUY2
            case PixelFormat.RGB_565:
                mVideoFrameSize = (mWidthFrame * mHeightFrame) << 1;
                break;
            case PixelFormat.RGBA_8888:
            case PixelFormat.RGBX_8888:
                mVideoFrameSize = (mWidthFrame * mHeightFrame) << 2;
                break;
            case PixelFormat.RGB_888:
                mVideoFrameSize = (mWidthFrame * mHeightFrame) * 3;
                break;
            default:
                Log.e(TAG, "Invalid pixel format: " + mPixelFormat);
                close();
                return false;
        }
        Log.d(TAG, "Frame buffer size = " + mVideoFrameSize);
        try {
            mVideoFrame = ByteBuffer.allocateDirect(mVideoFrameSize);
        } catch(Exception e){
            e.printStackTrace();
            Log.e(TAG, "Failed to allocate buffer: " + mVideoFrameSize);
            close();
            return false;
        }

        mCamera.setPreviewCallbackWithBuffer(mPreviewCallback);

        mVideoCallbackBytes = new byte[mVideoFrame.capacity()];
        for (int i = 0; i< CALLABACK_BUFFERS_COUNT; ++i) {
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
            final SurfaceTexture surfaceTexture = (mSurfaceTexture == null ?  mSurfaceTextureDummy : mSurfaceTexture);
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

    public boolean stop() {
        Log.d(TAG, "stop()");
        if (mCamera != null) {
            mCamera.setPreviewCallbackWithBuffer(null);
            mCamera.stopPreview();
        }
        mStarted = false;
        return true;
    }

    public boolean setUseFront(boolean useFront) {
        Log.d(TAG, "setUseFront("+useFront+")");
        mCameraFacingPref = useFront ? Camera.CameraInfo.CAMERA_FACING_FRONT : Camera.CameraInfo.CAMERA_FACING_BACK;
        return true;
    }

    public boolean setSurfaceTextureName(int textureName) {
        Log.d(TAG, "setUseFront("+textureName+")");
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
        Log.d(TAG, "setSurfaceTexture("+surfaceTexture+")");
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

    public int getPixelFormat() {
        return mPixelFormat;
    }

    public CompVCameraAndroidProxy getProxy() {
        return mProxy;
    }

    public void setCallbackFunc(long funcptr, long userData) {
        mProxy.setCallbackFunc(funcptr, userData);
    }

    private Camera.Size getBestPreviewSize(){
        final List<Camera.Size> prevSizes = mCamera.getParameters().getSupportedPreviewSizes();
        Camera.Size minSize = null;
        int minScore = Integer.MAX_VALUE;
        for (Camera.Size size : prevSizes){
            final int score = Math.abs(size.width - mWidthPref) + Math.abs(size.height - mHeightPref);
            if (minScore > score){
                minScore = score;
                minSize = size;
            }
        }
        return minSize;
    }
}
