/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_utils.h"
#include "compv/mf/compv_mf_guidname.h"
#include "compv/base/compv_debug.h"

#include <KS.h>/*  KS.H must be included before codecapi.H */
#include <Codecapi.h>
#include <initguid.h>
#include <wmcodecdsp.h>

#define COMPV_THIS_CLASSNAME "CompVMFUtils"

// Video Processor
// https://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx
DEFINE_GUID(CLSID_VideoProcessorMFT,
	0x88753b26, 0x5b24, 0x49bd, 0xb2, 0xe7, 0xc, 0x44, 0x5c, 0x78, 0xc9, 0x82);

const char* CompVMFUtilsGuidName(const GUID& guid) { return compv::CompVMFUtils::guidName(guid); } // Used in 'mf_config.h' to avoid including 'mf_utils.h'(recurssive include)

COMPV_NAMESPACE_BEGIN()

struct {
	COMPV_SUBTYPE subType;
	const GUID& guid;
} static const kCompVSubTypeGuidPairs[] = {
	// Most computer vision features require grayscale image as input.
	// YUV formats first because it's easier to convert to grayscal compared to RGB.
	{ COMPV_SUBTYPE_PIXELS_YUV420P, MFVideoFormat_I420 }, // YUV420P, IYUV, I420: used by MPEG codecs. Planar and easy to convert to grayscale.
	{ COMPV_SUBTYPE_PIXELS_NV12, MFVideoFormat_NV12 }, // Used by NVIDIA CUDA
	{ COMPV_SUBTYPE_PIXELS_UYVY, MFVideoFormat_UYVY },
	{ COMPV_SUBTYPE_PIXELS_YUY2, MFVideoFormat_YUY2 },
	{ COMPV_SUBTYPE_PIXELS_YV12, MFVideoFormat_YV12 },

	// RGB formats as fallback
	{ COMPV_SUBTYPE_PIXELS_BGR24, MFVideoFormat_RGB24 },
	{ COMPV_SUBTYPE_PIXELS_BGRA32, MFVideoFormat_RGB32 },
	{ COMPV_SUBTYPE_PIXELS_ABGR32, MFVideoFormat_ARGB32 },
	{ COMPV_SUBTYPE_PIXELS_RGB565LE, MFVideoFormat_RGB565 },
};
static const size_t kCompVSubTypeGuidPairsCount = sizeof(kCompVSubTypeGuidPairs) / sizeof(kCompVSubTypeGuidPairs[0]);

#define CompVFindSubTypePairByGuid(_guid, _index) { \
	*_index = -1; \
	for (int _i = 0; _i < kCompVSubTypeGuidPairsCount; ++_i) { \
		if (InlineIsEqualGUID(kCompVSubTypeGuidPairs[_i].guid, _guid)) { \
			*_index = _i; break; \
		} \
	} \
}

bool CompVMFUtils::s_bStarted = false;
const TOPOID CompVMFUtils::s_ullTopoIdSinkMain = 111111111;
const TOPOID CompVMFUtils::s_ullTopoIdSinkPreview = 222222222;
const TOPOID CompVMFUtils::s_ullTopoIdSource = 333333333;
const TOPOID CompVMFUtils::s_ullTopoIdVideoProcessor = 444444444;
CompVMFDeviceListVideo* CompVMFUtils::s_DeviceListVideo = NULL;

const char* CompVMFUtils::guidName(__in const GUID& guid)
{
	if (InlineIsEqualGUID(guid, MFVideoFormat_I420)) {
		return "MFVideoFormat_I420";
	}
	return GuidNames[guid];
}


HRESULT CompVMFUtils::startup()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	HRESULT hr = S_OK;
	if (!s_bStarted) {
		s_bStarted = true; // required by shutdown for full execution
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		COMPV_CHECK_HRESULT_EXP_BAIL(FAILED(hr) && hr != 0x80010106, hr); // 0x80010106 when called from managed code (e.g. ADAS or VR C# systems) - More info: http://support.microsoft.com/kb/824480
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFStartup(MF_VERSION));
		s_DeviceListVideo = new CompVMFDeviceListVideo();
		COMPV_CHECK_HRESULT_EXP_BAIL(!s_DeviceListVideo, hr = E_OUTOFMEMORY);
		s_bStarted = true;
	}

bail:
	if (FAILED(hr)) {
		COMPV_CHECK_HRESULT_CODE_NOP(CompVMFUtils::shutdown());
	}
	return hr;
}

HRESULT CompVMFUtils::shutdown()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	if (s_bStarted) {
		COMPV_CHECK_HRESULT_CODE_NOP(MFShutdown());
		CoUninitialize();
		if (s_DeviceListVideo) {
			delete s_DeviceListVideo;
			s_DeviceListVideo = NULL;
		}
		s_bStarted = false;
	}
	return S_OK;
}

COMPV_ERROR_CODE CompVMFUtils::devices(__out CompVCameraDeviceInfoList& list)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	CompVCameraDeviceInfoList list_;
	HRESULT hr = S_OK;
	UINT32 count;
	WCHAR *_pszName = NULL;
	WCHAR *_pszId = NULL;
	char pczString[MAX_PATH] = { 0 };
	int length;
	std::string name, id;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = s_DeviceListVideo->enumerateDevices());
	count = s_DeviceListVideo->count();

	for (UINT32 i = 0; i < count; ++i) {
		if (SUCCEEDED(s_DeviceListVideo->deviceName(i, &_pszName)) && SUCCEEDED(s_DeviceListVideo->deviceId(i, &_pszId))) {
			// Unique Id
			if ((length = static_cast<int>(wcstombs(pczString, _pszId, MAX_PATH))) <= 0) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "wcstombs(%ls) failed: %d", _pszId, length);
				goto next;
			}
			id = std::string(pczString, length);
			// Friendly name
			if ((length = static_cast<int>(wcstombs(pczString, _pszName, MAX_PATH))) <= 0) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "wcstombs(%ls) failed: %d", _pszName, length);
				goto next;
			}
			name = std::string(pczString, length);
			// Add to the list
			list_.push_back(CompVCameraDeviceInfo(id, name, name));
		}
next:
		if (_pszName) {
			CoTaskMemFree(_pszName), _pszName = NULL;
		}
		if (_pszId) {
			CoTaskMemFree(_pszId), _pszId = NULL;
		}
	}

	list = list_;

