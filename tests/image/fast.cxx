#include <compv/compv_api.h>

using namespace compv;

#define THRESHOLD			10
#define NONMAXIMA			1
#define FASTTYPE			COMPV_FAST_TYPE_9

#define JPEG_IMG_OPENGLBOOK	"C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg" // OpenGL book
#define JPEG_IMG_GRIOTS		"C:/Projects/GitHub/pan360/images/mandekalou.JPG" // Mande Griots
#define JPEG_IMG_VOITURE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

#define TEST_TYPE_OPENGLBOOK			0
#define TEST_TYPE_GRIOTS				1
#define TEST_TYPE_VOITURE				2

#define TEST_TYPE						TEST_TYPE_VOITURE

#if TEST_TYPE == TEST_TYPE_OPENGLBOOK
#	define JPEG_IMG						JPEG_IMG_OPENGLBOOK
#	define FAST9_T10_CORNERS_COUNT		5066
#	define FAST9_T10_CORNERS_SCORES		140402
#	define FAST9_T10_NONMAX_COUNT		1209
#	define FAST9_T10_NONMAX_SCORES		42470
#	define FAST9_T10_XF					457331.000f
#	define FAST9_T10_YF					643348.000f
#	define FAST9_T10_NONMAX_XF			111052.000f
#	define FAST9_T10_NONMAX_YF			151239.000f
#	define FAST10_T10_CORNERS_COUNT		3885
#	define FAST10_T10_CORNERS_SCORES	103004
#	define FAST10_T10_NONMAX_COUNT		1101
#	define FAST10_T10_NONMAX_SCORES		35089
#	define FAST12_T10_CORNERS_COUNT		2622
#	define FAST12_T10_CORNERS_SCORES	63005
#	define FAST12_T10_NONMAX_COUNT		890
#	define FAST12_T10_NONMAX_SCORES		25035
#	define FAST12_T10_XF				232877.000f
#	define FAST12_T10_YF				329083.000f
#	define FAST12_T10_NONMAX_XF			80539.0000f
#	define FAST12_T10_NONMAX_YF			110791.000f
#elif TEST_TYPE == TEST_TYPE_GRIOTS
#	define JPEG_IMG						JPEG_IMG_GRIOTS
#	define FAST9_T10_CORNERS_COUNT		22496
#	define FAST9_T10_CORNERS_SCORES		559737
#	define FAST9_T10_NONMAX_COUNT		5281
#	define FAST9_T10_NONMAX_SCORES		155897
#	define FAST9_T10_XF					6254713.00f
#	define FAST9_T10_YF					6051599.00f
#	define FAST9_T10_NONMAX_XF			1470054.00f
#	define FAST9_T10_NONMAX_YF			1422441.00f
#	define FAST10_T10_CORNERS_COUNT		16198
#	define FAST10_T10_CORNERS_SCORES	390719
#	define FAST10_T10_NONMAX_COUNT		4483
#	define FAST10_T10_NONMAX_SCORES		123855
#	define FAST12_T10_CORNERS_COUNT		9469
#	define FAST12_T10_CORNERS_SCORES	212953
#	define FAST12_T10_NONMAX_COUNT		3175
#	define FAST12_T10_NONMAX_SCORES		79383
#	define FAST12_T10_XF				2680181.00f
#	define FAST12_T10_YF				2452336.00f
#	define FAST12_T10_NONMAX_XF			903014.000f
#	define FAST12_T10_NONMAX_YF			820502.000f
#elif TEST_TYPE == TEST_TYPE_VOITURE
#	define JPEG_IMG						JPEG_IMG_VOITURE
#	define FAST9_T10_CORNERS_COUNT		27106
#	define FAST9_T10_CORNERS_SCORES		566896
#	define FAST9_T10_NONMAX_COUNT		6920
#	define FAST9_T10_NONMAX_SCORES		153412
#	define FAST9_T10_XF					26746936.0f
#	define FAST9_T10_YF					14513604.0f
#	define FAST9_T10_NONMAX_XF			7122647.00f
#	define FAST9_T10_NONMAX_YF			3715590.00f
#	define FAST10_T10_CORNERS_COUNT		18726
#	define FAST10_T10_CORNERS_SCORES	385062
#	define FAST10_T10_NONMAX_COUNT		5411
#	define FAST10_T10_NONMAX_SCORES		116492
#	define FAST12_T10_CORNERS_COUNT		10991
#	define FAST12_T10_CORNERS_SCORES	213734
#	define FAST12_T10_NONMAX_COUNT		3726
#	define FAST12_T10_NONMAX_SCORES		74713
#	define FAST12_T10_XF				11180928.0f
#	define FAST12_T10_YF				5918576.0f
#	define FAST12_T10_NONMAX_XF			3889938.00f
#	define FAST12_T10_NONMAX_YF			2000219.00f
#endif

#define FAST_LOOP_COUNT	1000

static const uint16_t Fast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
static const uint16_t Fast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

