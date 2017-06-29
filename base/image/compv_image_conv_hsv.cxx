/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_hsv.h"
#include "compv/base/image/compv_image_conv_to_rgbx.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_ssse3.h"
#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_avx2.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToHSV"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVImageConvRgb24ToHsv_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvRgb24ToHsv_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb24Ptr, COMPV_ALIGNED(AVX) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X64 */
#endif /* COMPV_ASM */

template <typename xType>
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

//
// CompVImageConvToHSV
//

COMPV_ERROR_CODE CompVImageConvToHSV::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageHSV)
{
	// Internal function, do not check input parameters (already done)

	CompVMatPtr imageRGBx;

	CompVMatPtr imageOut = (*imageHSV == imageIn) ? nullptr : *imageHSV;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_HSV, imageIn->cols(), imageIn->rows(), imageIn->stride()));

	// Convert to RGBA32 or RGB24
	switch (imageIn->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_RGB24:
		imageRGBx = imageIn;
		break;
	default:
		// On X86: "RGBA32 -> HSV" is faster than "RGB24 -> HSV" because of (de-)interleaving RGB24 which is slow
		// On ARM: "RGB24 -> HSV" is faster than "RGBA32 -> HSV" because less data and more cache-friendly. No (de-)interleaving issues, thanks to vld3.u8 and vst3.u8.
		// Another good reason to use RGB24: "input === output" -> Cache-friendly
		// Another good reason to use RGB24: there is very faaast ASM code for NEON, SSSE3 and AVX2
#if COMPV_ARCH_ARM || 1
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::process(imageIn, COMPV_SUBTYPE_PIXELS_RGB24, &imageOut));
		// Call 'newObj8u' to change the subtype, no memory will be allocated as HSV and RGB24 have the same size.
		const void* oldPtr = imageOut->ptr<const void*>();
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, COMPV_SUBTYPE_PIXELS_HSV, imageIn->cols(), imageIn->rows(), imageIn->stride()));
		COMPV_CHECK_EXP_RETURN(oldPtr != imageOut->ptr<const void*>(), COMPV_ERROR_CODE_E_INVALID_CALL, "Data reallocation not expected to change the pointer address");
		imageRGBx = imageOut;
#else
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Using RGB32 as pivot to convert to HSV should not be called on final code");
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::process(imageIn, COMPV_SUBTYPE_PIXELS_RGBA32, &imageRGBx));
#endif
		break;
	}

	//!\\ Important: 'imageRGBx' and 'imageOut' are the same data when intermediate format is RGB24. No data
	// override at conversion because RGB24 and HSV have the same size. Not the case for RGBA32.
	COMPV_CHECK_CODE_RETURN(CompVImageConvToHSV::rgbxToHsv(imageRGBx, imageOut));

	*imageHSV = imageOut;
	return COMPV_ERROR_CODE_S_OK;
}

