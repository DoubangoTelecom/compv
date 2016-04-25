/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
/* @description
* This class implements ORB (Oriented FAST and Rotated BRIEF) feature descriptor.
* Some literature:
* ORB final: https://www.willowgarage.com/sites/default/files/orb_final.pdf
* BRIEF descriptor: https://www.robots.ox.ac.uk/~vgg/rg/papers/CalonderLSF10.pdf
* Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf (check "Intensity centroid" used in ORB vs "Gradient centroid")
* Image moments: https://en.wikipedia.org/wiki/Image_moment
* Centroid: https://en.wikipedia.org/wiki/Centroid
*/
#include "compv/features/orb/compv_feature_orb_desc.h"
#include "compv/features/orb/compv_feature_orb_dete.h"
#include "compv/compv_mathutils.h"
#include "compv/compv_mem.h"
#include "compv/compv_gauss.h"
#include "compv/compv_debug.h"

#include <algorithm>

COMPV_NAMESPACE_BEGIN()

// Default values from the detector
extern int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT;
extern int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT;
extern bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP;
extern int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
extern float COMPV_FEATURE_DETE_ORB_PYRAMID_SF;
extern COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE;

int COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE = 7;
double COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA = 2.0;

// FIXME: GaussianBlure
// FIXME: BorderExtend
// FIXME: variable patch size

CompVFeatureDescORB::CompVFeatureDescORB()
    : CompVFeatureDesc(COMPV_ORB_ID)
{

}

CompVFeatureDescORB::~CompVFeatureDescORB()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDescORB::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case -1:
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

