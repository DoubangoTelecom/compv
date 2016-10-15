/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_UI_H_)
#define _COMPV_UI_UI_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/parallel/compv_mutex.h"

#include "compv/ui/compv_window.h"

#include <map>

COMPV_NAMESPACE_BEGIN()

class COMPV_UI_API CompVUI : public CompVObj
{
	friend class CompVWindow;
protected:
	CompVUI();
public:
	virtual ~CompVUI();
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static COMPV_INLINE bool isInitialized() { return s_bInitialized; }
	static COMPV_INLINE bool isLoopRunning() { return s_bLoopRunning; }

	static size_t windowsCount();
	static COMPV_ERROR_CODE runLoop();
	static COMPV_ERROR_CODE breakLoop();

private:
	static COMPV_ERROR_CODE registerWindow(CompVPtr<CompVWindow* > window);
	static COMPV_ERROR_CODE unregisterWindow(CompVPtr<CompVWindow* > window);
	static COMPV_ERROR_CODE unregisterWindow(compv_window_id_t windowId);

private:
	static bool s_bInitialized;
	static bool s_bLoopRunning;
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static std::map<compv_window_id_t, CompVPtr<CompVWindow* > > m_sWindows;
	static CompVPtr<CompVMutex* > m_sWindowsMutex;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_UI_UI_H_ */
