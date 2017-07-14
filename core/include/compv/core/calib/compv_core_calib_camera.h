/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CALIB_CAMERA_H_)
#define _COMPV_CORE_CALIB_CAMERA_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_features.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CalibCamera)

enum COMPV_CALIB_CAMERA_RESULT_CODE {
	COMPV_CALIB_CAMERA_RESULT_NONE,
	COMPV_CALIB_CAMERA_RESULT_OK,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS,
};

struct CompVCalibCameraResult {
	COMPV_CALIB_CAMERA_RESULT_CODE code;
	CompVHoughLineVector hough_lines; // polar coords.
	CompVLineFloat32Vector grouped_lines; // cartesian coords.
	CompVMatPtr edges;
public:
	void reset() {
		code = COMPV_CALIB_CAMERA_RESULT_NONE;
		hough_lines.clear();
		grouped_lines.clear();
		edges = nullptr;
	}
};

class COMPV_CORE_API CompVCalibCamera : public CompVObj
{
protected:
	CompVCalibCamera();

public:
	virtual ~CompVCalibCamera();
	COMPV_OBJECT_GET_ID(CompVBoxInterestPoint);

	COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVCalibCameraResult& result);

	COMPV_INLINE CompVEdgeDetePtr edgeDetector() { return m_ptrCanny; }
	COMPV_INLINE CompVHoughPtr houghTransform() { return m_ptrHough; }
	
	static COMPV_ERROR_CODE newObj(CompVCalibCameraPtrPtr calib);

private:
	
	

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	size_t m_nPatternCornersNumRow;
	size_t m_nPatternCornersNumCol;
	size_t m_nPatternCornersTotal;
	size_t m_nPatternLinesTotal;
	CompVEdgeDetePtr m_ptrCanny;
	CompVHoughPtr m_ptrHough;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_CAMERA_H_ */