// FIXME:
static int32_t bit_pattern_31_[256 * 4] = {
    8, -3, 9, 5/*mean (0), correlation (0)*/,
    4, 2, 7, -12/*mean (1.12461e-05), correlation (0.0437584)*/,
    -11, 9, -8, 2/*mean (3.37382e-05), correlation (0.0617409)*/,
    7, -12, 12, -13/*mean (5.62303e-05), correlation (0.0636977)*/,
    2, -13, 2, 12/*mean (0.000134953), correlation (0.085099)*/,
    1, -7, 1, 6/*mean (0.000528565), correlation (0.0857175)*/,
    -2, -10, -2, -4/*mean (0.0188821), correlation (0.0985774)*/,
    -13, -13, -11, -8/*mean (0.0363135), correlation (0.0899616)*/,
    -13, -3, -12, -9/*mean (0.121806), correlation (0.099849)*/,
    10, 4, 11, 9/*mean (0.122065), correlation (0.093285)*/,
    -13, -8, -8, -9/*mean (0.162787), correlation (0.0942748)*/,
    -11, 7, -9, 12/*mean (0.21561), correlation (0.0974438)*/,
    7, 7, 12, 6/*mean (0.160583), correlation (0.130064)*/,
    -4, -5, -3, 0/*mean (0.228171), correlation (0.132998)*/,
    -13, 2, -12, -3/*mean (0.00997526), correlation (0.145926)*/,
    -9, 0, -7, 5/*mean (0.198234), correlation (0.143636)*/,
    12, -6, 12, -1/*mean (0.0676226), correlation (0.16689)*/,
    -3, 6, -2, 12/*mean (0.166847), correlation (0.171682)*/,
    -6, -13, -4, -8/*mean (0.101215), correlation (0.179716)*/,
    11, -13, 12, -8/*mean (0.200641), correlation (0.192279)*/,
    4, 7, 5, 1/*mean (0.205106), correlation (0.186848)*/,
    5, -3, 10, -3/*mean (0.234908), correlation (0.192319)*/,
    3, -7, 6, 12/*mean (0.0709964), correlation (0.210872)*/,
    -8, -7, -6, -2/*mean (0.0939834), correlation (0.212589)*/,
    -2, 11, -1, -10/*mean (0.127778), correlation (0.20866)*/,
    -13, 12, -8, 10/*mean (0.14783), correlation (0.206356)*/,
    -7, 3, -5, -3/*mean (0.182141), correlation (0.198942)*/,
    -4, 2, -3, 7/*mean (0.188237), correlation (0.21384)*/,
    -10, -12, -6, 11/*mean (0.14865), correlation (0.23571)*/,
    5, -12, 6, -7/*mean (0.222312), correlation (0.23324)*/,
    5, -6, 7, -1/*mean (0.229082), correlation (0.23389)*/,
    1, 0, 4, -5/*mean (0.241577), correlation (0.215286)*/,
    9, 11, 11, -13/*mean (0.00338507), correlation (0.251373)*/,
    4, 7, 4, 12/*mean (0.131005), correlation (0.257622)*/,
    2, -1, 4, 4/*mean (0.152755), correlation (0.255205)*/,
    -4, -12, -2, 7/*mean (0.182771), correlation (0.244867)*/,
    -8, -5, -7, -10/*mean (0.186898), correlation (0.23901)*/,
    4, 11, 9, 12/*mean (0.226226), correlation (0.258255)*/,
    0, -8, 1, -13/*mean (0.0897886), correlation (0.274827)*/,
    -13, -2, -8, 2/*mean (0.148774), correlation (0.28065)*/,
    -3, -2, -2, 3/*mean (0.153048), correlation (0.283063)*/,
    -6, 9, -4, -9/*mean (0.169523), correlation (0.278248)*/,
    8, 12, 10, 7/*mean (0.225337), correlation (0.282851)*/,
    0, 9, 1, 3/*mean (0.226687), correlation (0.278734)*/,
    7, -5, 11, -10/*mean (0.00693882), correlation (0.305161)*/,
    -13, -6, -11, 0/*mean (0.0227283), correlation (0.300181)*/,
    10, 7, 12, 1/*mean (0.125517), correlation (0.31089)*/,
    -6, -3, -6, 12/*mean (0.131748), correlation (0.312779)*/,
    10, -9, 12, -4/*mean (0.144827), correlation (0.292797)*/,
    -13, 8, -8, -12/*mean (0.149202), correlation (0.308918)*/,
    -13, 0, -8, -4/*mean (0.160909), correlation (0.310013)*/,
    3, 3, 7, 8/*mean (0.177755), correlation (0.309394)*/,
    5, 7, 10, -7/*mean (0.212337), correlation (0.310315)*/,
    -1, 7, 1, -12/*mean (0.214429), correlation (0.311933)*/,
    3, -10, 5, 6/*mean (0.235807), correlation (0.313104)*/,
    2, -4, 3, -10/*mean (0.00494827), correlation (0.344948)*/,
    -13, 0, -13, 5/*mean (0.0549145), correlation (0.344675)*/,
    -13, -7, -12, 12/*mean (0.103385), correlation (0.342715)*/,
    -13, 3, -11, 8/*mean (0.134222), correlation (0.322922)*/,
    -7, 12, -4, 7/*mean (0.153284), correlation (0.337061)*/,
    6, -10, 12, 8/*mean (0.154881), correlation (0.329257)*/,
    -9, -1, -7, -6/*mean (0.200967), correlation (0.33312)*/,
    -2, -5, 0, 12/*mean (0.201518), correlation (0.340635)*/,
    -12, 5, -7, 5/*mean (0.207805), correlation (0.335631)*/,
    3, -10, 8, -13/*mean (0.224438), correlation (0.34504)*/,
    -7, -7, -4, 5/*mean (0.239361), correlation (0.338053)*/,
    -3, -2, -1, -7/*mean (0.240744), correlation (0.344322)*/,
    2, 9, 5, -11/*mean (0.242949), correlation (0.34145)*/,
    -11, -13, -5, -13/*mean (0.244028), correlation (0.336861)*/,
    -1, 6, 0, -1/*mean (0.247571), correlation (0.343684)*/,
    5, -3, 5, 2/*mean (0.000697256), correlation (0.357265)*/,
    -4, -13, -4, 12/*mean (0.00213675), correlation (0.373827)*/,
    -9, -6, -9, 6/*mean (0.0126856), correlation (0.373938)*/,
    -12, -10, -8, -4/*mean (0.0152497), correlation (0.364237)*/,
    10, 2, 12, -3/*mean (0.0299933), correlation (0.345292)*/,
    7, 12, 12, 12/*mean (0.0307242), correlation (0.366299)*/,
    -7, -13, -6, 5/*mean (0.0534975), correlation (0.368357)*/,
    -4, 9, -3, 4/*mean (0.099865), correlation (0.372276)*/,
    7, -1, 12, 2/*mean (0.117083), correlation (0.364529)*/,
    -7, 6, -5, 1/*mean (0.126125), correlation (0.369606)*/,
    -13, 11, -12, 5/*mean (0.130364), correlation (0.358502)*/,
    -3, 7, -2, -6/*mean (0.131691), correlation (0.375531)*/,
    7, -8, 12, -7/*mean (0.160166), correlation (0.379508)*/,
    -13, -7, -11, -12/*mean (0.167848), correlation (0.353343)*/,
    1, -3, 12, 12/*mean (0.183378), correlation (0.371916)*/,
    2, -6, 3, 0/*mean (0.228711), correlation (0.371761)*/,
    -4, 3, -2, -13/*mean (0.247211), correlation (0.364063)*/,
    -1, -13, 1, 9/*mean (0.249325), correlation (0.378139)*/,
    7, 1, 8, -6/*mean (0.000652272), correlation (0.411682)*/,
    1, -1, 3, 12/*mean (0.00248538), correlation (0.392988)*/,
    9, 1, 12, 6/*mean (0.0206815), correlation (0.386106)*/,
    -1, -9, -1, 3/*mean (0.0364485), correlation (0.410752)*/,
    -13, -13, -10, 5/*mean (0.0376068), correlation (0.398374)*/,
    7, 7, 10, 12/*mean (0.0424202), correlation (0.405663)*/,
    12, -5, 12, 9/*mean (0.0942645), correlation (0.410422)*/,
    6, 3, 7, 11/*mean (0.1074), correlation (0.413224)*/,
    5, -13, 6, 10/*mean (0.109256), correlation (0.408646)*/,
    2, -12, 2, 3/*mean (0.131691), correlation (0.416076)*/,
    3, 8, 4, -6/*mean (0.165081), correlation (0.417569)*/,
    2, 6, 12, -13/*mean (0.171874), correlation (0.408471)*/,
    9, -12, 10, 3/*mean (0.175146), correlation (0.41296)*/,
    -8, 4, -7, 9/*mean (0.183682), correlation (0.402956)*/,
    -11, 12, -4, -6/*mean (0.184672), correlation (0.416125)*/,
    1, 12, 2, -8/*mean (0.191487), correlation (0.386696)*/,
    6, -9, 7, -4/*mean (0.192668), correlation (0.394771)*/,
    2, 3, 3, -2/*mean (0.200157), correlation (0.408303)*/,
    6, 3, 11, 0/*mean (0.204588), correlation (0.411762)*/,
    3, -3, 8, -8/*mean (0.205904), correlation (0.416294)*/,
    7, 8, 9, 3/*mean (0.213237), correlation (0.409306)*/,
    -11, -5, -6, -4/*mean (0.243444), correlation (0.395069)*/,
    -10, 11, -5, 10/*mean (0.247672), correlation (0.413392)*/,
    -5, -8, -3, 12/*mean (0.24774), correlation (0.411416)*/,
    -10, 5, -9, 0/*mean (0.00213675), correlation (0.454003)*/,
    8, -1, 12, -6/*mean (0.0293635), correlation (0.455368)*/,
    4, -6, 6, -11/*mean (0.0404971), correlation (0.457393)*/,
    -10, 12, -8, 7/*mean (0.0481107), correlation (0.448364)*/,
    4, -2, 6, 7/*mean (0.050641), correlation (0.455019)*/,
    -2, 0, -2, 12/*mean (0.0525978), correlation (0.44338)*/,
    -5, -8, -5, 2/*mean (0.0629667), correlation (0.457096)*/,
    7, -6, 10, 12/*mean (0.0653846), correlation (0.445623)*/,
    -9, -13, -8, -8/*mean (0.0858749), correlation (0.449789)*/,
    -5, -13, -5, -2/*mean (0.122402), correlation (0.450201)*/,
    8, -8, 9, -13/*mean (0.125416), correlation (0.453224)*/,
    -9, -11, -9, 0/*mean (0.130128), correlation (0.458724)*/,
    1, -8, 1, -2/*mean (0.132467), correlation (0.440133)*/,
    7, -4, 9, 1/*mean (0.132692), correlation (0.454)*/,
    -2, 1, -1, -4/*mean (0.135695), correlation (0.455739)*/,
    11, -6, 12, -11/*mean (0.142904), correlation (0.446114)*/,
    -12, -9, -6, 4/*mean (0.146165), correlation (0.451473)*/,
    3, 7, 7, 12/*mean (0.147627), correlation (0.456643)*/,
    5, 5, 10, 8/*mean (0.152901), correlation (0.455036)*/,
    0, -4, 2, 8/*mean (0.167083), correlation (0.459315)*/,
    -9, 12, -5, -13/*mean (0.173234), correlation (0.454706)*/,
    0, 7, 2, 12/*mean (0.18312), correlation (0.433855)*/,
    -1, 2, 1, 7/*mean (0.185504), correlation (0.443838)*/,
    5, 11, 7, -9/*mean (0.185706), correlation (0.451123)*/,
    3, 5, 6, -8/*mean (0.188968), correlation (0.455808)*/,
    -13, -4, -8, 9/*mean (0.191667), correlation (0.459128)*/,
    -5, 9, -3, -3/*mean (0.193196), correlation (0.458364)*/,
    -4, -7, -3, -12/*mean (0.196536), correlation (0.455782)*/,
    6, 5, 8, 0/*mean (0.1972), correlation (0.450481)*/,
    -7, 6, -6, 12/*mean (0.199438), correlation (0.458156)*/,
    -13, 6, -5, -2/*mean (0.211224), correlation (0.449548)*/,
    1, -10, 3, 10/*mean (0.211718), correlation (0.440606)*/,
    4, 1, 8, -4/*mean (0.213034), correlation (0.443177)*/,
    -2, -2, 2, -13/*mean (0.234334), correlation (0.455304)*/,
    2, -12, 12, 12/*mean (0.235684), correlation (0.443436)*/,
    -2, -13, 0, -6/*mean (0.237674), correlation (0.452525)*/,
    4, 1, 9, 3/*mean (0.23962), correlation (0.444824)*/,
    -6, -10, -3, -5/*mean (0.248459), correlation (0.439621)*/,
    -3, -13, -1, 1/*mean (0.249505), correlation (0.456666)*/,
    7, 5, 12, -11/*mean (0.00119208), correlation (0.495466)*/,
    4, -2, 5, -7/*mean (0.00372245), correlation (0.484214)*/,
    -13, 9, -9, -5/*mean (0.00741116), correlation (0.499854)*/,
    7, 1, 8, 6/*mean (0.0208952), correlation (0.499773)*/,
    7, -8, 7, 6/*mean (0.0220085), correlation (0.501609)*/,
    -7, -4, -7, 1/*mean (0.0233806), correlation (0.496568)*/,
    -8, 11, -7, -8/*mean (0.0236505), correlation (0.489719)*/,
    -13, 6, -12, -8/*mean (0.0268781), correlation (0.503487)*/,
    2, 4, 3, 9/*mean (0.0323324), correlation (0.501938)*/,
    10, -5, 12, 3/*mean (0.0399235), correlation (0.494029)*/,
    -6, -5, -6, 7/*mean (0.0420153), correlation (0.486579)*/,
    8, -3, 9, -8/*mean (0.0548021), correlation (0.484237)*/,
    2, -12, 2, 8/*mean (0.0616622), correlation (0.496642)*/,
    -11, -2, -10, 3/*mean (0.0627755), correlation (0.498563)*/,
    -12, -13, -7, -9/*mean (0.0829622), correlation (0.495491)*/,
    -11, 0, -10, -5/*mean (0.0843342), correlation (0.487146)*/,
    5, -3, 11, 8/*mean (0.0929937), correlation (0.502315)*/,
    -2, -13, -1, 12/*mean (0.113327), correlation (0.48941)*/,
    -1, -8, 0, 9/*mean (0.132119), correlation (0.467268)*/,
    -13, -11, -12, -5/*mean (0.136269), correlation (0.498771)*/,
    -10, -2, -10, 11/*mean (0.142173), correlation (0.498714)*/,
    -3, 9, -2, -13/*mean (0.144141), correlation (0.491973)*/,
    2, -3, 3, 2/*mean (0.14892), correlation (0.500782)*/,
    -9, -13, -4, 0/*mean (0.150371), correlation (0.498211)*/,
    -4, 6, -3, -10/*mean (0.152159), correlation (0.495547)*/,
    -4, 12, -2, -7/*mean (0.156152), correlation (0.496925)*/,
    -6, -11, -4, 9/*mean (0.15749), correlation (0.499222)*/,
    6, -3, 6, 11/*mean (0.159211), correlation (0.503821)*/,
    -13, 11, -5, 5/*mean (0.162427), correlation (0.501907)*/,
    11, 11, 12, 6/*mean (0.16652), correlation (0.497632)*/,
    7, -5, 12, -2/*mean (0.169141), correlation (0.484474)*/,
    -1, 12, 0, 7/*mean (0.169456), correlation (0.495339)*/,
    -4, -8, -3, -2/*mean (0.171457), correlation (0.487251)*/,
    -7, 1, -6, 7/*mean (0.175), correlation (0.500024)*/,
    -13, -12, -8, -13/*mean (0.175866), correlation (0.497523)*/,
    -7, -2, -6, -8/*mean (0.178273), correlation (0.501854)*/,
    -8, 5, -6, -9/*mean (0.181107), correlation (0.494888)*/,
    -5, -1, -4, 5/*mean (0.190227), correlation (0.482557)*/,
    -13, 7, -8, 10/*mean (0.196739), correlation (0.496503)*/,
    1, 5, 5, -13/*mean (0.19973), correlation (0.499759)*/,
    1, 0, 10, -13/*mean (0.204465), correlation (0.49873)*/,
    9, 12, 10, -1/*mean (0.209334), correlation (0.49063)*/,
    5, -8, 10, -9/*mean (0.211134), correlation (0.503011)*/,
    -1, 11, 1, -13/*mean (0.212), correlation (0.499414)*/,
    -9, -3, -6, 2/*mean (0.212168), correlation (0.480739)*/,
    -1, -10, 1, 12/*mean (0.212731), correlation (0.502523)*/,
    -13, 1, -8, -10/*mean (0.21327), correlation (0.489786)*/,
    8, -11, 10, -6/*mean (0.214159), correlation (0.488246)*/,
    2, -13, 3, -6/*mean (0.216993), correlation (0.50287)*/,
    7, -13, 12, -9/*mean (0.223639), correlation (0.470502)*/,
    -10, -10, -5, -7/*mean (0.224089), correlation (0.500852)*/,
    -10, -8, -8, -13/*mean (0.228666), correlation (0.502629)*/,
    4, -6, 8, 5/*mean (0.22906), correlation (0.498305)*/,
    3, 12, 8, -13/*mean (0.233378), correlation (0.503825)*/,
    -4, 2, -3, -3/*mean (0.234323), correlation (0.476692)*/,
    5, -13, 10, -12/*mean (0.236392), correlation (0.475462)*/,
    4, -13, 5, -1/*mean (0.236842), correlation (0.504132)*/,
    -9, 9, -4, 3/*mean (0.236977), correlation (0.497739)*/,
    0, 3, 3, -9/*mean (0.24314), correlation (0.499398)*/,
    -12, 1, -6, 1/*mean (0.243297), correlation (0.489447)*/,
    3, 2, 4, -8/*mean (0.00155196), correlation (0.553496)*/,
    -10, -10, -10, 9/*mean (0.00239541), correlation (0.54297)*/,
    8, -13, 12, 12/*mean (0.0034413), correlation (0.544361)*/,
    -8, -12, -6, -5/*mean (0.003565), correlation (0.551225)*/,
    2, 2, 3, 7/*mean (0.00835583), correlation (0.55285)*/,
    10, 6, 11, -8/*mean (0.00885065), correlation (0.540913)*/,
    6, 8, 8, -12/*mean (0.0101552), correlation (0.551085)*/,
    -7, 10, -6, 5/*mean (0.0102227), correlation (0.533635)*/,
    -3, -9, -3, 9/*mean (0.0110211), correlation (0.543121)*/,
    -1, -13, -1, 5/*mean (0.0113473), correlation (0.550173)*/,
    -3, -7, -3, 4/*mean (0.0140913), correlation (0.554774)*/,
    -8, -2, -8, 3/*mean (0.017049), correlation (0.55461)*/,
    4, 2, 12, 12/*mean (0.01778), correlation (0.546921)*/,
    2, -5, 3, 11/*mean (0.0224022), correlation (0.549667)*/,
    6, -9, 11, -13/*mean (0.029161), correlation (0.546295)*/,
    3, -1, 7, 12/*mean (0.0303081), correlation (0.548599)*/,
    11, -1, 12, 4/*mean (0.0355151), correlation (0.523943)*/,
    -3, 0, -3, 6/*mean (0.0417904), correlation (0.543395)*/,
    4, -11, 4, 12/*mean (0.0487292), correlation (0.542818)*/,
    2, -4, 2, 1/*mean (0.0575124), correlation (0.554888)*/,
    -10, -6, -8, 1/*mean (0.0594242), correlation (0.544026)*/,
    -13, 7, -11, 1/*mean (0.0597391), correlation (0.550524)*/,
    -13, 12, -11, -13/*mean (0.0608974), correlation (0.55383)*/,
    6, 0, 11, -13/*mean (0.065126), correlation (0.552006)*/,
    0, -1, 1, 4/*mean (0.074224), correlation (0.546372)*/,
    -13, 3, -9, -2/*mean (0.0808592), correlation (0.554875)*/,
    -9, 8, -6, -3/*mean (0.0883378), correlation (0.551178)*/,
    -13, -6, -8, -2/*mean (0.0901035), correlation (0.548446)*/,
    5, -9, 8, 10/*mean (0.0949843), correlation (0.554694)*/,
    2, 7, 3, -9/*mean (0.0994152), correlation (0.550979)*/,
    -1, -6, -1, -1/*mean (0.10045), correlation (0.552714)*/,
    9, 5, 11, -2/*mean (0.100686), correlation (0.552594)*/,
    11, -3, 12, -8/*mean (0.101091), correlation (0.532394)*/,
    3, 0, 3, 5/*mean (0.101147), correlation (0.525576)*/,
    -1, 4, 0, 10/*mean (0.105263), correlation (0.531498)*/,
    3, -6, 4, 5/*mean (0.110785), correlation (0.540491)*/,
    -13, 0, -10, 5/*mean (0.112798), correlation (0.536582)*/,
    5, 8, 12, 11/*mean (0.114181), correlation (0.555793)*/,
    8, 9, 9, -6/*mean (0.117431), correlation (0.553763)*/,
    7, -4, 8, -12/*mean (0.118522), correlation (0.553452)*/,
    -10, 4, -10, 9/*mean (0.12094), correlation (0.554785)*/,
    7, 3, 12, 4/*mean (0.122582), correlation (0.555825)*/,
    9, -7, 10, -2/*mean (0.124978), correlation (0.549846)*/,
    7, 0, 12, -2/*mean (0.127002), correlation (0.537452)*/,
    -1, -6, 0, -11/*mean (0.127148), correlation (0.547401)*/
};