bail:
	if (_pszName) {
		CoTaskMemFree(_pszName);
	}
	if (_pszId) {
		CoTaskMemFree(_pszId);
	}
	COMPV_CHECK_EXP_RETURN(FAILED(hr), COMPV_ERROR_CODE_E_MFOUNDATION);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMFUtils::device(__in const char* pszId, __out IMFActivate **ppActivate)
{
	COMPV_CHECK_EXP_RETURN(!ppActivate, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	wchar_t pczwSrcUniqueId[MAX_PATH] = { 0 };
	int length;

	length = static_cast<int>(mbstowcs(pczwSrcUniqueId, pszId, sizeof(pczwSrcUniqueId) / sizeof(pczwSrcUniqueId[0])));
	COMPV_CHECK_EXP_RETURN(length <= 0, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(FAILED(s_DeviceListVideo->deviceWithId(pczwSrcUniqueId, ppActivate)), COMPV_ERROR_CODE_E_MFOUNDATION);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMFUtils::convertSubType(__in const COMPV_SUBTYPE& subTypeIn, __out GUID &subTypeOut)
{
	for (size_t i = 0; i < kCompVSubTypeGuidPairsCount; ++i) {
		if (kCompVSubTypeGuidPairs[i].subType == subTypeIn) {
			subTypeOut = kCompVSubTypeGuidPairs[i].guid;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	COMPV_DEBUG_ERROR("Media Foundation camera implementation doesn't support subType %d", subTypeIn);
	return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
}

COMPV_ERROR_CODE CompVMFUtils::convertSubType(__in const GUID &subTypeIn, __out COMPV_SUBTYPE &subTypeOut)
{
	for (size_t i = 0; i < kCompVSubTypeGuidPairsCount; ++i) {
		if (InlineIsEqualGUID(kCompVSubTypeGuidPairs[i].guid, subTypeIn)) {
			subTypeOut = kCompVSubTypeGuidPairs[i].subType;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	COMPV_DEBUG_ERROR("CompV doesn't support subType %s", CompVMFUtils::guidName(subTypeIn));
	return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
}

HRESULT CompVMFUtils::isAsyncMFT(
	IMFTransform *pMFT, // The MFT to check
	BOOL* pbIsAsync // Whether the MFT is Async
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pbIsAsync || !pMFT, E_POINTER);

	IMFAttributes *pAttributes = NULL;
	UINT32 nIsAsync = 0;
	HRESULT hr = S_OK;

	hr = pMFT->GetAttributes(&pAttributes);
	if (SUCCEEDED(hr)) {
		hr = pAttributes->GetUINT32(MF_TRANSFORM_ASYNC, &nIsAsync);
	}

	// Never fails: just say not Async
	COMPV_CHECK_HRESULT_CODE_NOP(hr = S_OK);

	*pbIsAsync = !!nIsAsync;
	
	return hr;
}

HRESULT CompVMFUtils::unlockAsyncMFT(
	IMFTransform *pMFT // The MFT to unlock
)
{
	IMFAttributes *pAttributes = NULL;
	UINT32 nValue = 0;
	HRESULT hr = S_OK;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMFT->GetAttributes(&pAttributes));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pAttributes->GetUINT32(MF_TRANSFORM_ASYNC, &nValue));
	if (nValue == TRUE) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pAttributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE));
	}

bail:
	COMPV_MF_SAFE_RELEASE(&pAttributes);
	return hr;
}
//-------------------------------------------------------------------
// CreatePCMAudioType
//
// Creates a media type that describes an uncompressed PCM audio
// format.
//-------------------------------------------------------------------

HRESULT CompVMFUtils::createPCMAudioType(
	UINT32 sampleRate,        // Samples per second
	UINT32 bitsPerSample,     // Bits per sample
	UINT32 cChannels,         // Number of channels
	IMFMediaType **ppType     // Receives a pointer to the media type.
)
{
	HRESULT hr = S_OK;

	IMFMediaType *pType = NULL;

	// Calculate derived values.
	UINT32 blockAlign = cChannels * (bitsPerSample / 8);
	UINT32 bytesPerSecond = blockAlign * sampleRate;

	// Create the empty media type.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMediaType(&pType));

	// Set attributes on the type.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, cChannels));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlign));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSecond));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

	*ppType = pType;
	(*ppType)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pType);
	return hr;
}


//-------------------------------------------------------------------
// CreateVideoType
//
// Creates a media type that describes a video subtype
// format.
//-------------------------------------------------------------------
HRESULT CompVMFUtils::createVideoType(
	const GUID* subType, // video subType
	IMFMediaType **ppType,     // Receives a pointer to the media type.
	UINT32 unWidth, // Video width (0 to ignore)
	UINT32 unHeight // Video height (0 to ignore)
)
{
	HRESULT hr = S_OK;

	IMFMediaType *pType = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMediaType(&pType));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetGUID(MF_MT_SUBTYPE, *subType));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE)); // UnCompressed
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE)); // UnCompressed
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
	if (unWidth > 0 && unHeight > 0) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, unWidth, unHeight));
	}
	*ppType = pType;
	(*ppType)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pType);
	return hr;
}

//-------------------------------------------------------------------
// Name: ValidateVideoFormat
// Description: Validates a media type for this sink.
//-------------------------------------------------------------------
HRESULT CompVMFUtils::validateVideoFormat(IMFMediaType *pmt)
{
	GUID major_type = GUID_NULL;
	GUID subtype = GUID_NULL;
	MFVideoInterlaceMode interlace = MFVideoInterlace_Unknown;
	UINT32 val = 0;
	BOOL bFoundMatchingSubtype = FALSE;

	HRESULT hr = S_OK;

	// Major type must be video.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &major_type));

	if (major_type != MFMediaType_Video) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MF_E_INVALIDMEDIATYPE);
	}

	// Subtype must be one of the subtypes in our global list.

	// Get the subtype GUID.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));

#if 0
	// Look for the subtype in our list of accepted types.
	for (DWORD i = 0; i < g_NumVideoSubtypes; i++) {
		if (subtype == *g_VideoSubtypes[i]) {
			bFoundMatchingSubtype = TRUE;
			break;
		}
	}
	if (!bFoundMatchingSubtype) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MF_E_INVALIDMEDIATYPE);
	}
#endif

	// Video must be progressive frames.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pmt->GetUINT32(MF_MT_INTERLACE_MODE, (UINT32*)&interlace));
	if (interlace != MFVideoInterlace_Progressive) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MF_E_INVALIDMEDIATYPE);
	}

