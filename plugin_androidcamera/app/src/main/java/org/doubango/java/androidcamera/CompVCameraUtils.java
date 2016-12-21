/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
package org.doubango.java.androidcamera;

import android.hardware.Camera;
import android.util.Log;

import java.util.List;

public class CompVCameraUtils {
    private static final String TAG = CompVCameraUtils.class.getCanonicalName();
    static final int formatPrefs[] = {
            // Most computer vision features require grayscale image as input.
            // YUV formats first because it's easier to convert to grayscal compared to RGB.
            CompVCamera.PIXEL_FORMAT_NV21, // NV21 (all android devices are required to support this format)
            CompVCamera.PIXEL_FORMAT_YUY2, // YUY2
            // CompVCamera.PIXEL_FORMAT_NV16, // TODO(dmi): NV16
            CompVCamera.PIXEL_FORMAT_RGB,
            CompVCamera.PIXEL_FORMAT_RGBA,
            CompVCamera.PIXEL_FORMAT_RGB565,
    };

    public static CompVCameraCaps getNegCaps(final Camera camera) {
        final Camera.Parameters parameters = camera.getParameters();
        return new CompVCameraCaps(
                parameters.getPreviewSize().width,
                parameters.getPreviewSize().height,
                parameters.getPreviewFrameRate(),
                parameters.getPreviewFormat());
    }

    public static boolean applyCaps(final Camera camera, final CompVCameraCaps capsPref) {
        final Camera.Parameters parameters = camera.getParameters();
        parameters.setPreviewFormat(capsPref.mFormat);
        parameters.setPreviewSize(capsPref.mWidth, capsPref.mHeight);
        parameters.setPreviewFrameRate(capsPref.mFps);
        try {
            camera.setParameters(parameters);
        } catch (Exception e) {
            Log.d(TAG, "applyCaps("+capsPref+") failed." + e);
            return false;
        }
        return true;
    }

    public static CompVCameraCaps getBestCaps(final Camera camera, final CompVCameraCaps capsPref) {
        final List<Integer> formats = camera.getParameters().getSupportedPreviewFormats();
        final List<int[]> fpss = camera.getParameters().getSupportedPreviewFpsRange();
        if (formats.size() != fpss.size()) {
            Log.e(TAG, "Sizes mismatch: " + formats.size() + "<>" + fpss.size());
            return null;
        }
        int idx = 0;
        int formatIdx = -1;
        step0: {
            for (Integer format : formats) {
                if (format == capsPref.mFormat) {
                    formatIdx = idx;
                    break step0;
                }
                ++idx;
            }
        }
        step1: {
            if (formatIdx == -1) {
                idx = 0;
                for (Integer formatPref : formatPrefs) {
                    for (Integer format : formats) {
                        if (formatPref == format) {
                            formatIdx = idx;
                            break step1;
                        }
                    }
                    ++idx;
                }
            }
        }
        if (formatIdx == -1) {
            String formats_ = "";
            for (Integer format : formats) {
                formats_ += getFormatName(format) + ",";
            }
            Log.e(TAG, "Failed to find best format:" + formats_);
            return null;
        }
        Camera.Size sizeBest = getBestSize(camera, capsPref);
        final int[] fpsRange = fpss.get(formatIdx);
        final int fps = capsPref.mFps * 1000;
        final int fpsBest = (fps < fpsRange[0] ? fpsRange[0] : (fps > fpsRange[1] ? fpsRange[1] : fps)) / 1000;
        return new CompVCameraCaps(sizeBest.width, sizeBest.height, fpsBest, formats.get(formatIdx));
    }

    public static String getFormatName(final int format) {
        switch (format) {
            case CompVCamera.PIXEL_FORMAT_NV21: return "NV21";
            case CompVCamera.PIXEL_FORMAT_YUY2: return "YUY2";
            case CompVCamera.PIXEL_FORMAT_NV16: return "NV16";
            case CompVCamera.PIXEL_FORMAT_RGB: return "RGB";
            case CompVCamera.PIXEL_FORMAT_RGBA: return "RGBA";
            case CompVCamera.PIXEL_FORMAT_RGB565: return "RGB565";
            default: return Integer.toString(format);
        }
    }

    public static int getBestFormat(final Camera camera, final CompVCameraCaps capsPref) {
        final List<Integer> prevFormats = camera.getParameters().getSupportedPreviewFormats();
        for(Integer supp : prevFormats) {
            if (supp == capsPref.mFormat) {
                return supp;
            }
        }
        for(Integer pref : formatPrefs) {
            for(Integer supp : prevFormats) {
                if (supp == pref) {
                    return supp;
                }
            }
        }
        return formatPrefs[0]; // NV21
    }

    public static Camera.Size getBestSize(final Camera camera, final CompVCameraCaps capsPref) {
        final List<Camera.Size> prevSizes = camera.getParameters().getSupportedPreviewSizes();
        Camera.Size minSize = null;
        int minScore = Integer.MAX_VALUE;
        int score;
        for(Camera.Size size : prevSizes){
            score = Math.abs(size.width - capsPref.mWidth) + Math.abs(size.height - capsPref.mHeight);
            if(minScore > score){
                minScore = score;
                minSize = size;
            }
        }
        return minSize;
    }
}
