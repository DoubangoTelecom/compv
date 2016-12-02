/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CAMERA_CAMERA_H_)
#define _COMPV_CAMERA_CAMERA_H_

#include "compv/camera/compv_camera_config.h"
#include "compv/camera/compv_camera_common.h"
#include "compv/base/compv_sharedlib.h"
#include "compv/base/compv_obj.h"

#include <vector>

COMPV_NAMESPACE_BEGIN()

struct CompVCameraDeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    CompVCameraDeviceInfo(const std::string& id_, const std::string& name_, const std::string& description_) {
        id = id_;
        name = name_;
        description = description_;
    }
};
typedef std::vector<CompVCameraDeviceInfo > CompVCameraDeviceInfoList;

COMPV_OBJECT_DECLARE_PTRS(Camera)

typedef COMPV_ERROR_CODE(*CompVCameraNewFunc)(CompVCameraPtrPtr camera);

class COMPV_CAMERA_API CompVCamera : public CompVObj
{
protected:
    CompVCamera();
public:
    virtual ~CompVCamera();

    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
    static COMPV_INLINE bool isInitialized() {
        return s_bInitialized;
    }

    virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) = 0;
    virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") = 0;
    virtual COMPV_ERROR_CODE stop() = 0;

    static COMPV_ERROR_CODE newObj(CompVCameraPtrPtr camera);

    struct CameraFactory {
        CompVCameraNewFunc funcNew;
        CompVSharedLibPtr lib;
        bool isValid() {
            return funcNew && lib;
        }
        void deinit() {
            funcNew = NULL;
            lib = NULL;
        }
    };

private:
    static bool s_bInitialized;
    static CameraFactory s_CameraFactory;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CAMERA_CAMERA_H_ */
