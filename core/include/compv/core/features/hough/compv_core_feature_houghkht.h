/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOUGHKHT_H_)
#define _COMPV_CORE_FEATURES_HOUGHKHT_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_allocators.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define KHT_TYP compv_float64_t // almost no speed gain compared to float32

struct CompVHoughKhtVote {
	size_t rho_index; size_t theta_index;
	int32_t count;
	COMPV_ALWAYS_INLINE CompVHoughKhtVote(size_t _rho_index, size_t _theta_index, int32_t _count) : rho_index(_rho_index), theta_index(_theta_index), count(_count) {}
};
typedef std::vector<CompVHoughKhtVote, CompVAllocatorNoDefaultConstruct<CompVHoughKhtVote> > CompVHoughKhtVotes;

struct CompVHoughKhtPos {
	int y; int x;
	KHT_TYP cy; KHT_TYP cx;
	COMPV_ALWAYS_INLINE CompVHoughKhtPos(int _y, int _x, const KHT_TYP edgesHeightDiv2, const KHT_TYP edgesWidthDiv2) : y(_y), x(_x) {
        cx = (x - edgesWidthDiv2);
        cy = (y - edgesHeightDiv2);
    }
};
typedef std::vector<CompVHoughKhtPos, CompVAllocatorNoDefaultConstruct<CompVHoughKhtPos> > CompVHoughKhtPoss;

struct CompVHoughKhtString {
    size_t begin;
	size_t end;
	COMPV_ALWAYS_INLINE CompVHoughKhtString(size_t _begin, size_t _end): begin(_begin), end(_end) {}
};
typedef std::vector<CompVHoughKhtString, CompVAllocatorNoDefaultConstruct<CompVHoughKhtString> > CompVHoughKhtStrings;

typedef CompVHoughKhtString CompVHoughKhtCluster;
typedef std::vector<CompVHoughKhtCluster, CompVAllocatorNoDefaultConstruct<CompVHoughKhtCluster> > CompVHoughKhtClusters;

struct CompVHoughKhtKernel {
	KHT_TYP rho; // computed using Eq3
	KHT_TYP theta; // computed using Eq3
	KHT_TYP h; // height
    
	// Matrix 'M' computed in Algorithm 2 and holding sigma values
	KHT_TYP sigma_theta_square;
	KHT_TYP sigma_rho_square;
	KHT_TYP m2;
	KHT_TYP sigma_rho_times_theta;
};
typedef std::vector<CompVHoughKhtKernel, CompVAllocatorNoDefaultConstruct<CompVHoughKhtKernel> > CompVHoughKhtKernels;

class CompVHoughKht : public CompVHough
{
protected:
	CompVHoughKht(float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);
public:
	virtual ~CompVHoughKht();
	COMPV_OBJECT_GET_ID(CompVHoughKht);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions = NULL) override /*Overrides(CompVHough)*/;
	virtual COMPV_ERROR_CODE toCartesian(const size_t imageWidth, const size_t imageHeight, const CompVHoughLineVector& polar, CompVLineFloat32Vector& cartesian) override /*Overrides(CompVHough)*/;

	static COMPV_ERROR_CODE newObj(CompVHoughPtrPtr hough, float rho = 1.f, float theta = 1.f, size_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(KHT_TYP dRho, KHT_TYP dTheta, size_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings);
	void linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtStrings& strings, const int x_ref, const int y_ref);
	COMPV_ERROR_CODE clusters_find(CompVHoughKhtClusters& clusters, CompVHoughKhtStrings::const_iterator strings_begin, CompVHoughKhtStrings::const_iterator strings_end);
	KHT_TYP clusters_subdivision(CompVHoughKhtClusters& clusters, const CompVHoughKhtString& string, const size_t start_index, const size_t end_index);
	COMPV_ERROR_CODE voting_Algorithm2_Kernels(const CompVHoughKhtClusters& clusters, CompVHoughKhtKernels& kernels, KHT_TYP& hmax);
	COMPV_ERROR_CODE voting_Algorithm2_DiscardShortKernels(CompVHoughKhtKernels& kernels, const KHT_TYP hmax);
	COMPV_ERROR_CODE voting_Algorithm2_Gmin(const CompVHoughKhtKernels& kernels, KHT_TYP &Gmin);
	COMPV_ERROR_CODE voting_Algorithm2_Count(int32_t* countsPtr, const size_t countsStride, CompVHoughKhtKernels::const_iterator kernels_begin, CompVHoughKhtKernels::const_iterator kernels_end, const KHT_TYP Gs);
	void vote_Algorithm4(int32_t* countsPtr, const size_t countsStride, size_t rho_start_index, const size_t theta_start_index, const KHT_TYP rho_start, const KHT_TYP theta_start, int inc_rho_index, const int inc_theta_index, const KHT_TYP scale, const CompVHoughKhtKernel& kernel);
	COMPV_ERROR_CODE peaks_Section3_4_VotesCountAndClearVisitedMap(CompVHoughKhtVotes& votes, const size_t theta_index_start, const size_t theta_index_end);
	COMPV_ERROR_CODE peaks_Section3_4_VotesSort(CompVHoughKhtVotes& votes);
	COMPV_ERROR_CODE peaks_Section3_4_Lines(CompVHoughLineVector& lines, const CompVHoughKhtVotes& votes);

private:
	KHT_TYP m_dRho;
	KHT_TYP m_dTheta_rad;
	KHT_TYP m_dTheta_deg;
	KHT_TYP m_cluster_min_deviation;
	size_t m_cluster_min_size;
	KHT_TYP m_kernel_min_heigth;
	KHT_TYP m_dGS;
	size_t m_nThreshold;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nMaxLines;
	bool m_bOverrideInputEdges;
	CompVMatPtr m_edges;
	CompVMatPtr m_rho; // CompVMatPtr<KHT_TYP>
	CompVMatPtr m_theta; // CompVMatPtr<KHT_TYP>
	CompVMatPtr m_count; // CompVMatPtr<int32_t>
	CompVMatPtr m_visited; // CompVMatPtr<uint8_t>
	CompVHoughKhtPoss m_poss;
	CompVHoughKhtStrings m_strings;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOUGHKHT_H_ */