bail:
	return hr;
}

HRESULT CompVMFUtils::convertVideoTypeToUncompressedType(
	IMFMediaType *pType,    // Pointer to an encoded video type.
	const GUID& subtype,    // Uncompressed subtype (eg, RGB-32, AYUV)
	IMFMediaType **ppType   // Receives a matching uncompressed video type.
)
{
	IMFMediaType *pTypeUncomp = NULL;

	HRESULT hr = S_OK;
	GUID majortype = { 0 };
	MFRatio par = { 0 };

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->GetMajorType(&majortype));
	COMPV_CHECK_HRESULT_EXP_BAIL(majortype != MFMediaType_Video, hr = MF_E_INVALIDMEDIATYPE);

	// Create a new media type and copy over all of the items.
	// This ensures that extended color information is retained.
	
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMediaType(&pTypeUncomp));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pType->CopyAllItems(pTypeUncomp));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTypeUncomp->SetGUID(MF_MT_SUBTYPE, subtype));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTypeUncomp->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

	// Fix up PAR if not set on the original type.
	if (SUCCEEDED(hr)) {
		hr = MFGetAttributeRatio(
			pTypeUncomp,
			MF_MT_PIXEL_ASPECT_RATIO,
			(UINT32*)&par.Numerator,
			(UINT32*)&par.Denominator
		);

		// Default to square pixels.
		if (FAILED(hr)) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFSetAttributeRatio(
				pTypeUncomp,
				MF_MT_PIXEL_ASPECT_RATIO,
				1, 1
			));
		}
	}
	
	*ppType = pTypeUncomp;
	(*ppType)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pTypeUncomp);
	return hr;
}

HRESULT CompVMFUtils::createMediaSample(
	DWORD cbData, // Maximum buffer size
	IMFSample **ppSample // Receives the sample
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppSample, E_INVALIDARG);

	HRESULT hr = S_OK;

	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateSample(&pSample));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateMemoryBuffer(cbData, &pBuffer));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSample->AddBuffer(pBuffer));

	*ppSample = pSample;
	(*ppSample)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pSample);
	COMPV_MF_SAFE_RELEASE(&pBuffer);
	return hr;
}

// Check whether video processor (http://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx) is supported
HRESULT CompVMFUtils::isVideoProcessorSupported(__out BOOL* pbSupported)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pbSupported, E_INVALIDARG);
	IMFTransform *pTransform = NULL;
	*pbSupported = SUCCEEDED(CoCreateInstance(CLSID_VideoProcessorMFT, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTransform)));
	COMPV_MF_SAFE_RELEASE(&pTransform);
	return S_OK;
}

HRESULT CompVMFUtils::bestVideoProcessor(
	const GUID& inputFormat, // The input MediaFormat (e.g. MFVideoFormat_I420)
	const GUID& outputFormat, // The output MediaFormat (e.g. MFVideoFormat_NV12)
	IMFTransform **ppProcessor // Receives the video processor
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppProcessor, E_INVALIDARG);

	*ppProcessor = NULL;

	HRESULT hr = S_OK;
	UINT32 count = 0;

	IMFActivate **ppActivate = NULL;

	MFT_REGISTER_TYPE_INFO infoInput = { MFMediaType_Video, inputFormat };
	MFT_REGISTER_TYPE_INFO infoOutput = { MFMediaType_Video, outputFormat };

	UINT32 unFlags = MFT_ENUM_FLAG_HARDWARE |
		MFT_ENUM_FLAG_SYNCMFT |
		MFT_ENUM_FLAG_LOCALMFT |
		MFT_ENUM_FLAG_SORTANDFILTER;

	hr = MFTEnumEx(
		MFT_CATEGORY_VIDEO_PROCESSOR,
		unFlags,
		&infoInput,      // Input type
		&infoOutput,       // Output type
		&ppActivate,
		&count
	);

	for (UINT32 i = 0; i < count; ++i) {
		hr = ppActivate[i]->ActivateObject(IID_PPV_ARGS(ppProcessor));
		if (SUCCEEDED(hr) && *ppProcessor) {
			break;
		}
		COMPV_MF_SAFE_RELEASE(ppProcessor);
	}

	for (UINT32 i = 0; i < count; i++) {
		ppActivate[i]->Release();
	}
	CoTaskMemFree(ppActivate);

	return *ppProcessor ? S_OK : MF_E_NOT_FOUND;
}

// Add an transform node to a topology.
HRESULT CompVMFUtils::addTransformNode(
	IMFTopology *pTopology,     // Topology.
	IMFTransform *pMFT,     // MFT.
	DWORD dwId,                 // Identifier of the stream sink.
	IMFTopologyNode **ppNode   // Receives the node pointer.
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppNode, E_INVALIDARG);

	*ppNode = NULL;

	IMFTopologyNode *pNode = NULL;
	HRESULT hr = S_OK;

	// Create the node.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pNode));
	// Set the object pointer.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetObject(pMFT));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
	// Add the node to the topology.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopology->AddNode(pNode));

	// Return the pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pNode);
	return hr;
}

// Sets the IMFStreamSink pointer on an output node.
HRESULT CompVMFUtils::bindOutputNode(
	IMFTopologyNode *pNode // The Node
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pNode, E_INVALIDARG);

	HRESULT hr = S_OK;
	IUnknown *pNodeObject = NULL;
	IMFActivate *pActivate = NULL;
	IMFStreamSink *pStream = NULL;
	IMFMediaSink *pSink = NULL;

	// Get the node's object pointer.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->GetObject(&pNodeObject));

	// The object pointer should be one of the following:
	// 1. An activation object for the media sink.
	// 2. The stream sink.

	// If it's #2, then we're already done.

	// First, check if it's an activation object.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeObject->QueryInterface(IID_PPV_ARGS(&pActivate)));

	if (SUCCEEDED(hr)) {
		DWORD dwStreamID = 0;

		// The object pointer is an activation object.

		// Try to create the media sink.
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pActivate->ActivateObject(IID_PPV_ARGS(&pSink)));

		// Look up the stream ID. (Default to zero.)
		
		dwStreamID = MFGetAttributeUINT32(pNode, MF_TOPONODE_STREAMID, 0);
		

		// Now try to get or create the stream sink.

		// Check if the media sink already has a stream sink with the requested ID.
		
		hr = pSink->GetStreamSinkById(dwStreamID, &pStream);
		if (FAILED(hr)) {
			// Try to add a new stream sink.
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSink->AddStreamSink(dwStreamID, NULL, &pStream));
		}

		// Replace the node's object pointer with the stream sink.
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetObject(pStream));
	}
	else {
		// Not an activation object. Is it a stream sink?
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeObject->QueryInterface(IID_PPV_ARGS(&pStream)));
	}