static void FastBuildCode(int N)
{
    std::string code = "";
    const uint16_t* FastXFlags = (N == 9 ? Fast9Flags : Fast12Flags);

    for (unsigned i = 0; i < 16; ++i) {
        code += ("ndarker = 255;\n");
        code += ("nbrighter = 255;\n");
        code += ("if ((fbrighters & " + std::to_string(FastXFlags[i]) + ") == " + std::to_string(FastXFlags[i]) + ") {\n");
        for (unsigned j = i; j < i + N; ++j) {
            code += ("if (dbrighters[" + std::to_string(j & 15) + "] < nbrighter) nbrighter = dbrighters[" + std::to_string(j & 15) + "];\n");
        }
        code += ("}\n");
        code += ("if ((fdarkers & " + std::to_string(FastXFlags[i]) + ") == " + std::to_string(FastXFlags[i]) + ") {\n");
        for (unsigned j = i; j < i + N; ++j) {
            code += ("if (ddarkers[" + std::to_string(j & 15) + "] < ndarker) ndarker = ddarkers[" + std::to_string(j & 15) + "];\n");
        }
        code += ("}\n");
        code += ("else if (nbrighter == 255) { goto next" + std::to_string(i) + "; }\n");

        code += ("strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));\n");
        code += ("next" + std::to_string(i) + ": void();\n");
    }

    FILE*file = fopen(std::string("./fast" + std::to_string(N)+".txt").c_str(), "w+");
    if (file) {
        fwrite(code.c_str(), 1, code.length(), file);
        fclose(file);
    }
}

static void FastFlagsArc(int N)
{
    int m, n, c;
    uint16_t flags[16], flag;
    for (m = 0; m < 16; ++m) {
        c = N + m;
        flag = 0;
        for (n = m; n < c; ++n) {
            flag |= (1 << (n & 15));
        }
        flags[m] = flag;
    }
    printf("Flags%d[16] = {", N);
    for (m = 0; m < 16; ++m) {
        printf("0x%x,", flags[m]);
    }
    printf("}");
}

// Shuffles used for horizontal minimum
static void FastShufflesArc(int N)
{
    int m, n, c, s;
    uint8_t shuffles[16][16];
    for (m = 0; m < 16; ++m) {
        c = N + m;
        // When the bit flag is set to 0xff the element is ignored and replaced with a zero.
        // Computing the min requires N consecutive non-zero elements (this is the arc) this is why the first element is duplicated.
        for (s = 0; s < 16; ++s) {
            shuffles[m][s] = m;
        }

        for (s = 0, n = m; n < c; ++n, ++s) {
            shuffles[m][(n & 15)] = (n & 15);
        }
    }
    printf("shuffles%d[16][16] = {\n", N);
    for (m = 0; m < 16; ++m) {
        printf("{");
        for (s = 0; s < 16; ++s) {
            printf("0x%x,", shuffles[m][s]);
        }
        printf("},\n");
    }
    printf("};");
}

bool TestFAST()
{
    CompVObjWrapper<CompVFeatureDete* > fast;
    CompVObjWrapper<CompVImage *> image;
	CompVObjWrapper<CompVBoxInterestPoint* > interestPoints;
    int32_t val32;
    bool valBool;
    uint64_t timeStart, timeEnd;

    //FastBuildCode(12);
    //FastShufflesArc(12);

    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert the image to grayscal (required by feture detectors)
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

    // Create the FAST feature detector
    COMPV_CHECK_CODE_ASSERT(CompVFeatureDete::newObj(COMPV_FAST_ID, &fast));

    // Set the default values
    val32 = THRESHOLD;
    COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_FAST_SET_INT32_THRESHOLD, &val32, sizeof(val32)));
    val32 = FASTTYPE;
    COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_FAST_SET_INT32_FAST_TYPE, &val32, sizeof(val32)));
    valBool = NONMAXIMA;
    COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));

    // Detect keypoints
    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < FAST_LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_ASSERT(fast->process(image, interestPoints));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

    // Regression test
#if THRESHOLD == 10
    float sum_scores = 0.f;
    float xf = 0.f;
    float yf = 0.f;
	for (size_t i = 0; i < interestPoints->size(); ++i) {
		sum_scores += interestPoints->at(i)->strength;
		xf += interestPoints->at(i)->xf();
		yf += interestPoints->at(i)->yf();
	}
#	if NONMAXIMA == 1
	COMPV_ASSERT(interestPoints->size() == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_NONMAX_COUNT : ((FASTTYPE == COMPV_FAST_TYPE_12) ? FAST12_T10_NONMAX_COUNT : FAST10_T10_NONMAX_COUNT)));
    COMPV_ASSERT(sum_scores == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_NONMAX_SCORES : ((FASTTYPE == COMPV_FAST_TYPE_12) ? FAST12_T10_NONMAX_SCORES : FAST10_T10_NONMAX_SCORES)));
	COMPV_ASSERT(xf == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_NONMAX_XF : FAST12_T10_NONMAX_XF));
	COMPV_ASSERT(yf == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_NONMAX_YF : FAST12_T10_NONMAX_YF));
#	else
    COMPV_ASSERT(interestPoints->size() == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_CORNERS_COUNT : ((FASTTYPE == COMPV_FAST_TYPE_12) ? FAST12_T10_CORNERS_COUNT : FAST10_T10_CORNERS_COUNT)));
    COMPV_ASSERT(sum_scores == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_CORNERS_SCORES : ((FASTTYPE == COMPV_FAST_TYPE_12) ? FAST12_T10_CORNERS_SCORES : FAST10_T10_CORNERS_SCORES)));
	COMPV_ASSERT(xf == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_XF : FAST12_T10_XF));
	COMPV_ASSERT(yf == ((FASTTYPE == COMPV_FAST_TYPE_9) ? FAST9_T10_YF : FAST12_T10_YF));
#	endif
#endif
    return true;
}