// RGBx = RGBA32 or RGB24
COMPV_ERROR_CODE CompVImageConvToHSV::rgbxToHsv(const CompVMatPtr& imageRGBx, CompVMatPtr& imageHSV)
{
	// Internal function, do not check input parameters (already done)

	void(*rgbx_to_hsv)(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;

	switch (imageRGBx->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_HSV: // See above, this is a hack use to overwrite the memory (input == output)
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x3_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Asm_X64_SSSE3);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX() && imageHSV->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(rgbx_to_hsv = CompVImageConvRgb24ToHsv_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_RGBA32:
		rgbx_to_hsv = rgbx_to_hsv_C<compv_uint8x4_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE() && imageHSV->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(rgbx_to_hsv = CompVImageConvRgba32ToHsv_Intrin_SSSE3);
		}
#elif COMPV_ARCH_ARM
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> HSV not supported", CompVGetSubtypeString(imageRGBx->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBx->cols();
	const size_t heightInSamples = imageRGBx->rows();
	const size_t strideInSamples = imageRGBx->stride();

	const uint8_t* rgbxPtr = imageRGBx->ptr<const uint8_t>();
	uint8_t* hsvPtr = imageHSV->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(widthInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	// Process
	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount);
		const size_t lastHeight = heights + (heightInSamples % threadsCount);
		const size_t rgbxPaddingInBytes = (imageRGBx->strideInBytes() * heights);
		const size_t hsvPaddingInBytes = (imageHSV->strideInBytes() * heights);
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* rgbxPtr_, uint8_t* hsvPtr_, compv_uscalar_t height_) -> void {
			rgbx_to_hsv(rgbxPtr_, hsvPtr_, widthInSamples, height_, strideInSamples);
		};

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgbxPtr, hsvPtr, heights), taskIds), "Dispatching task failed");
			rgbxPtr += rgbxPaddingInBytes;
			hsvPtr += hsvPaddingInBytes;
		}
		if (lastHeight > 0) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, rgbxPtr, hsvPtr, lastHeight), taskIds), "Dispatching task failed");
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		rgbx_to_hsv(rgbxPtr, hsvPtr, widthInSamples, heightInSamples, strideInSamples);
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <typename xType>
static void rgbx_to_hsv_C(const uint8_t* rgbxPtr, uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	static const compv_float32_t kHsvScaleTimes43[256] = {
		0.f, 43.f, 21.5f, 14.333334f, 10.75f, 8.60000038f, 7.16666698f, 6.14285755f, 5.375f,
		4.77777767f, 4.30000019f, 3.909091f, 3.58333349f, 3.30769253f, 3.07142878f, 2.86666679f,
		2.6875f, 2.52941179f, 2.38888884f, 2.26315784f, 2.1500001f, 2.0476191f, 1.9545455f,
		1.86956525f, 1.79166675f, 1.71999991f, 1.65384626f, 1.5925926f, 1.53571439f, 1.48275864f,
		1.4333334f, 1.38709676f, 1.34375f, 1.30303037f, 1.2647059f, 1.22857141f, 1.19444442f,
		1.16216218f, 1.13157892f, 1.1025641f, 1.07500005f, 1.04878044f, 1.02380955f, 1.f,
		0.977272749f, 0.955555558f, 0.934782624f, 0.914893568f, 0.895833373f, 0.877551019f,
		0.859999955f, 0.843137264f, 0.826923132f, 0.811320782f, 0.796296299f, 0.781818151f,
		0.767857194f, 0.754385948f, 0.741379321f, 0.728813589f, 0.716666698f, 0.704917967f,
		0.693548381f, 0.682539701f, 0.671875f, 0.661538482f, 0.651515186f, 0.641791046f,
		0.632352948f, 0.623188436f, 0.614285707f, 0.605633795f, 0.597222209f, 0.589041114f,
		0.581081092f, 0.573333323f, 0.565789461f, 0.558441579f, 0.551282048f, 0.544303834f,
		0.537500024f, 0.530864179f, 0.524390221f, 0.518072248f, 0.511904776f, 0.505882382f,
		0.5f, 0.494252861f, 0.488636374f, 0.483146071f, 0.477777779f, 0.472527474f, 0.467391312f, 0.462365597f
		, 0.457446784f, 0.452631593f, 0.447916687f, 0.443298966f, 0.43877551f, 0.434343427f,
		0.429999977f, 0.425742567f, 0.421568632f, 0.41747573f, 0.413461566f, 0.409523815f,
		0.405660391f, 0.401869148f, 0.398148149f, 0.394495398f, 0.390909076f, 0.387387395f,
		0.383928597f, 0.380530983f, 0.377192974f, 0.37391302f, 0.37068966f,
		0.367521375f, 0.364406794f, 0.361344546f, 0.358333349f, 0.355371892f, 0.352458984f,
		0.34959349f, 0.346774191f, 0.344000012f, 0.341269851f, 0.338582665f, 0.3359375f,
		0.333333343f, 0.330769241f, 0.328244269f, 0.325757593f, 0.323308289f, 0.320895523f,
		0.318518519f, 0.316176474f, 0.313868612f, 0.311594218f, 0.309352517f, 0.307142854f,
		0.304964542f, 0.302816898f, 0.300699294f, 0.298611104f, 0.296551734f, 0.294520557f,
		0.292517006f, 0.290540546f, 0.28859061f, 0.286666662f, 0.284768224f, 0.282894731f,
		0.281045765f, 0.27922079f, 0.277419358f, 0.275641024f, 0.273885369f, 0.272151917f,
		0.270440251f, 0.268750012f, 0.267080754f, 0.26543209f, 0.263803661f, 0.26219511f,
		0.26060605f, 0.259036124f, 0.257485032f, 0.255952388f, 0.254437864f, 0.252941191f,
		0.251461983f, 0.25f, 0.248554915f, 0.24712643f, 0.245714277f, 0.244318187f,
		0.242937848f, 0.241573036f, 0.240223452f, 0.23888889f, 0.237569064f,
		0.236263737f, 0.234972671f, 0.233695656f, 0.23243244f, 0.231182799f, 0.229946524f,
		0.228723392f, 0.227513224f, 0.226315796f, 0.225130886f, 0.223958343f, 0.222797915f,
		0.221649483f, 0.220512822f, 0.219387755f, 0.218274102f, 0.217171714f, 0.216080397f,
		0.214999989f, 0.213930339f, 0.212871283f, 0.211822659f, 0.210784316f, 0.209756091f,
		0.208737865f, 0.207729459f, 0.206730783f, 0.205741614f, 0.204761907f, 0.203791469f,
		0.202830195f, 0.201877937f, 0.200934574f, 0.200000003f, 0.199074075f, 0.198156685f,
		0.197247699f, 0.196347028f, 0.195454538f, 0.194570139f, 0.193693697f, 0.192825124f,
		0.191964298f, 0.191111118f, 0.190265492f, 0.189427301f, 0.188596487f, 0.18777293f,
		0.18695651f, 0.186147183f, 0.18534483f, 0.184549361f, 0.183760688f, 0.182978719f,
		0.182203397f, 0.181434587f, 0.180672273f, 0.179916307f, 0.179166675f, 0.178423241f,
		0.177685946f, 0.176954731f, 0.176229492f, 0.175510198f, 0.174796745f, 0.174089074f,
		0.173387095f, 0.172690749f,0.172000006f, 0.171314746f, 0.170634925f, 0.169960484f,
		0.169291332f, 0.168627456f,
	};
	static const compv_float32_t kHsvScaleTimes255[256] = {
		0.f, 255.f, 127.5f, 85.f, 63.75f, 51.f, 42.5f, 36.4285736f, 31.875f, 28.333334f, 25.5f,
		23.181818f, 21.25f, 19.6153851f, 18.2142868f, 17.f, 15.9375f, 15.f, 14.166667f,
		13.4210529f, 12.75f, 12.1428576f, 11.590909f, 11.086957f, 10.625f, 10.1999998f,
		9.80769253f, 9.44444466f, 9.1071434f, 8.79310322f, 8.5f, 8.22580624f, 7.96875f,
		7.72727299f, 7.5f, 7.28571415f, 7.08333349f, 6.89189196f, 6.71052647f, 6.53846169f,
		6.375f, 6.21951199f, 6.07142878f, 5.93023252f, 5.7954545f, 5.66666698f, 5.54347849f,
		5.42553186f, 5.3125f, 5.20408154f, 5.0999999f, 5.f, 4.90384626f, 4.81132078f,
		4.72222233f, 4.63636351f, 4.5535717f, 4.47368431f, 4.39655161f, 4.32203388f,
		4.25f, 4.18032742f, 4.11290312f, 4.04761934f, 3.984375f, 3.92307687f, 3.86363649f,
		3.80596995f, 3.75f, 3.69565225f, 3.64285707f, 3.59154916f, 3.54166675f,
		3.49315071f, 3.44594598f, 3.4000001f, 3.35526323f, 3.31168842f, 3.26923084f, 3.22784829f,
		3.1875f, 3.14814806f, 3.10975599f, 3.07228899f, 3.03571439f, 3.f, 2.96511626f, 2.93103456f,
		2.89772725f, 2.86516857f, 2.83333349f, 2.80219793f, 2.77173924f, 2.74193549f, 2.71276593f,
		2.68421054f, 2.65625f, 2.62886596f, 2.60204077f,
		2.5757575f, 2.54999995f, 2.52475238f, 2.5f, 2.47572827f, 2.45192313f, 2.42857146f,
		2.40566039f, 2.38317752f, 2.36111116f, 2.33944941f, 2.31818175f, 2.29729724f, 2.27678585f,
		2.2566371f, 2.23684216f, 2.21739125f, 2.1982758f, 2.17948723f, 2.16101694f, 2.14285731f,
		2.125f, 2.10743785f, 2.09016371f, 2.07317066f, 2.05645156f, 2.0400002f, 2.02380967f,
		2.00787401f, 1.9921875f, 1.97674417f, 1.96153843f, 1.94656491f, 1.93181825f, 1.91729331f,
		1.90298498f, 1.88888884f, 1.875f, 1.86131382f, 1.84782612f, 1.83453238f, 1.82142854f,
		1.80851054f, 1.79577458f, 1.78321671f, 1.77083337f, 1.75862074f, 1.74657536f,
		1.73469388f, 1.72297299f, 1.71140945f, 1.70000005f, 1.68874168f, 1.67763162f,
		1.66666663f, 1.65584421f, 1.64516127f, 1.63461542f, 1.6242038f, 1.61392415f, 1.60377347f,
		1.59375f, 1.58385098f, 1.57407403f, 1.56441712f, 1.554878f, 1.5454545f, 1.5361445f, 1.52694619f,
		1.51785719f, 1.50887573f, 1.5f, 1.4912281f, 1.48255813f, 1.47398841f, 1.46551728f, 1.45714283f,
		1.44886363f, 1.440678f, 1.43258429f, 1.42458093f, 1.41666675f, 1.40883982f,
		1.40109897f, 1.39344263f, 1.38586962f, 1.37837839f, 1.37096775f, 1.36363637f, 1.35638297f,
		1.34920633f, 1.34210527f, 1.3350786f, 1.328125f, 1.32124352f, 1.31443298f, 1.30769229f,
		1.30102038f, 1.29441619f, 1.28787875f, 1.281407f, 1.27499998f, 1.26865673f, 1.26237619f,
		1.25615764f, 1.25f, 1.24390244f, 1.23786414f, 1.231884f, 1.22596157f, 1.22009563f,
		1.21428573f, 1.2085309f, 1.2028302f, 1.19718313f, 1.19158876f, 1.18604648f, 1.18055558f,
		1.17511523f, 1.1697247f, 1.16438353f, 1.15909088f, 1.15384614f, 1.14864862f, 1.14349782f,
		1.13839293f, 1.13333333f, 1.12831855f, 1.123348f, 1.11842108f, 1.11353719f, 1.10869563f,
		1.10389614f, 1.0991379f, 1.09442055f, 1.08974361f, 1.08510637f, 1.08050847f, 1.07594931f,
		1.07142866f, 1.06694555f, 1.0625f, 1.05809128f, 1.05371892f, 1.04938269f, 1.04508185f, 1.04081631f,
		1.03658533f, 1.03238869f, 1.02822578f, 1.02409637f, 1.0200001f, 1.01593626f, 1.01190484f, 1.00790513f,
		1.00393701f, 1.f,
	};
#if 1
	size_t i, j;
	const xType* rgbxPtr_ = reinterpret_cast<const xType*>(rgbxPtr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);
	int minVal, maxVal, minus, r, g, b;
	int diff;
	compv_float32_t s255, s43;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const xType& rgbx = rgbxPtr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgbx[0], g = rgbx[1], b = rgbx[2];

			minVal = COMPV_MATH_MIN_INT(g, b);
			minVal = COMPV_MATH_MIN_INT(r, minVal);
			maxVal = COMPV_MATH_MAX_INT(g, b);
			maxVal = COMPV_MATH_MAX_INT(r, maxVal);

			diff = (maxVal == r) ? (g - b) : ((maxVal == g) ? (b - r) : (r - g)); // ASM: CMOV
			minus = maxVal - minVal;
			s43 = diff * kHsvScaleTimes43[minus];
			s255 = minus * kHsvScaleTimes255[maxVal];

			hsv[0] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s43, uint8_t) +
				((maxVal == r) ? 0 : ((maxVal == g) ? 85 : 171)); // ASM: CMOV

			hsv[1] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s255, uint8_t);

			hsv[2] = static_cast<uint8_t>(maxVal);
		}
		rgbxPtr_ += stride;
		hsvPtr_ += stride;
}
#else
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Branchless code for SIMD implementations (SSE, AVX and ARM NEON)");

	size_t i, j;
	const xType* rgbxPtr_ = reinterpret_cast<const xType*>(rgbxPtr);
	compv_uint8x3_t* hsvPtr_ = reinterpret_cast<compv_uint8x3_t*>(hsvPtr);
	int minVal, maxVal, minus, r, g, b;
	int diff, m0, m1, m2;
	compv_float32_t s255, s43;
	for (j = 0; j <height; ++j) {
		for (i = 0; i < width; ++i) {
			const xType& rgbx = rgbxPtr_[i];
			compv_uint8x3_t& hsv = hsvPtr_[i];
			r = rgbx[0], g = rgbx[1], b = rgbx[2];

			minVal = COMPV_MATH_MIN_INT(g, b);
			minVal = COMPV_MATH_MIN_INT(r, minVal);
			maxVal = COMPV_MATH_MAX_INT(g, b);
			maxVal = COMPV_MATH_MAX_INT(r, maxVal);

			m0 = -(maxVal == r); // (maxVal == r) ? 0xff : 0x00;
			m1 = -(maxVal == g) & ~m0; // ((maxVal == r) ? 0xff : 0x00) & ~m0
			m2 = ~(m0 | m1);
			diff = ((g - b) & m0) | ((b - r) & m1) | ((r - g) & m2);
			minus = maxVal - minVal;
			s43 = diff * kHsvScaleTimes43[minus];
			s255 = minus * kHsvScaleTimes255[maxVal];
			hsv[0] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s43, uint8_t)
				+ ((85 & m1) | (171 & m2));
			hsv[1] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s255, uint8_t);
			hsv[2] = static_cast<uint8_t>(maxVal);
		}
		rgbxPtr_ += stride;
		hsvPtr_ += stride;
	}
#endif
}

COMPV_NAMESPACE_END()