bail:
	COMPV_MF_SAFE_RELEASE(&pNodeObject);
	COMPV_MF_SAFE_RELEASE(&pActivate);
	COMPV_MF_SAFE_RELEASE(&pStream);
	COMPV_MF_SAFE_RELEASE(&pSink);
	return hr;
}

// Add an output node to a topology.
HRESULT CompVMFUtils::addOutputNode(
	IMFTopology *pTopology,     // Topology.
	IMFActivate *pActivate,     // Media sink activation object.
	DWORD dwId,                 // Identifier of the stream sink.
	IMFTopologyNode **ppNode)   // Receives the node pointer
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppNode, E_INVALIDARG);
	IMFTopologyNode *pNode = NULL;

	HRESULT hr = S_OK;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetObject(pActivate));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopology->AddNode(pNode));

	// Return the pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pNode);
	return hr;
}

// Add a source node to a topology
HRESULT CompVMFUtils::addSourceNode(
	IMFTopology *pTopology,           // Topology.
	IMFMediaSource *pSource,          // Media source.
	IMFPresentationDescriptor *pPD,   // Presentation descriptor.
	IMFStreamDescriptor *pSD,         // Stream descriptor.
	IMFTopologyNode **ppNode          // Receives the node pointer.
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppNode, E_INVALIDARG);
	IMFTopologyNode *pNode = NULL;

	HRESULT hr = S_OK;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopology->AddNode(pNode));

	// Return the pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pNode);
	return hr;
}

// Create the topology
//
// [source] -> (Transform) -> [SinkMain]
//         \-> (SinkPreview)
//
HRESULT CompVMFUtils::createTopology(
	IMFMediaSource *pSource, // Media source
	IMFTransform *pTransform, // Transform filter (e.g. encoder or decoder) to insert between the source and Sink. NULL is valid.
	IMFActivate *pSinkActivateMain, // Main sink (e.g. sample grabber or EVR).
	IMFActivate *pSinkActivatePreview, // Preview sink. Optional. Could be NULL.
	IMFMediaType *pIputTypeMain, // Main sink input MediaType
	IMFTopology **ppTopo // Receives the newly created topology
)
{
	IMFTopology *pTopology = NULL;
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFTopologyNode *pNodeSource = NULL;
	IMFTopologyNode *pNodeSinkMain = NULL;
	IMFTopologyNode *pNodeSinkPreview = NULL;
	IMFTopologyNode *pNodeTransform = NULL;
	IMFTopologyNode *pNodeTee = NULL;
	IMFMediaType *pMediaType = NULL;
	IMFTransform *pVideoProcessor = NULL;
	IMFTopologyNode *pNodeVideoProcessor = NULL;
	IMFTransform *pConvFrameRate = NULL;
	IMFTransform *pConvSize = NULL;
	IMFTransform *pConvColor = NULL;
	IMFTopologyNode *pNodeConvFrameRate = NULL;
	IMFTopologyNode *pNodeConvSize = NULL;
	IMFTopologyNode *pNodeConvColor = NULL;
	IMFMediaType *pTransformInputType = NULL;
	IMFMediaType *pSinkMainInputType = NULL;
	const IMFTopologyNode *pcNodeBeforeSinkMain = NULL;

	HRESULT hr = S_OK;
	DWORD cStreams = 0;
	BOOL bSourceFound = FALSE;
	BOOL bSupportedSize = FALSE;
	BOOL bSupportedFps = FALSE;
	BOOL bSupportedFormat = FALSE;
	BOOL bVideoProcessorSupported = FALSE;
	GUID inputMajorType, inputSubType;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = isVideoProcessorSupported(&bVideoProcessorSupported));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pIputTypeMain->GetMajorType(&inputMajorType));

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopology(&pTopology));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSource->CreatePresentationDescriptor(&pPD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorCount(&cStreams));

	for (DWORD i = 0; i < cStreams; i++) {
		BOOL fSelected = FALSE;
		GUID majorType;

		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSD->GetMediaTypeHandler(&pHandler));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMajorType(&majorType));

		if (majorType == inputMajorType && fSelected) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addSourceNode(pTopology, pSource, pPD, pSD, &pNodeSource));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeSource->SetTopoNodeID(CompVMFUtils::s_ullTopoIdSource));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addOutputNode(pTopology, pSinkActivateMain, 0, &pNodeSinkMain));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeSinkMain->SetTopoNodeID(CompVMFUtils::s_ullTopoIdSinkMain));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::bindOutputNode(pNodeSinkMain)); // To avoid MF_E_TOPO_SINK_ACTIVATES_UNSUPPORTED

			/* Create preview */
			if (pSinkActivatePreview) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addOutputNode(pTopology, pSinkActivatePreview, 0, &pNodeSinkPreview));
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeSinkPreview->SetTopoNodeID(CompVMFUtils::s_ullTopoIdSinkPreview));
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::bindOutputNode(pNodeSinkPreview));

				COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pNodeTee));
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopology->AddNode(pNodeTee));
			}
			
			/* Create converters */
			if (majorType == MFMediaType_Video) {
				// Even when size matches the topology could add a resizer which doesn't keep ratio when resizing while video processor does.
				if (!bVideoProcessorSupported) {
					hr = CompVMFUtils::isSupported(
						pPD,
						i,
						pIputTypeMain,
						&bSupportedSize,
						&bSupportedFps,
						&bSupportedFormat);
				}

				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pIputTypeMain->GetGUID(MF_MT_SUBTYPE, &inputSubType));

				if (!bSupportedSize || !bSupportedFps || !bSupportedFormat) {
					// Use video processor single MFT or 3 different MFTs
					if (!pVideoProcessor) {
						hr = CoCreateInstance(CLSID_VideoProcessorMFT, NULL,
							CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pVideoProcessor));
					}
					if (!pVideoProcessor) {
						// Video Resizer DSP(http://msdn.microsoft.com/en-us/library/windows/desktop/ff819491(v=vs.85).aspx) supports I420 only
						if (!bSupportedSize && !pConvSize && inputSubType == MFVideoFormat_I420) {
							hr = CoCreateInstance(CLSID_CResizerDMO, NULL,
								CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pConvSize));
						}
