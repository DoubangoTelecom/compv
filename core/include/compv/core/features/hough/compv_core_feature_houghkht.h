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
#include "compv/base/compv_features.h"
#include "compv/base/compv_box.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVHoughKhtVote {
	size_t rho_index; size_t theta_index;
	int32_t count;
	CompVHoughKhtVote() : rho_index(0), theta_index(0), count(0) {}
	CompVHoughKhtVote(size_t _rho_index, size_t _theta_index, int32_t _count) : rho_index(_rho_index), theta_index(_theta_index), count(_count) {}
};
typedef std::vector<CompVHoughKhtVote> CompVHoughKhtVotes;

struct CompVHoughKhtPos {
	int y; int x;
	double cy; double cx;
	CompVHoughKhtPos() : y(0), x(0){}
	CompVHoughKhtPos(int _y, int _x) : y(_y), x(_x) {}
};
typedef std::vector<CompVHoughKhtPos> CompVHoughKhtString;
typedef std::vector<CompVHoughKhtString> CompVHoughKhtStrings;

typedef CompVBox<CompVHoughKhtPos> CompVHoughKhtPosBox;
typedef CompVPtr<CompVHoughKhtPosBox* > CompVHoughKhtPosBoxPtr;

struct CompVHoughKhtCluster {
	CompVHoughKhtCluster(CompVHoughKhtString::const_iterator _begin, CompVHoughKhtString::const_iterator _end) :
		begin (_begin), end(_end) { }
	CompVHoughKhtCluster() { }
	CompVHoughKhtString::const_iterator begin;
	CompVHoughKhtString::const_iterator end;
};
typedef std::vector<CompVHoughKhtCluster> CompVHoughKhtClusters;

struct CompVHoughKhtKernel {
	double rho; // computed using Eq3
	double theta; // computed using Eq3
	double h; // height
#define kCompVHoughKhtKernelIndex_SigmaRhoSquare			0
#define kCompVHoughKhtKernelIndex_SigmaRhoTimesTheta		1
#define kCompVHoughKhtKernelIndex_2							2
#define kCompVHoughKhtKernelIndex_SigmaThetaSquare			3

	double M[4]; // Matrix 'M' computed in Algorithm 2 and holding sigma values
};
typedef std::vector<CompVHoughKhtKernel> CompVHoughKhtKernels;

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

	static COMPV_ERROR_CODE newObj(CompVHoughPtrPtr hough, float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(double dRho, double dTheta, size_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings);
	void linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtPosBoxPtr& tmp_box, CompVHoughKhtStrings& strings, const int x_ref, const int y_ref);
	uint8_t* linking_next_Algorithm6(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, int &x_seed, int &y_seed);
	COMPV_ERROR_CODE clusters_find(CompVHoughKhtClusters& clusters, CompVHoughKhtStrings::const_iterator strings_begin, CompVHoughKhtStrings::const_iterator strings_end);
	double clusters_subdivision(CompVHoughKhtClusters& clusters, const CompVHoughKhtString& string, const size_t start_index, const size_t end_index);
	COMPV_ERROR_CODE voting_Algorithm2_Kernels(const CompVHoughKhtClusters& clusters, CompVHoughKhtKernels& kernels, double& hmax);
	COMPV_ERROR_CODE voting_Algorithm2_DiscardShortKernels(CompVHoughKhtKernels& kernels, const double hmax);
	COMPV_ERROR_CODE voting_Algorithm2_Gmin(const CompVHoughKhtKernels& kernels, double &Gmin);
	COMPV_ERROR_CODE voting_Algorithm2_Count(int32_t* countsPtr, const size_t countsStride, CompVHoughKhtKernels::const_iterator kernels_begin, CompVHoughKhtKernels::const_iterator kernels_end, const double Gs);
	void vote_Algorithm4(int32_t* countsPtr, const size_t countsStride, size_t rho_start_index, const size_t theta_start_index, const double rho_start, const double theta_start, int inc_rho_index, const int inc_theta_index, const double scale, const CompVHoughKhtKernel& kernel);
	COMPV_ERROR_CODE peaks_Section3_4_VotesCountAndClearVisitedMap(CompVHoughKhtVotes& votes, const size_t theta_index_start, const size_t theta_index_end);
	COMPV_ERROR_CODE peaks_Section3_4_VotesSort(CompVHoughKhtVotes& votes);
	COMPV_ERROR_CODE peaks_Section3_4_Lines(CompVHoughLineVector& lines, const CompVHoughKhtVotes& votes);

private:
	double m_dRho;
	double m_dTheta_rad;
	double m_dTheta_deg;
	double m_cluster_min_deviation;
	size_t m_cluster_min_size;
	double m_kernel_min_heigth;
	double m_dGS;
	size_t m_nThreshold;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nMaxLines;
	bool m_bOverrideInputEdges;
	CompVMatPtr m_edges;
	CompVMatPtr m_rho; // CompVMatPtr<double>
	CompVMatPtr m_theta; // CompVMatPtr<double>
	CompVMatPtr m_count; // CompVMatPtr<int32_t>
	CompVMatPtr m_visited; // CompVMatPtr<uint8_t>
	CompVHoughKhtStrings m_strings;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOUGHKHT_H_ */