// Helpful site to generate gaussian values: http://dev.theomader.com/gaussian-kernel-calculator/
static const double gfilterGaussianBlur2[7][7] = {
    { 0.00492233, 0.00919613, 0.01338028, 0.01516185, 0.01338028, 0.00919613, 0.00492233 },
    { 0.00919613, 0.01718062, 0.02499766, 0.02832606, 0.02499766, 0.01718062, 0.00919613 },
    { 0.01338028, 0.02499766, 0.03637138, 0.04121417, 0.03637138, 0.02499766, 0.01338028 },
    { 0.01516185, 0.02832606, 0.04121417, 0.04670178, 0.04121417, 0.02832606, 0.01516185 },
    { 0.01338028, 0.02499766, 0.03637138, 0.04121417, 0.03637138, 0.02499766, 0.01338028 },
    { 0.00919613, 0.01718062, 0.02499766, 0.02832606, 0.02499766, 0.01718062, 0.00919613 },
    { 0.00492233, 0.00919613, 0.01338028, 0.01516185, 0.01338028, 0.00919613, 0.00492233 }
};
static const double gfilterGaussianBlur1[7] = { 0.07015933, 0.13107488, 0.19071282, 0.21610594, 0.19071282, 0.13107488, 0.07015933 };
static COMPV_ERROR_CODE convlt2(uint8_t* img, int imgw, int imgs, int imgh, const double* ker, int ker_size)
{
    COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

    uint8_t* outImg = (uint8_t*)CompVMem::malloc(imgh * imgs);
    const uint8_t *topleft, *img_ptr;
    double sum;
    const double *ker_ptr;
    int imgpad, i, j, row, col;
    int ker_size_div2 = ker_size >> 1;
    img_ptr = img;
    imgpad = (imgs - imgw) + ker_size_div2 + ker_size_div2;

    for (j = ker_size_div2; j < imgh - ker_size_div2; ++j) {
        for (i = ker_size_div2; i < imgw - ker_size_div2; ++i) {
            sum = 0;
            topleft = img_ptr;
            ker_ptr = ker;
            for (row = 0; row < ker_size; ++row) {
                for (col = 0; col < ker_size; ++col) {
                    sum += topleft[col] * ker_ptr[col];
                }
                ker_ptr += ker_size;
                topleft += imgs;
            }
            outImg[(j * imgs) + i] = (uint8_t)sum;
            ++img_ptr;
        }
        img_ptr += imgpad;
    }
    CompVMem::copy(img, outImg, imgh * imgs); // FIXME: garbage
    CompVMem::free((void**)&outImg);

    return COMPV_ERROR_CODE_S_OK;
}
static COMPV_ERROR_CODE convlt1(uint8_t* img, int imgw, int imgs, int imgh, const double* ker, int ker_size)
{
    COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

    uint8_t *imgTmp;
    const uint8_t *topleft, *img_ptr;
    double sum;
    int imgpad, i, j, row, col;
    int ker_size_div2 = ker_size >> 1;

    imgTmp = (uint8_t*)CompVMem::malloc(imgh * imgs);
    COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    // Horizontal
    img_ptr = img + ker_size_div2;
    imgpad = (imgs - imgw) + ker_size_div2 + ker_size_div2;
    for (j = 0; j < imgh; ++j) {
        for (i = ker_size_div2; i < imgw - ker_size_div2; ++i) {
            sum = 0;
            topleft = img_ptr - ker_size_div2;
            for (col = 0; col < ker_size; ++col) {
                sum += topleft[col] * ker[col];
            }
            imgTmp[(j * imgs) + i] = (uint8_t)sum;
            ++img_ptr;
        }
        img_ptr += imgpad;
    }

    // Vertical
    img_ptr = imgTmp + (ker_size_div2 * imgs); // output from hz filtering is now used as input
    imgpad = (imgs - imgw);
    for (j = ker_size_div2; j < imgh - ker_size_div2; ++j) {
        for (i = 0; i < imgw; ++i) {
            sum = 0;
            topleft = img_ptr - (ker_size_div2 * imgs);
            for (row = 0; row < ker_size; ++row) {
                sum += topleft[0] * ker[row];
                topleft += imgs;
            }
            img[(j * imgs) + i] = (uint8_t)sum;
            ++img_ptr;
        }
        img_ptr += imgpad;
    }

    CompVMem::free((void**)&imgTmp);

    return COMPV_ERROR_CODE_S_OK;
}