#if 0
						// Frame Rate Converter DSP(http://msdn.microsoft.com/en-us/library/windows/desktop/ff819100(v=vs.85).aspx) supports neither NV12 nor I420
						if (!bSupportedFps && !pConvFrameRate) {
							hr = CoCreateInstance(CLSID_CFrameRateConvertDmo, NULL,
							CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pConvFrameRate));
						}
#endif
						// Color Converter DSP (http://msdn.microsoft.com/en-us/library/windows/desktop/ff819079(v=vs.85).aspx) supports both NV12 and I420
						if (!bSupportedFormat && !pConvColor) {
							hr = CoCreateInstance(CLSID_CColorConvertDMO, NULL,
								CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pConvColor));
						}
					}
				}
				else {
					// MediaType supported
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->SetCurrentMediaType(pIputTypeMain));
				}

				if (pVideoProcessor && !pNodeVideoProcessor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addTransformNode(pTopology, pVideoProcessor, 0, &pNodeVideoProcessor));
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeVideoProcessor->SetTopoNodeID(CompVMFUtils::s_ullTopoIdVideoProcessor));
				}
				if (pConvColor && !pNodeConvColor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addTransformNode(pTopology, pConvColor, 0, &pNodeConvColor));
				}
				if (pConvFrameRate && !pNodeConvFrameRate) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addTransformNode(pTopology, pConvFrameRate, 0, &pNodeConvFrameRate));
				}
				if (pConvSize && !pNodeConvSize) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addTransformNode(pTopology, pConvSize, 0, &pNodeConvSize));
				}
			} // if (majorType == MFMediaType_Video)

			/* Set media type */
			if (pTransform) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addTransformNode(pTopology, pTransform, 0, &pNodeTransform));
				hr = pTransform->GetInputCurrentType(0, &pTransformInputType);
				if (FAILED(hr)) {
					pTransformInputType = pIputTypeMain;
					pTransformInputType->AddRef();
					hr = S_OK;
				}
				if (pVideoProcessor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pVideoProcessor->SetOutputType(0, pTransformInputType, 0));
				}
				else {
					if (pConvColor) {
						/*CHECK_HR*/(hr = pConvColor->SetOutputType(0, pTransformInputType, 0));
					}
					if (pConvFrameRate) {
						/*CHECK_HR*/(hr = pConvFrameRate->SetOutputType(0, pTransformInputType, 0));
					}
#if 0
					if (pConvSize) {
						// Transform requires NV12
						//Video Resizer DSP(http://msdn.microsoft.com/en-us/library/windows/desktop/ff819491(v=vs.85).aspx) doesn't support NV12
						//*CHECK_HR*/(hr = pConvSize->SetOutputType(0, pTransformInputType, 0));
					}
#endif
				}
			}
			else {
				hr = pNodeSinkMain->GetInputPrefType(0, &pSinkMainInputType);
				if (FAILED(hr)) {
					pSinkMainInputType = pIputTypeMain;
					pSinkMainInputType->AddRef();
					hr = S_OK;
				}
				if (SUCCEEDED(hr)) {
					if (pVideoProcessor) {
						COMPV_CHECK_HRESULT_CODE_BAIL(hr = pVideoProcessor->SetOutputType(0, pSinkMainInputType, 0));
					}
					else {
#if 0
						//!\ MUST NOT SET OUTPUT TYPE
						if (pConvColor) {
							/*CHECK_HR*/(hr = pConvColor->SetOutputType(0, pSinkMainInputType, 0));
						}
						if (pConvFrameRate) {
							/*CHECK_HR*/(hr = pConvFrameRate->SetOutputType(0, pSinkMainInputType, 0));
						}
						if (pConvSize) {
							/*CHECK_HR*/(hr = pConvSize->SetOutputType(0, pSinkMainInputType, 0));
						}
#endif
					}
				}
			} // if (pTransform)

			/* Connect */
			if (pNodeTee) {
				// Connect(Source -> Tee)
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeSource->ConnectOutput(0, pNodeTee, 0));

				// Connect(Tee -> SinkPreview)
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeTee->ConnectOutput(1, pNodeSinkPreview, 0));

				// Connect(Tee ->(Processors)
				if (pVideoProcessor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeTee->ConnectOutput(0, pNodeVideoProcessor, 0));
					pcNodeBeforeSinkMain = pNodeVideoProcessor;
				}
				else if (pNodeConvFrameRate || pNodeConvSize || pNodeConvColor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::connectConverters(
						pNodeTee,
						0,
						pNodeConvFrameRate,
						pNodeConvColor,
						pNodeConvSize
					));
					pcNodeBeforeSinkMain = pNodeConvSize ? pNodeConvSize : (pNodeConvColor ? pNodeConvColor : pNodeConvFrameRate);
				}
				else {
					pcNodeBeforeSinkMain = pNodeTee;
				}
			}
			else {
				// Connect(Source -> (Processors))
				if (pVideoProcessor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeSource->ConnectOutput(0, pNodeVideoProcessor, 0));
					pcNodeBeforeSinkMain = pNodeVideoProcessor;
				}
				else if (pNodeConvFrameRate || pNodeConvFrameRate || pNodeConvColor) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::connectConverters(
						pNodeSource,
						0,
						pNodeConvFrameRate,
						pNodeConvSize,
						pNodeConvColor
					));
					pcNodeBeforeSinkMain = pNodeConvSize ? pNodeConvSize : (pNodeConvColor ? pNodeConvColor : pNodeConvFrameRate);
				}
				else {
					pcNodeBeforeSinkMain = pNodeSource;
				}
			}


			if (pNodeTransform) {
				// Connect(X->Transform)
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = ((IMFTopologyNode *)pcNodeBeforeSinkMain)->ConnectOutput(0, pNodeTransform, 0));
				pcNodeBeforeSinkMain = pNodeTransform;
			}

			// Connect(X -> SinkMain)
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = ((IMFTopologyNode *)pcNodeBeforeSinkMain)->ConnectOutput(0, pNodeSinkMain, 0));

			bSourceFound = TRUE;
			break;
		}
		else {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->DeselectStream(i));
		}
		COMPV_MF_SAFE_RELEASE(&pSD);
		COMPV_MF_SAFE_RELEASE(&pHandler);
	}

	*ppTopo = pTopology;
	(*ppTopo)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pTopology);
	COMPV_MF_SAFE_RELEASE(&pNodeSource);
	COMPV_MF_SAFE_RELEASE(&pNodeSinkMain);
	COMPV_MF_SAFE_RELEASE(&pNodeSinkPreview);
	COMPV_MF_SAFE_RELEASE(&pNodeTransform);
	COMPV_MF_SAFE_RELEASE(&pNodeTee);
	COMPV_MF_SAFE_RELEASE(&pPD);
	COMPV_MF_SAFE_RELEASE(&pSD);
	COMPV_MF_SAFE_RELEASE(&pHandler);
	COMPV_MF_SAFE_RELEASE(&pMediaType);
	COMPV_MF_SAFE_RELEASE(&pTransformInputType);
	COMPV_MF_SAFE_RELEASE(&pSinkMainInputType);

	COMPV_MF_SAFE_RELEASE(&pVideoProcessor);
	COMPV_MF_SAFE_RELEASE(&pNodeVideoProcessor);
	COMPV_MF_SAFE_RELEASE(&pConvFrameRate);
	COMPV_MF_SAFE_RELEASE(&pConvSize);
	COMPV_MF_SAFE_RELEASE(&pConvColor);
	COMPV_MF_SAFE_RELEASE(&pNodeConvFrameRate);
	COMPV_MF_SAFE_RELEASE(&pNodeConvSize);
	COMPV_MF_SAFE_RELEASE(&pNodeConvColor);

	// Check if we found the source
	COMPV_CHECK_HRESULT_EXP_RETURN(!bSourceFound, (hr = E_NOT_SET));

	return hr;
}

