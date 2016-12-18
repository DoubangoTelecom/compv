/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_UTILS_H_
#define _COMPV_PLUGIN_MFOUNDATION_UTILS_H_

#include "compv/mf/compv_mf_config.h"
#include "compv/mf/compv_mf_devices.h"
#include "compv/camera/compv_camera.h" /* CompVCameraDeviceInfoList */

#include <new>
#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>
#include <shlwapi.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMFUtils
{
public:
	static const char* guidName(__in const GUID& guid);

	static HRESULT startup();
	static HRESULT shutdown();

	static COMPV_ERROR_CODE devices(__out CompVCameraDeviceInfoList& list);
	static COMPV_ERROR_CODE device(__in const char* pszId, __out IMFActivate **ppActivate);

	static COMPV_ERROR_CODE convertSubType(__in const COMPV_SUBTYPE& subTypeIn, __out GUID &subTypeOut);
	static COMPV_ERROR_CODE convertSubType(__in const GUID &subTypeIn, __out COMPV_SUBTYPE &subTypeOut);

	static HRESULT isAsyncMFT(
		IMFTransform *pMFT, // The MFT to check
		BOOL* pbIsAsync // Whether the MFT is Async
	);
	static HRESULT unlockAsyncMFT(
		IMFTransform *pMFT // The MFT to unlock
	);

	static HRESULT createPCMAudioType(
		UINT32 sampleRate,        // Samples per second
		UINT32 bitsPerSample,     // Bits per sample
		UINT32 cChannels,         // Number of channels
		IMFMediaType **ppType     // Receives a pointer to the media type.
	);
	static HRESULT createVideoType(
		const GUID* subType, // video subType
		IMFMediaType **ppType,     // Receives a pointer to the media type.
		UINT32 unWidth = 0, // Video width (0 to ignore)
		UINT32 unHeight = 0 // Video height (0 to ignore)
	);
	static HRESULT convertVideoTypeToUncompressedType(
		IMFMediaType *pType,    // Pointer to an encoded video type.
		const GUID& subtype,    // Uncompressed subtype (eg, RGB-32, AYUV)
		IMFMediaType **ppType   // Receives a matching uncompressed video type.
	);
	static HRESULT createMediaSample(
		DWORD cbData, // Maximum buffer size
		IMFSample **ppSample // Receives the sample
	);
	static HRESULT validateVideoFormat(
		IMFMediaType *pmt
	);
	static HRESULT isVideoProcessorSupported(BOOL *pbSupported);
	static HRESULT bestVideoProcessor(
		const GUID& inputFormat, // The input MediaFormat (e.g. MFVideoFormat_I420)
		const GUID& outputFormat, // The output MediaFormat (e.g. MFVideoFormat_NV12)
		IMFTransform **ppProcessor // Receives the video processor
	);
	static HRESULT bindOutputNode(
		IMFTopologyNode *pNode // The Node
	);
	static HRESULT addOutputNode(
		IMFTopology *pTopology,     // Topology.
		IMFActivate *pActivate,     // Media sink activation object.
		DWORD dwId,                 // Identifier of the stream sink.
		IMFTopologyNode **ppNode   // Receives the node pointer.
	);
	static HRESULT addTransformNode(
		IMFTopology *pTopology,     // Topology.
		IMFTransform *pMFT,     // MFT.
		DWORD dwId,                 // Identifier of the stream sink.
		IMFTopologyNode **ppNode   // Receives the node pointer.
	);
	static HRESULT addSourceNode(
		IMFTopology *pTopology,           // Topology.
		IMFMediaSource *pSource,          // Media source.
		IMFPresentationDescriptor *pPD,   // Presentation descriptor.
		IMFStreamDescriptor *pSD,         // Stream descriptor.
		IMFTopologyNode **ppNode          // Receives the node pointer.
	);
	static HRESULT createTopology(
		IMFMediaSource *pSource, // Media source
		IMFTransform *pTransform, // Transform filter (e.g. encoder or decoder) to insert between the source and Sink. NULL is valid.
		IMFActivate *pSinkActivateMain, // Main sink (e.g. sample grabber or EVR).
		IMFActivate *pSinkActivatePreview, // Preview sink. Optional. Could be NULL.
		IMFMediaType *pIputTypeMain, // Main sink input MediaType
		IMFTopology **ppTopo // Receives the newly created topology
	);
	static HRESULT resolveTopology(
		IMFTopology *pInputTopo, // A pointer to the IMFTopology interface of the partial topology to be resolved.
		IMFTopology **ppOutputTopo, // Receives a pointer to the IMFTopology interface of the completed topology. The caller must release the interface.
		IMFTopology *pCurrentTopo = NULL // A pointer to the IMFTopology interface of the previous full topology. The topology loader can re-use objects from this topology in the new topology. This parameter can be NULL.
	);
	static HRESULT findNodeObject(
		IMFTopology *pInputTopo, // The Topology containing the node to find
		TOPOID qwTopoNodeID, //The identifier for the node
		void** ppObject // Receives the Object
	);
	static HRESULT createMediaSinkActivate(
		IMFStreamDescriptor *pSourceSD,     // Pointer to the stream descriptor.
		HWND hVideoWindow,                  // Handle to the video clipping window.
		IMFActivate **ppActivate
	);
	static HRESULT setMediaType(
		IMFMediaSource *pSource,        // Media source.
		IMFMediaType* pMediaType // Media Type.
	);
	static HRESULT setVideoWindow(
		IMFTopology *pTopology,         // Topology.
		IMFMediaSource *pSource,        // Media source.
		HWND hVideoWnd                 // Window for video playback.
	);
	static HRESULT runSession(
		IMFMediaSession *pSession, // Session to run
		IMFTopology *pTopology // The toppology
	);
	static HRESULT shutdownSession(
		IMFMediaSession *pSession, // The Session
		IMFMediaSource *pSource = NULL // Source to shutdown (optional)
	);
	static HRESULT pauseSession(
		IMFMediaSession *pSession, // The session
		IMFMediaSource *pSource = NULL// Source to pause (optional)
	);
	static HRESULT supportedCaps(
		__in IMFMediaSource *pSource, // The source
		__out std::vector<CompVMFCameraCaps>& caps,
		__in const GUID& majorType = MFMediaType_Video // The MediaType
	);
	static HRESULT isSupported(
		IMFPresentationDescriptor *pPD,
		DWORD cStreamIndex,
		UINT32 nWidth,
		UINT32 nHeight,
		UINT32 nFps,
		const GUID& guidFormat,
		BOOL* pbSupportedSize,
		BOOL* pbSupportedFps,
		BOOL* pbSupportedFormat
	);
	static HRESULT isSupported(
		IMFPresentationDescriptor *pPD,
		DWORD cStreamIndex,
		IMFMediaType* pMediaType,
		BOOL* pbSupportedSize,
		BOOL* pbSupportedFps,
		BOOL* pbSupportedFormat
	);
	static HRESULT isSupportedByInput(
		IMFPresentationDescriptor *pPD,
		DWORD cStreamIndex,
		IMFTopologyNode *pNode,
		BOOL* pbSupportedSize,
		BOOL* pbSupportedFps,
		BOOL* pbSupportedFormat
	);
	static HRESULT connectConverters(
		IMFTopologyNode *pNode,
		DWORD dwOutputIndex,
		IMFTopologyNode *pNodeConvFrameRate,
		IMFTopologyNode *pNodeConvColor,
		IMFTopologyNode *pNodeConvSize
	);

	template <class Q>
	static HRESULT GetTopoNodeObject(IMFTopologyNode *pNode, Q **ppObject) {
		IUnknown *pUnk = NULL;   // zero output

		HRESULT hr = pNode->GetObject(&pUnk);
		if (SUCCEEDED(hr)) {
			pUnk->QueryInterface(IID_PPV_ARGS(ppObject));
			pUnk->Release();
		}
		return hr;
	}

public:
	static const TOPOID s_ullTopoIdSinkMain;
	static const TOPOID s_ullTopoIdSinkPreview;
	static const TOPOID s_ullTopoIdSource;
	static const TOPOID s_ullTopoIdVideoProcessor;

private:
	static bool s_bStarted;
	static CompVMFDeviceListVideo* s_DeviceListVideo;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_MFOUNDATION_UTILS_H_ */