// FIXME: adds support for BRIEF 128, 256 or 512 -> uint16, uint32, uint64
static void brief256(const uint8_t* img, int imgs, int imgw, int imgh, int kpx, int kpy, float cosT, float sinT, void* desc)
{
    uint8_t xi, yi;
    int x, y, i, j;

    uint64_t* _desc = (uint64_t*)desc;
    const uint8_t* center = img + (int32_t)(((int)kpy * imgs) + (int)kpx); // Translate the image to have the keypoint at the center. This is required before applying the rotated patch.

    // FIXME: To generate the locations (Xi, Yi):
    // See brief document: Isotropic Gaussian distribution
    static int Xi[256][2/*x,y*/] = { };
    static int Yi[256][2/*x,y*/] = { };
    static bool gen = false;
    static const int patch_div2 = 15;
    static const uint64_t u64_1 = 1;
    if (!gen) {
        for (int i = 0; i < 256; ++i) {
            // Patch centered at (0, 0). Rotation will be applied then translated to the image's center.
            Xi[i][0] = (rand() % patch_div2) * ((rand() & 1) ? 1 : -1);
            Xi[i][1] = (rand() % patch_div2) * ((rand() & 1) ? 1 : -1);
            Yi[i][0] = (rand() % patch_div2) * ((rand() & 1) ? 1 : -1);
            Yi[i][1] = (rand() % patch_div2) * ((rand() & 1) ? 1 : -1);
        }
        gen = true;
    }

    // 256bits = 32Bytes = 4 uint64
    _desc[0] = _desc[1] = _desc[2] = _desc[3] = 0;

    // FIXME: remove keypoints too close to the border
    if ((kpx - patch_div2) < 0) {
        return;
    }
    if ((kpx + patch_div2) >= imgw) {
        return;
    }
    if ((kpy - patch_div2) < 0) {
        return;
    }
    if ((kpy + patch_div2) >= imgh) {
        return;
    }

    // Applying rotation matrix to each (x, y) point in the patch gives us:
    // xr = x*cosT - y*sinT and yr = x*sinT + y*cosT

#if 0 // FIXME: my code
    for (i = 0, j = 0; i < 256; ++i, j = i & 63) {
        x = (int)round(Xi[i][0] * cosT - Yi[i][0] * sinT);
        y = (int)round(Xi[i][0] * sinT + Yi[i][0] * cosT);
        //if (x < 0 || x >= imgw) continue; // FIXME: remove keypoints too close to the border
        //if (y < 0 || y >= imgh) continue; // FIXME: remove keypoints too close to the border
        xi = center[(y * imgs) + x];

        x = (int)round(Xi[i][1] * cosT - Yi[i][1] * sinT);
        y = (int)round(Xi[i][1] * sinT + Yi[i][1] * cosT);
        //if (x < 0 || x >= imgw) continue; // FIXME: remove keypoints too close to the border
        //if (y < 0 || y >= imgh) continue; // FIXME: remove keypoints too close to the border
        yi = center[(y * imgs) + x];

        _desc[0] |= (xi < yi) ? (u64_1 << j) : 0;
        if (i != 0 && j == 0) {
            ++_desc;
        }
    }
#else // FIXME: OpenCV's code
    for (i = 0, j = 0; i < 256; ++i, j = i & 63) {
        int32_t* bp = &bit_pattern_31_[i * 4];
        x = (int)round(bp[0] * cosT - bp[1] * sinT);
        y = (int)round(bp[0] * sinT + bp[1] * cosT);
        //if (x < 0 || x >= imgw) continue; // FIXME: remove keypoints too close to the border
        //if (y < 0 || y >= imgh) continue; // FIXME: remove keypoints too close to the border
        xi = center[(y * imgs) + x];

        x = (int)round(bp[2] * cosT - bp[3] * sinT);
        y = (int)round(bp[2] * sinT + bp[3] * cosT);
        //if (x < 0 || x >= imgw) continue; // FIXME: remove keypoints too close to the border
        //if (y < 0 || y >= imgh) continue; // FIXME: remove keypoints too close to the border
        yi = center[(y * imgs) + x];

        _desc[0] |= (xi < yi) ? (u64_1 << j) : 0;
        if (i != 0 && j == 0) {
            ++_desc;
        }
    }
#endif
}