// Creates a fully loaded topology from the input partial topology.
HRESULT CompVMFUtils::resolveTopology(
	IMFTopology *pInputTopo, // A pointer to the IMFTopology interface of the partial topology to be resolved.
	IMFTopology **ppOutputTopo, // Receives a pointer to the IMFTopology interface of the completed topology. The caller must release the interface.
	IMFTopology *pCurrentTopo /*= NULL*/ // A pointer to the IMFTopology interface of the previous full topology. The topology loader can re-use objects from this topology in the new topology. This parameter can be NULL.
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppOutputTopo || !pInputTopo, E_INVALIDARG);

	HRESULT hr = S_OK;
	IMFTopoLoader* pTopoLoader = NULL;

	*ppOutputTopo = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateTopoLoader(&pTopoLoader));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTopoLoader->Load(pInputTopo, ppOutputTopo, pCurrentTopo));

bail:
	COMPV_MF_SAFE_RELEASE(&pTopoLoader);
	return hr;
}

HRESULT CompVMFUtils::findNodeObject(
	IMFTopology *pInputTopo, // The Topology containing the node to find
	TOPOID qwTopoNodeID, //The identifier for the node
	void** ppObject // Receives the Object
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pInputTopo || !ppObject, E_INVALIDARG);

	*ppObject = NULL;

	IMFTopologyNode *pNode = NULL;
	HRESULT hr = S_OK;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pInputTopo->GetNodeByID(qwTopoNodeID, &pNode));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->GetObject((IUnknown**)ppObject));

bail:
	COMPV_MF_SAFE_RELEASE(&pNode);
	return hr;
}

//  Create an activation object for a renderer, based on the stream media type.
HRESULT CompVMFUtils::createMediaSinkActivate(
	IMFStreamDescriptor *pSourceSD,     // Pointer to the stream descriptor.
	HWND hVideoWindow,                  // Handle to the video clipping window.
	IMFActivate **ppActivate
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppActivate, E_INVALIDARG);

	HRESULT hr = S_OK;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFActivate *pActivate = NULL;

	// Get the media type handler for the stream.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
	// Get the major media type.
	GUID guidMajorType;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMajorType(&guidMajorType));

	// Create an IMFActivate object for the renderer, based on the media type.
	if (MFMediaType_Audio == guidMajorType) {
		// Create the audio renderer.
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateAudioRendererActivate(&pActivate));
	}
	else if (MFMediaType_Video == guidMajorType) {
		// Create the video renderer.
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateVideoRendererActivate(hVideoWindow, &pActivate));
	}
	else {
		// Unknown stream type.
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = E_FAIL);
		// Optionally, you could deselect this stream instead of failing.
	}

	// Return IMFActivate pointer to caller.
	*ppActivate = pActivate;
	(*ppActivate)->AddRef();

bail:
	COMPV_MF_SAFE_RELEASE(&pHandler);
	COMPV_MF_SAFE_RELEASE(&pActivate);
	return hr;
}

// Set source output media type
HRESULT CompVMFUtils::setMediaType(
	IMFMediaSource *pSource,        // Media source.
	IMFMediaType* pMediaType // Media Type.
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pSource || !pMediaType, E_INVALIDARG);

	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;

	HRESULT hr = S_OK;
	DWORD cStreams = 0;
	GUID inputMajorType;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSource->CreatePresentationDescriptor(&pPD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorCount(&cStreams));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaType->GetMajorType(&inputMajorType));

	for (DWORD i = 0; i < cStreams; i++) {
		BOOL fSelected = FALSE;
		GUID majorType;

		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSD->GetMediaTypeHandler(&pHandler));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMajorType(&majorType));

		if (majorType == inputMajorType && fSelected) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->SetCurrentMediaType(pMediaType));
		}
		else {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->DeselectStream(i));
		}
		COMPV_MF_SAFE_RELEASE(&pSD);
		COMPV_MF_SAFE_RELEASE(&pHandler);
	}


bail:
	COMPV_MF_SAFE_RELEASE(&pPD);
	COMPV_MF_SAFE_RELEASE(&pSD);
	COMPV_MF_SAFE_RELEASE(&pHandler);

	return hr;
}

HRESULT CompVMFUtils::setVideoWindow(
	IMFTopology *pTopology,         // Topology.
	IMFMediaSource *pSource,        // Media source.
	HWND hVideoWnd                 // Window for video playback.
)
{
	HRESULT hr = S_OK;
	IMFStreamDescriptor *pSD = NULL;
	IMFPresentationDescriptor *pPD = NULL;
	IMFActivate         *pSinkActivate = NULL;
	IMFTopologyNode     *pSourceNode = NULL;
	IMFTopologyNode     *pOutputNode = NULL;
	DWORD cStreams = 0, iStream;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSource->CreatePresentationDescriptor(&pPD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorCount(&cStreams));

	for (iStream = 0; iStream < cStreams; ++iStream) {
		BOOL fSelected = FALSE;

		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD));

		if (fSelected) {
			// Create the media sink activation object.
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::createMediaSinkActivate(pSD, hVideoWnd, &pSinkActivate));
			// Add a source node for this stream.
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode));
			// Create the output node for the renderer.
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::addOutputNode(pTopology, pSinkActivate, 0, &pOutputNode));
			// Connect the source node to the output node.
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
		}
		// else: If not selected, don't add the branch.
	}

