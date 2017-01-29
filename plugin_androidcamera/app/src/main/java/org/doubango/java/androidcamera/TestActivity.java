/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
package org.doubango.java.androidcamera;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.Log;
import android.view.TextureView;

public class TestActivity extends Activity  implements TextureView.SurfaceTextureListener{
    private static final String TAG = TestActivity.class.getCanonicalName();
    private CompVCamera mCamera;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);

        CompVCameraCaps caps = new CompVCameraCaps(1280, 720, 25, CompVCamera.PIXEL_FORMAT_YUY2, true);
        mCamera = new CompVCamera();
        mCamera.setCaps(caps);

        TextureView tv = new TextureView(this);
        tv.setSurfaceTextureListener(this);
        setContentView(tv);
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();


    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onResume();
        mCamera.stop();
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        Log.d(TAG, "onSurfaceTextureAvailable");
        mCamera.setSurfaceTexture(surface);
        mCamera.start(0);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        Log.d(TAG, "onSurfaceTextureSizeChanged");
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        Log.d(TAG, "onSurfaceTextureDestroyed");
        mCamera.setSurfaceTexture(null);
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        //Log.d(TAG, "onSurfaceTextureUpdated");
    }
}