// override CompVFeatureDesc::process
COMPV_ERROR_CODE CompVFeatureDescORB::process(const CompVObjWrapper<CompVImage*>& image, const CompVObjWrapper<CompVBoxInterestPoint* >& interestPoints, CompVObjWrapper<CompVFeatureDescriptions*>* descriptions)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE || !descriptions || !interestPoints || interestPoints->empty(),
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVObjWrapper<CompVFeatureDescriptions*> _descriptions;
    CompVObjWrapper<CompVImageScalePyramid * > _pyramid;
    CompVObjWrapper<CompVImage*> imageAtLevelN;
    CompVObjWrapper<CompVFeatureDete*> attachedDete = getAttachedDete();
    uint8_t* _descriptionsPtr = NULL;

    // return COMPV_ERROR_CODE_S_OK;

    int nFeatures = (int)interestPoints->size();
    int nFeaturesBits = 256; // FIXME: depends on the patch size and brief type
    int nFeaturesBytes = nFeaturesBits >> 3;
    COMPV_CHECK_CODE_RETURN(err_ = CompVFeatureDescriptions::newObj(nFeatures, nFeaturesBits, &_descriptions));
    _descriptionsPtr = (uint8_t*)_descriptions->getDataPtr();

    // Get the pyramid from the detector or use or own pyramid
    if ((attachedDete = getAttachedDete())) {
        switch (attachedDete->getId()) {
        case COMPV_ORB_ID: {
            const void* valuePtr = NULL;
            COMPV_CHECK_CODE_RETURN(err_ = attachedDete->get(COMPV_FEATURE_GET_PTR_PYRAMID, valuePtr, sizeof(CompVImageScalePyramid)));
            _pyramid = (CompVImageScalePyramid*)(valuePtr);
            break;
        }
        }
    }
    if (!_pyramid) {
        // This code is called when we fail to get a pyramid from the attached detector or when none is attached.
        // The pyramid should come from the detector. Attach a detector to this descriptor to give it access to the pyramid.
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image));
        _pyramid = m_pyramid;
    }

    // apply gaussianblur filter on the pyramid
    for (int level = 0; level < _pyramid->getLevels(); ++level) {
        COMPV_CHECK_CODE_RETURN(err_ = _pyramid->getImage(level, &imageAtLevelN));
        //convlt2((uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), (const double*)gfilterGaussianBlur2, 7); // Gaussing blur
        convlt1((uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), (const double*)gfilterGaussianBlur1, 7);
    }

    // TODO(dmi): multi-threading
    for (size_t i = 0; i < interestPoints->size(); ++i) {
        const CompVInterestPoint* point = interestPoints->at(i);
        COMPV_CHECK_CODE_RETURN(err_ = _pyramid->getImage(point->level, &imageAtLevelN));
        // When the points were computed by the detector they have been rescaled to be in the imput image coords (level=0) -> now rescale the coords to the coresponding level
        float fkpx = (point->x * _pyramid->getScaleFactor(point->level));
        float fkpy = (point->y * _pyramid->getScaleFactor(point->level));
        float angleDeg = point->orient; // orient to the center -> like having the orientation a zero-based
        //if (angleDeg > 180) angleDeg -= 360;
        // Rotate the point
        float angleRad = (float)COMPV_MATH_DEGREE_TO_RADIAN(angleDeg);
        //if (angleRad > COMPV_MATH_PI) angleRad -= (float)(2 * COMPV_MATH_PI);
        float cosT = cos(angleRad);
        float sinT = sin(angleRad);
        // ipx/ipy are defined in cartesien coords x€(-w/2,w/2), y€(-h/2,h/2)
        int32_t ikpx = (int32_t)round(fkpx);
        int32_t ikpy = (int32_t)round(fkpy);

        //FIXME: make sure we're really using PatchSize 31
        brief256((const uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getStride(), imageAtLevelN->getWidth(), imageAtLevelN->getHeight(), ikpx, ikpy, cosT, sinT, (void*)_descriptionsPtr);
        _descriptionsPtr += nFeaturesBytes;
    }

    *descriptions = _descriptions;

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDescORB::newObj(CompVObjWrapper<CompVFeatureDesc* >* orb)
{
    COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVImageScalePyramid * > pyramid_;
    CompVObjWrapper<CompVArray<double>* > kern_;
    CompVObjWrapper<CompVConvlt* > convlt_;
    // Create Gauss kernel values
    COMPV_CHECK_CODE_RETURN(CompVGaussKern::buildKern1(&kern_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
    // Create convolution context
    COMPV_CHECK_CODE_RETURN(CompVConvlt::newObj(&convlt_));
    // Create the pyramid
    COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

    CompVObjWrapper<CompVFeatureDescORB* >_orb = new CompVFeatureDescORB();
    if (!_orb) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    _orb->m_pyramid = pyramid_;
    _orb->m_kern = kern_;
    _orb->m_convlt = convlt_;

    *orb = *_orb;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