bail:
	COMPV_MF_SAFE_RELEASE(&pPD);
	COMPV_MF_SAFE_RELEASE(&pSD);
	COMPV_MF_SAFE_RELEASE(&pSinkActivate);
	COMPV_MF_SAFE_RELEASE(&pSourceNode);
	COMPV_MF_SAFE_RELEASE(&pOutputNode);
	return hr;
}

// Run the session
HRESULT CompVMFUtils::runSession(
	IMFMediaSession *pSession, // Session to run
	IMFTopology *pTopology // The toppology
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pSession || !pTopology, E_INVALIDARG);

	IMFMediaEvent *pEvent = NULL;

	PROPVARIANT var;
	PropVariantInit(&var);

	MediaEventType met;
	HRESULT hrStatus = S_OK;
	HRESULT hr = S_OK;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, pTopology)); // MFSESSION_SETTOPOLOGY_IMMEDIATE required to update (reload) topology when media type change
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSession->Start(&GUID_NULL, &var));

	// Check first event
	hr = pSession->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent);
	if (hr == MF_E_NO_EVENTS_AVAILABLE || hr == MF_E_MULTIPLE_SUBSCRIBERS) { // MF_E_MULTIPLE_SUBSCRIBERS means already listening
		hr = S_OK;
		goto bail;
	}
	if (pEvent) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pEvent->GetStatus(&hrStatus));
	}
	else {
		hrStatus = hr;
	}
	if (FAILED(hrStatus)) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pEvent->GetType(&met));
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Session error: 0x%x (event id: %d)", hrStatus, met);
		hr = hrStatus;
		goto bail;
	}

bail:
	COMPV_MF_SAFE_RELEASE(&pEvent);
	return hr;
}

// Shutdown session
HRESULT CompVMFUtils::shutdownSession(
	IMFMediaSession *pSession, // The Session
	IMFMediaSource *pSource // Source to shutdown (optional)
)
{
	// MUST be source then session
	if (pSource) {
		pSource->Stop();
		pSource->Shutdown();
	}
	if (pSession) {
		pSession->Stop();
		pSession->Shutdown();
	}
	return S_OK;
}

HRESULT CompVMFUtils::stopSession(
	IMFMediaSession *pSession, // The Session
	IMFMediaSource *pSource COMPV_DEFAULT(NULL) // Source to shutdown (optional)
)
{
	// MUST be source then session
	if (pSource) {
		pSource->Stop();
	}
	if (pSession) {
		pSession->Stop();
	}
	return S_OK;
}

// Pause session
HRESULT CompVMFUtils::pauseSession(
	IMFMediaSession *pSession, // The session
	IMFMediaSource *pSource // Source to pause (optional)
)
{
	if (!pSession) {
		return E_INVALIDARG;
	}
	if (pSource) {
		pSource->Pause();
	}
	return pSession->Pause();
}

HRESULT CompVMFUtils::supportedCaps(
	__in IMFMediaSource *pSource, // The source
	__out std::vector<CompVMFCameraCaps>& caps,
	__in const GUID& majorType_ COMPV_DEFAULT(MFMediaType_Video) // The MediaType
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pSource, E_INVALIDARG);

	std::vector<CompVMFCameraCaps> caps_;
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFMediaType *pMediaType = NULL;
	
	HRESULT hr = S_OK;
	DWORD cStreams = 0, cMediaTypesCount;
	GUID majorType, subType;
	BOOL fSelected;
	UINT32 width, height, numeratorFps, denominatorFps;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSource->CreatePresentationDescriptor(&pPD));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorCount(&cStreams));
	
	for (DWORD cStreamIndex = 0; cStreamIndex < cStreams; ++cStreamIndex) {
		fSelected = FALSE;
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorByIndex(cStreamIndex, &fSelected, &pSD));
		if (fSelected) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSD->GetMediaTypeHandler(&pHandler));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMajorType(&majorType));
			if (majorType == majorType_) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMediaTypeCount(&cMediaTypesCount));
				for (DWORD cMediaTypesIndex = 0; cMediaTypesIndex < cMediaTypesCount; ++cMediaTypesIndex) {
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMediaTypeByIndex(cMediaTypesIndex, &pMediaType));
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType));
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &width, &height));
					COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &numeratorFps, &denominatorFps));
					caps_.push_back(CompVMFCameraCaps(
						static_cast<LONG>(width),
						static_cast<LONG>(height),
						static_cast<int>(numeratorFps / denominatorFps),
						subType)
					);
					COMPV_MF_SAFE_RELEASE(&pMediaType);
				}
			}
		}

		COMPV_MF_SAFE_RELEASE(&pSD);
		COMPV_MF_SAFE_RELEASE(&pHandler);
	}

	caps = caps_;

bail:
	COMPV_MF_SAFE_RELEASE(&pMediaType);
	COMPV_MF_SAFE_RELEASE(&pPD);
	COMPV_MF_SAFE_RELEASE(&pSD);
	COMPV_MF_SAFE_RELEASE(&pHandler);

	return hr;
}

HRESULT CompVMFUtils::isSupported(
	IMFPresentationDescriptor *pPD,
	DWORD cStreamIndex,
	UINT32 nWidth,
	UINT32 nHeight,
	UINT32 nFps,
	const GUID& guidFormat,
	BOOL* pbSupportedSize,
	BOOL* pbSupportedFps,
	BOOL* pbSupportedFormat
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pPD || !pbSupportedSize || !pbSupportedFps || !pbSupportedFormat, E_INVALIDARG);

	HRESULT hr = S_OK;

	BOOL fSelected = FALSE;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFMediaType *pMediaType = NULL;
	UINT32 _nWidth = 0, _nHeight = 0, numeratorFps = 0, denominatorFps = 0;
	GUID subType;
	DWORD cMediaTypesCount;

	*pbSupportedSize = FALSE;
	*pbSupportedFps = FALSE;
	*pbSupportedFormat = FALSE;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pPD->GetStreamDescriptorByIndex(cStreamIndex, &fSelected, &pSD));
	if (fSelected) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pSD->GetMediaTypeHandler(&pHandler));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMediaTypeCount(&cMediaTypesCount));
		for (DWORD cMediaTypesIndex = 0; cMediaTypesIndex < cMediaTypesCount; ++cMediaTypesIndex) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetMediaTypeByIndex(cMediaTypesIndex, &pMediaType));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &_nWidth, &_nHeight));
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType));
			if (FAILED(hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &numeratorFps, &denominatorFps))) {
				numeratorFps = 30;
				denominatorFps = 1;
			}

			// all must match for the same stream
			if (_nWidth == nWidth && _nHeight == nHeight && subType == guidFormat && (numeratorFps / denominatorFps) == nFps) {
				*pbSupportedSize = TRUE;
				*pbSupportedFormat = TRUE;
				*pbSupportedFps = TRUE;
				break;
			}

			COMPV_MF_SAFE_RELEASE(&pMediaType);
		}
		COMPV_MF_SAFE_RELEASE(&pHandler);
	}

bail:
	COMPV_MF_SAFE_RELEASE(&pSD);
	COMPV_MF_SAFE_RELEASE(&pHandler);
	COMPV_MF_SAFE_RELEASE(&pMediaType);

	return hr;
}

HRESULT CompVMFUtils::isSupported(
	IMFPresentationDescriptor *pPD,
	DWORD cStreamIndex,
	IMFMediaType* pMediaType,
	BOOL* pbSupportedSize,
	BOOL* pbSupportedFps,
	BOOL* pbSupportedFormat
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pPD || !pMediaType || !pbSupportedSize || !pbSupportedFps || !pbSupportedFormat, E_INVALIDARG);

	HRESULT hr = S_OK;

	UINT32 nWidth = 0, nHeight = 0, nFps = 0, numeratorFps = 30, denominatorFps = 1;
	GUID subType;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &nWidth, &nHeight));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType));
	if (FAILED(hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &numeratorFps, &denominatorFps))) {
		numeratorFps = 30;
		denominatorFps = 1;
	}

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isSupported(
		pPD,
		cStreamIndex,
		nWidth,
		nHeight,
		(numeratorFps / denominatorFps),
		subType,
		pbSupportedSize,
		pbSupportedFps,
		pbSupportedFormat
	));
bail:
	return hr;
}

HRESULT CompVMFUtils::isSupportedByInput(
	IMFPresentationDescriptor *pPD,
	DWORD cStreamIndex,
	IMFTopologyNode *pNode,
	BOOL* pbSupportedSize,
	BOOL* pbSupportedFps,
	BOOL* pbSupportedFormat
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pPD || !pNode || !pbSupportedSize || !pbSupportedFps || !pbSupportedFormat, E_INVALIDARG);

	HRESULT hr = S_OK;

	IMFMediaType *pMediaType = NULL;
	IUnknown* pObject = NULL;
	IMFActivate *pActivate = NULL;
	IMFMediaSink *pMediaSink = NULL;
	IMFTransform *pTransform = NULL;
	IMFStreamSink *pStreamSink = NULL;
	IMFMediaTypeHandler *pHandler = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->GetObject(&pObject));
	hr = pObject->QueryInterface(IID_PPV_ARGS(&pActivate));
	if (SUCCEEDED(hr)) {
		COMPV_MF_SAFE_RELEASE(&pObject);
		hr = pActivate->ActivateObject(IID_IMFMediaSink, (void**)&pObject);
		if (FAILED(hr)) {
			hr = pActivate->ActivateObject(IID_IMFTransform, (void**)&pObject);
		}
	}

	if (!pObject) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = E_NOINTERFACE);
	}

	hr = pObject->QueryInterface(IID_PPV_ARGS(&pMediaSink));
	if (FAILED(hr)) {
		hr = pObject->QueryInterface(IID_PPV_ARGS(&pTransform));
	}
	
	if (pMediaSink) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pMediaSink->GetStreamSinkByIndex(0, &pStreamSink));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pStreamSink->GetMediaTypeHandler(&pHandler));
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pHandler->GetCurrentMediaType(&pMediaType));

	}
	else if (pTransform) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pTransform->GetInputCurrentType(0, &pMediaType));
	}
	else {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->GetInputPrefType(0, &pMediaType));
	}

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVMFUtils::isSupported(
		pPD,
		cStreamIndex,
		pMediaType,
		pbSupportedSize,
		pbSupportedFps,
		pbSupportedFormat
	));

bail:
	COMPV_MF_SAFE_RELEASE(&pObject);
	COMPV_MF_SAFE_RELEASE(&pActivate);
	COMPV_MF_SAFE_RELEASE(&pMediaType);
	COMPV_MF_SAFE_RELEASE(&pStreamSink);
	COMPV_MF_SAFE_RELEASE(&pHandler);
	return hr;
}

HRESULT CompVMFUtils::connectConverters(
	IMFTopologyNode *pNode,
	DWORD dwOutputIndex,
	IMFTopologyNode *pNodeConvFrameRate,
	IMFTopologyNode *pNodeConvColor,
	IMFTopologyNode *pNodeConvSize
)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pNode, E_INVALIDARG);

	HRESULT hr = S_OK;

	if (pNodeConvFrameRate) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->ConnectOutput(dwOutputIndex, pNodeConvFrameRate, 0));
		if (pNodeConvSize) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeConvFrameRate->ConnectOutput(0, pNodeConvSize, 0));
			if (pNodeConvColor) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeConvSize->ConnectOutput(0, pNodeConvColor, 0));
			}
		}
		else {
			if (pNodeConvColor) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeConvFrameRate->ConnectOutput(0, pNodeConvColor, 0));
			}
		}
	}
	else {
		if (pNodeConvSize) {
			COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->ConnectOutput(dwOutputIndex, pNodeConvSize, 0));
			if (pNodeConvColor) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNodeConvSize->ConnectOutput(0, pNodeConvColor, 0));
			}
		}
		else {
			if (pNodeConvColor) {
				COMPV_CHECK_HRESULT_CODE_BAIL(hr = pNode->ConnectOutput(dwOutputIndex, pNodeConvColor, 0));
			}
		}
	}

bail:
	return hr;
}

COMPV_NAMESPACE_END()
