/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/compv_core_feature_houghkht.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/parallel/compv_parallel.h"

#include <algorithm> /* std::reverse */

#define COMPV_THIS_CLASSNAME	"CompVHoughKht"

// Documentation:
//	- http://www2.ic.uff.br/~laffernandes/projects/kht/
//	- https://www.academia.edu/11637890/Real-time_line_detection_through_an_improved_Hough_transform_voting_scheme
//	- https://en.wikipedia.org/wiki/Hough_transform#Kernel-based_Hough_transform_.28KHT.29
//	- http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5388&rep=rep1&type=pdf

COMPV_NAMESPACE_BEGIN()

#define COMPV_HOUGHKHT_CLUSTER_MIN_DEVIATION		2.0
#define COMPV_HOUGHKHT_CLUSTER_MIN_SIZE				10
#define COMPV_HOUGHKHT_KERNEL_MIN_HEIGTH			0.002

CompVHoughKht::CompVHoughKht(float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
	:CompVHough(COMPV_HOUGHKHT_ID)
	, m_dRho(static_cast<double>(rho))
	, m_dTheta_rad(static_cast<double>(theta))
	, m_cluster_min_deviation(COMPV_HOUGHKHT_CLUSTER_MIN_DEVIATION)
	, m_cluster_min_size(COMPV_HOUGHKHT_CLUSTER_MIN_SIZE)
	, m_kernel_min_heigth(COMPV_HOUGHKHT_KERNEL_MIN_HEIGTH)
	, m_nThreshold(threshold)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nMaxLines(INT_MAX)
{

}

CompVHoughKht:: ~CompVHoughKht()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVHoughKht::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOUGH_SET_FLT32_RHO: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f || *reinterpret_cast<const compv_float32_t*>(valuePtr) > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fRho = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(static_cast<double>(fRho), m_dTheta_rad, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_FLT32_THETA: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fTheta = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, static_cast<double>(fTheta), m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nThreshold = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, m_dTheta_rad, static_cast<size_t>(nThreshold), m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_MAXLINES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nMaxLines = static_cast<size_t>(reinterpret_cast<const int*>(valuePtr) <= 0 ? INT_MAX : *reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_cluster_min_deviation = static_cast<double>(*reinterpret_cast<const compv_float32_t*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_cluster_min_size = static_cast<size_t>(*reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) < 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_kernel_min_heigth = static_cast<double>(*reinterpret_cast<const compv_float32_t*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	default: {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVHoughKht::process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions COMPV_DEFAULT(NULL)) /*Overrides(CompVHough)*/
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty() || edges->subType() != COMPV_SUBTYPE_PIXELS_Y, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Edges null or not grayscale");

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

	// Clone the edges (the linking procedure modifify the data)
	COMPV_CHECK_CODE_RETURN(edges->clone(&m_edges));

	// Init coords (sine and cosine tables)
	COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, m_dTheta_rad, m_nThreshold, edges->cols(), edges->rows()));

	// Appendix A. Linking procedure
	COMPV_CHECK_CODE_RETURN(linking_AppendixA(m_edges, m_strings));

	// Clusters
	COMPV_CHECK_CODE_RETURN(clusters_find(m_clusters, m_strings));

	// Voting
	COMPV_CHECK_CODE_RETURN(m_count->zero_all()); // reset (TODO(dmi): hide latency using MT and packing in previous functions)
	COMPV_CHECK_CODE_RETURN(voting_Algorithm2(m_clusters));

	// Peaks detection
	COMPV_CHECK_CODE_RETURN(m_visited->zero_all()); // reset (TODO(dmi): hide latency using MT and packing in previous functions)
	COMPV_CHECK_CODE_RETURN(peaks_Section3_4(lines));

	// Retain best lines
	const size_t maxLines = COMPV_MATH_MIN(lines.size(), m_nMaxLines);
	lines.resize(maxLines); // already sorted in 'peaks_Section3_4'

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::newObj(CompVHoughPtrPtr hough, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_EXP_RETURN(!hough || rho <= 0 || rho > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVHoughPtr hough_ = new CompVHoughKht(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::initCoords(double dRho, double dTheta, size_t nThreshold, size_t nWidth COMPV_DEFAULT(0), size_t nHeight COMPV_DEFAULT(0))
{
	nWidth = !nWidth ? m_nWidth : nWidth;
	nHeight = !nHeight ? m_nHeight : nHeight;
	if (m_dRho != dRho || m_dTheta_rad != dTheta || m_nWidth != nWidth || m_nHeight != nHeight) {
		double r0, *ptr0;
		const double dTheta_deg = COMPV_MATH_RADIAN_TO_DEGREE(m_dTheta_rad);
		double r = std::sqrt(static_cast<double>((nWidth * nWidth) + (nHeight * nHeight)));
		const size_t maxRhoCount = static_cast<size_t>(((r + 1.0) / dRho));
		const size_t maxThetaCount = static_cast<size_t>(180.0 / dTheta_deg);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&m_rho, 1, maxRhoCount));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&m_theta, 1, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&m_count, (maxThetaCount + 2), (maxRhoCount + 2)));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&m_visited, (maxThetaCount + 2), (maxRhoCount + 2)));
		
		// rho filling
		r0 = (r * -0.5);
		ptr0 = m_rho->ptr<double>();
		for (size_t i = 1; i < maxRhoCount; ++i, r0 += dRho) {
			ptr0[i] = r0;
		}

		// theta filling
		r0 = 0.0;
		ptr0 = m_theta->ptr<double>();
		for (size_t i = 1; i < maxThetaCount; ++i, r0 += dTheta_deg) {
			ptr0[i] = r0;
		}

		m_dRho = dRho;
		m_dTheta_rad = dTheta;
		m_dTheta_deg = dTheta_deg;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}
	m_nThreshold = nThreshold;
	return COMPV_ERROR_CODE_S_OK;
}

// Appendix A. Linking procedure
COMPV_ERROR_CODE CompVHoughKht::linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings)
{
	uint8_t* edgesPtr = edges->ptr<uint8_t>(1);
	const size_t edgesWidth = edges->cols();
	const size_t edgesHeight = edges->rows();
	const size_t edgesStride = edges->strideInBytes();
	const int maxi = static_cast<int>(edges->cols() - 1);
	const int maxj = static_cast<int>(edges->rows() - 1);

	CompVHoughKhtPosBoxPtr tmp_box; // box used as temporary container
	COMPV_CHECK_CODE_ASSERT(CompVHoughKhtPosBox::newObj(&tmp_box, 1000));

	strings.clear();

	for (int y_ref = 1; y_ref < maxj; ++y_ref) {
		for (int x_ref = 1; x_ref < maxi; ++x_ref) {
			if (edgesPtr[x_ref]) {
				linking_link_Algorithm5(&edgesPtr[x_ref], edgesWidth, edgesHeight, edgesStride, tmp_box, strings, x_ref, y_ref);
			}
		}
		edgesPtr += edges->strideInBytes();
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Algorithm 5 - Linking of neighboring edge pixels into strings
void CompVHoughKht::linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtPosBoxPtr& tmp_box, CompVHoughKhtStrings& strings, const int x_ref, const int y_ref)
{	
	CompVHoughKhtPos *new_pos;
	size_t reverse_size;

	tmp_box->reset();

	// {Find and add feature pixels to the end of the string}
	int x_seed = x_ref;
	int y_seed = y_ref;
	uint8_t* next_seed = edgesPtr;
	do {
		tmp_box->new_item(&new_pos);
		new_pos->x = x_seed, new_pos->y = y_seed;
		*next_seed = 0x00; // !! edges are modified here (not thread-safe) !!
	} while((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed)));

	reverse_size = tmp_box->size();

	// {Find and add feature pixels to the begin of the string}
	x_seed = x_ref;
	y_seed = y_ref;
	next_seed = edgesPtr;
	if ((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed))) {
		do {
			tmp_box->new_item(&new_pos);
			new_pos->x = x_seed, new_pos->y = y_seed;
			*next_seed = 0x00; // !! edges are modified here (not thread-safe) !!
		} while ((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed)));
	}

	if (tmp_box->size() >= m_cluster_min_size) {
		const double edgesWidthDiv2 = static_cast<double>(edgesWidth) * 0.5;
		const double edgesHeightDiv2 = static_cast<double>(edgesHeight) * 0.5;
		CompVHoughKhtString string;
		string.assign(tmp_box->begin(), tmp_box->end());
		if (reverse_size) {
			std::reverse(string.begin(), string.begin() + reverse_size);
		}
		std::for_each(string.begin(), string.end(), [edgesWidthDiv2, edgesHeightDiv2](CompVHoughKhtPos& pos) {
			pos.cx = (pos.x - edgesWidthDiv2);
			pos.cy = (pos.y - edgesHeightDiv2);
		});
		strings.push_back(string);
	}
}

// Algorithm 6 - Function Next(). It complements the linking procedure(Algorithm 5).
uint8_t* CompVHoughKht::linking_next_Algorithm6(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, int &x_seed, int &y_seed)
{
	const bool left_avail = x_seed > 0;
	const bool right_avail = (x_seed + 1) < static_cast<int>(edgesWidth);

	/* == top == */
	if (y_seed > 0) {
		uint8_t* top = edgesPtr - edgesStride;
		if (left_avail && top[-1]) { // top-left
			--x_seed, --y_seed; return &top[-1];
		}
		if (*top) { // top-center
			--y_seed; return top;
		}
		if (right_avail && top[1]) { // top-right
			++x_seed, --y_seed; return &top[1];
		}
	}
	
	/* == center == */
	if (left_avail && edgesPtr[-1]) { // center-left
		--x_seed; return &edgesPtr[-1];
	}
	if (right_avail && edgesPtr[1]) { // center-right
		++x_seed; return &edgesPtr[1];
	}

	/* == bottom == */
	if ((y_seed + 1) < static_cast<int>(edgesHeight)) {
		uint8_t* bottom = edgesPtr + edgesStride;
		if (left_avail && bottom[-1]) { // bottom-left
			--x_seed, ++y_seed;	return &bottom[-1];
		}
		if (*bottom) { // bottom-center
			++y_seed; return bottom;
		}
		if (right_avail && bottom[1]) { // bottom-right
			++x_seed, ++y_seed; return &bottom[1];
		}
	}
	return NULL;
}

COMPV_ERROR_CODE CompVHoughKht::clusters_find(CompVHoughKhtClusters& clusters, const CompVHoughKhtStrings& strings)
{
	clusters.clear();
	for (CompVHoughKhtStrings::const_iterator it = strings.begin(); it < strings.end(); ++it) {
		clusters_subdivision(clusters, *it, 0, it->size() - 1);
	}
	return COMPV_ERROR_CODE_S_OK;
}

// (i) cluster approximately collinear feature pixels
double CompVHoughKht::clusters_subdivision(CompVHoughKhtClusters& clusters, const CompVHoughKhtString& string, const size_t start_index, const size_t end_index)
{
	// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5388&rep=rep1&type=pdf
	//  Three-Dimensional Object Recognition from Single Two - Dimensional Images
	// Section 4.6 - Segmentation of linked points into straight line segments

	const size_t num_clusters_without_subs = clusters.size();

	const CompVHoughKhtPos &start = string[start_index];
	const CompVHoughKhtPos &end = string[end_index];
	const int diffx = start.x - end.x;
	const int diffy = start.y - end.y;
	const double length = std::sqrt(static_cast<double>((diffx * diffx) + (diffy * diffy)));
	const size_t count = string.size();

	// The significance of a straight line fit to a list of points can be estimated by calculating
	// 	the ratio of the length of the line segment divided by the maximum deviation of
	//	any point from the line(the maximum deviation is always assumed to be at least two
	//		pixels in size to account for limitations on measurement accuracy)
	
	size_t max_index = start_index;
	double deviation;
	double max_deviation = std::abs(static_cast<double>((((start.x - string[start_index].x) * diffy) - ((start.y - string[start_index].y) * diffx))));
	for (size_t i = start_index + 1; i < end_index; ++i) {
		const CompVHoughKhtPos &current = string[i];
		deviation = std::abs(static_cast<double>((((start.x - current.x) * diffy) - ((start.y - current.y) * diffx))));
		if (deviation > max_deviation) {
			max_index = i;
			max_deviation = deviation;
		}
	}	
	const double ratio = length / std::max((max_deviation / length), m_cluster_min_deviation);

	// A segment is recursively subdivided at the
	// point with maximum deviation from a line connecting its endpoints(Figure 7 (b, c)).
	
	// This process is repeated until each segment is no more than 4 pixels in length, producing
	// a binary tree of possible subdivisions.

	//	If the maximum
	//	significance of any of the subsegments is greater than the significance of the complete
	//	segment, then the subsegments are returned.Otherwise the single segment is returned.

	const bool left_have_more_than_minsize = ((max_index - start_index + 1) >= m_cluster_min_size);
	if (left_have_more_than_minsize) {
		const bool right_have_more_than_minsize = ((end_index - max_index + 1) >= m_cluster_min_size);
		if (right_have_more_than_minsize) {
			const double ratio_left = clusters_subdivision(clusters, string, start_index, max_index);
			const double ratio_right = clusters_subdivision(clusters, string, max_index, end_index);
			if ((ratio_left > ratio) || (ratio_right > ratio)) {
				return std::max(ratio_left, ratio_right);
			}
		}
	}
	
	// remove sub-clusters
	clusters.resize(num_clusters_without_subs);
	// push current cluster
	clusters.push_back(CompVHoughKhtCluster((string.begin() + start_index), (string.begin() + end_index + 1)));

	return ratio;
}

static double __gauss_Eq15(const double rho, const double theta, const CompVHoughKhtKernel& kernel)
{
	const double sigma_theta_square = kernel.M[kCompVHoughKhtKernelIndex_SigmaThetaSquare];
	const double sigma_rho_square = kernel.M[kCompVHoughKhtKernelIndex_SigmaRhoSquare];
	const double sigma_rho_times_theta = kernel.M[kCompVHoughKhtKernelIndex_SigmaRhoTimesTheta];
	const double sigma_rho_times_sigma_theta = std::sqrt(sigma_rho_square) * std::sqrt(sigma_theta_square);
	const double r = (sigma_rho_times_theta / sigma_rho_times_sigma_theta);
	const double r_square = r * r;
	const double x = 1.0 / (2.0 * COMPV_MATH_PI * sigma_rho_times_sigma_theta * std::sqrt(1.0 - r_square));
	const double y = 1.0 / (2.0 * (1.0 - r_square));
	const double z = ((rho * rho) / sigma_rho_square) - (((r * 2.0) * rho * theta) / sigma_rho_times_sigma_theta) + ((theta * theta) / sigma_theta_square);
	return x * std::exp(-z * y);
}

// Algorithm 2: Computation of the Gaussian kernel parameters
COMPV_ERROR_CODE CompVHoughKht::voting_Algorithm2(const CompVHoughKhtClusters& clusters)
{
	if (!clusters.empty()) {
		double mean_cx, mean_cy, cx, cy, cxx, cyy, cxy;
		double n; // number of pixels in Sk
		double ux, uy; // eigenvector in V for the biggest eigenvalue
		double vx, vy; // eigenvector in V for the smaller eigenvalue
		double sqrt_one_minus_vx2; // sqrt(1 - (vx^2)) - Eq14
		double matrix[2 * 2];
		double eigenVectors[2 * 2], eigenValues[2 * 2], M_Eq14[2 * 2] = {0.0, 1.0, 0.0, 0.0};
		double r0, r1, r2;
		size_t rho_index, theta_index;
		static const double rad_to_deg_scale = (180.0 / COMPV_MATH_PI);
		const double rho_scale = 1.0 / m_dRho;
		const double theta_scale = 1.0 / m_dTheta_deg;
		const double rho_max_neg = *m_rho->ptr<const double>(0, 1);

		CompVHoughKhtKernels kernels;
		kernels.resize(clusters.size());
		CompVHoughKhtKernels::iterator kernel = kernels.begin();

		// for each group of pixels Sk
		for (CompVHoughKhtClusters::const_iterator cluster = clusters.begin(); cluster < clusters.end(); ++cluster, ++kernel) {
			/* {Alternative reference system definition} */
			// computing the centroid
			mean_cx = mean_cy = 0;
			n = static_cast<double>((cluster->end - cluster->begin));
			for (CompVHoughKhtString::const_iterator p = cluster->begin; p < cluster->end; ++p) {
				mean_cx += p->cx;
				mean_cy += p->cy;
			}
			mean_cx /= n; // p_hat.x
			mean_cy /= n; // p_hat.y

			/* {Eigen-decomposition} */
			cxx = cyy = cxy = 0.0;
			for (CompVHoughKhtString::const_iterator p = cluster->begin; p < cluster->end; ++p) {
				cx = (p->cx - mean_cx); // (p.x - p_hat.x)
				cy = (p->cy - mean_cy); // (p.y - p_hat.y)
				cxx += (cx * cx);
				cyy += (cy * cy);
				cxy += (cx * cy);
			}
			matrix[0] = cxx, matrix[1] = cxy, matrix[2] = cxy, matrix[3] = cyy;		
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<double>::find2x2(matrix, eigenValues, eigenVectors)); // Eigen values and vectors are already sorted (highest to lowest)
			ux = eigenVectors[0], uy = eigenVectors[2]; // eigenvector in V for the biggest eigenvalue (first eigen vector)
			vx = eigenVectors[1], vy = eigenVectors[3]; // eigenvector in V for the smaller eigenvalue (second eigen vector)

			/* {yv < 0 condition verification} */
			if (vy < 0.0) {
				vx = -vx, vy = -vy;
			}

			/* {Normal equation parameters computation(Eq3)} */
			kernel->rho = (vx * mean_cx) + (vy * mean_cy);
			kernel->theta = std::acos(vx) * rad_to_deg_scale;

			/* {substituting Eq5 in Eq10} */
			// Eq.14
			sqrt_one_minus_vx2 = std::sqrt(1.0 - (vx * vx));
			M_Eq14[0] = -(ux * mean_cx) - (uy * mean_cy);
			M_Eq14[2] = (sqrt_one_minus_vx2 == 0.0) ? 0.0 : ((ux / sqrt_one_minus_vx2) * rad_to_deg_scale);
			// Compute M
			r0 = 0.0;
			for (CompVHoughKhtString::const_iterator p = cluster->begin; p < cluster->end; ++p) {
				r1 = (ux * (p->cx - mean_cx)) + (uy * (p->cy - mean_cy));
				r0 += (r1 * r1);
			}
			r0 = 1.0 / r0;
			r1 = (M_Eq14[0] * r0);
			r2 = (M_Eq14[2] * r0);
			kernel->M[kCompVHoughKhtKernelIndex_SigmaRhoSquare] = r1 * M_Eq14[0] + (1.0 / n);
			kernel->M[kCompVHoughKhtKernelIndex_SigmaRhoTimesTheta] = r1 * M_Eq14[2];
			kernel->M[kCompVHoughKhtKernelIndex_2] = r2 * M_Eq14[0];
			kernel->M[kCompVHoughKhtKernelIndex_SigmaThetaSquare] = r2 * M_Eq14[2];
			// line 22
			if (kernel->M[kCompVHoughKhtKernelIndex_SigmaThetaSquare] == 0.0) {
				kernel->M[kCompVHoughKhtKernelIndex_SigmaThetaSquare] = 0.1; // (2^2 * 0.1)
			}
			kernel->M[kCompVHoughKhtKernelIndex_SigmaRhoSquare] *= 4.0; // * (2^2)
			kernel->M[kCompVHoughKhtKernelIndex_SigmaThetaSquare] *= 4.0; // * (2^2)

			// Kernel's height
			kernel->h = __gauss_Eq15(0.0, 0.0, *kernel);
		} // end-of-for (CompVHoughKhtClusters::const_iterator cluster

		// TODO(dmi): Algorithm3 not thread-safe (because of hmax computing), split it in new function

		/* Algorithm 3. Proposed voting process.The Vote() procedure is in Algorithm 4. */

		/* {Discard groups with very short kernels} */
		r0 = kernels.begin()->h; // r0 = hmax
		for (CompVHoughKhtKernels::const_iterator k = (kernels.begin() + 1); k < kernels.end(); ++k) {
			if (k->h > r0) {
				r0 = k->h;
			}
		}
		r1 = 1.0 / r0; // r1 = (1/hmax) to avoid dividing by hmax in the loop		
		auto fncShortKernels = std::remove_if(kernels.begin(), kernels.end(), [this, r1](const CompVHoughKhtKernel& k) {
			return (k.h * r1) < m_kernel_min_heigth;
		});
		kernels.erase(fncShortKernels, kernels.end());

		if (!kernels.empty()) {
			/* {Find the gmin threshold. Gk function in Eq15} */
			r0 = DBL_MAX; // r0 = Gmin // http://www2.ic.uff.br/~laffernandes/projects/kht/ -> Errata - 01/14/2008
			for (CompVHoughKhtKernels::const_iterator k = kernels.begin(); k < kernels.end(); ++k) {
				COMPV_CHECK_CODE_RETURN(CompVMathEigen<double>::find2x2(k->M, eigenValues, eigenVectors));
				// Compute Gk (gauss function)
				r1 = std::sqrt(eigenValues[3]); // sqrt(smallest eigenvalue -> lambda_w)
				r2 = __gauss_Eq15(eigenVectors[1] * r1, eigenVectors[3] * r1, *k);
				if (r2 < r0) {
					r0 = r2;
				}
			}

			/* {Scale factor for integer votes} */
			r1 = r0 == 0.0 ? 1.0 : std::max((1.0 / r0), 1.0); // r1 = gs
			
			/* {Vote for each selected kernel} */
			for (CompVHoughKhtKernels::const_iterator k = kernels.begin(); k < kernels.end(); ++k) {
				rho_index = static_cast<size_t>(std::abs((k->rho - rho_max_neg) * rho_scale)) + 1;
				theta_index = static_cast<size_t>(std::abs(k->theta * theta_scale)) + 1;
				vote_Algorithm4(rho_index, theta_index, 0.0, 0.0, 1, 1, r1, *k);
				vote_Algorithm4(rho_index, theta_index - 1, 0.0, -m_dTheta_deg, 1, -1, r1, *k);
				vote_Algorithm4(rho_index - 1, theta_index, -m_dRho, 0.0, -1, 1, r1, *k);
				vote_Algorithm4(rho_index - 1, theta_index - 1, -m_dRho, -m_dTheta_deg, -1, -1, r1, *k);
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

void CompVHoughKht::vote_Algorithm4(size_t rho_start_index, const size_t theta_start_index, const double rho_start, const double theta_start, int inc_rho_index, const int inc_theta_index, const double scale, const CompVHoughKhtKernel& kernel)
{
	int32_t* pcount;
	const size_t rho_size = m_rho->cols(), theta_size = m_theta->cols();
	const double inc_rho = m_dRho * inc_rho_index;
	const double inc_theta = m_dTheta_deg * inc_theta_index;
	const double sigma_rho_square = kernel.M[kCompVHoughKhtKernelIndex_SigmaRhoSquare];
	const double sigma_theta_square = kernel.M[kCompVHoughKhtKernelIndex_SigmaThetaSquare];
	const double sigma_rho_times_theta = kernel.M[kCompVHoughKhtKernelIndex_SigmaRhoTimesTheta];
	const double sigma_rho_times_sigma_theta = std::sqrt(sigma_rho_square) * std::sqrt(sigma_theta_square);
	const double r = (sigma_rho_times_theta / sigma_rho_times_sigma_theta);
	const double r_times_2 = r * 2.0;
	const double x = 1.0 / (2.0 * COMPV_MATH_PI * sigma_rho_times_sigma_theta * std::sqrt(1.0 - (r * r)));
	const double y = 1.0 / (2.0 * (1.0 - (r * r)));

	double rho, theta;
	double z, w;
	int votes;
	size_t rho_index, theta_index, theta_count;

	/* {Loop for the  coordinates of the parameter space} */
	theta_index = theta_start_index;
	theta = theta_start;
	theta_count = 0;
	do {
		/* {Test if the kernel exceeds the parameter space limits} */
		if (!theta_index || theta_index > theta_size) {
			rho_start_index = (rho_size - rho_start_index) + 1;
			theta_index = theta_index ? 1 : theta_size;
			inc_rho_index = -inc_rho_index;
		}

		if (rho_start_index >= 1) {
			/* {Loop for the  coordinates of the parameter space} */
			pcount = m_count->ptr<int32_t>(theta_index);
			rho_index = rho_start_index;
			rho = rho_start;
			w = ((theta * theta) / sigma_theta_square);
			z = ((rho * rho) / sigma_rho_square) - ((r_times_2 * rho * theta) / sigma_rho_times_sigma_theta) + w;
			while (((rho_index <= rho_size) && (votes = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((x * std::exp(-z * y)) * scale), int)) > 0)) {
				pcount[rho_index] += votes;
				rho_index += inc_rho_index;
				rho += inc_rho;
				z = ((rho * rho) / sigma_rho_square) - ((r_times_2 * rho * theta) / sigma_rho_times_sigma_theta) + w;
			}
			theta_index += inc_theta_index;
			theta += inc_theta;
		}
		else break;
	} while ((rho != rho_start) && (++theta_count < theta_size));
}

COMPV_ERROR_CODE CompVHoughKht::peaks_Section3_4(CompVHoughLineVector& lines)
{
	lines.clear();

	bool bvisited;
	uint8_t* pvisited;
	const uint8_t *pvisited_top, *pvisited_bottom;
	const size_t pvisited_stride = m_visited->strideInBytes();
	const size_t pcount_stride = m_count->stride();
	const int32_t *pcount = m_count->ptr<const int32_t>(1), *pcount_top, *pcount_bottom;
	const double *rho = m_rho->ptr<const double>();
	const double *theta = m_theta->ptr<const double>();
	const size_t theta_end = m_theta->cols();
	const size_t rho_end = m_rho->cols();
	size_t theta_index, rho_index;

	CompVHoughKhtVotes votes;

	// Given a voting map, first we create a list with all cells that
	//	receive at least one vote.

	for (theta_index = 1; theta_index < theta_end; ++theta_index) {
		for (rho_index = 1; rho_index < rho_end; ++rho_index) {
			if (pcount[rho_index]) {
				pcount_top = (pcount - pcount_stride);
				pcount_bottom = (pcount + pcount_stride);
				votes.push_back(CompVHoughKhtVote(rho_index,
					theta_index,
					// convolution of the cells with a 3 × 3 Gaussian kernel.
					pcount_top[rho_index - 1] + pcount_top[rho_index + 1] + pcount_top[rho_index] + pcount_top[rho_index]
					+ pcount_bottom[rho_index - 1] + pcount_bottom[rho_index + 1] + pcount_bottom[rho_index] + pcount_bottom[rho_index]
					+ pcount[rho_index - 1] + pcount[rho_index - 1] + pcount[rho_index + 1] + pcount[rho_index + 1] + pcount[rho_index] + pcount[rho_index] + pcount[rho_index] + pcount[rho_index]));
			}
		}
		pcount += pcount_stride;
	}

	// Remove all votes with less than threshold's values in count
	auto fncShortVotes = std::remove_if(votes.begin(), votes.end(), [this](const CompVHoughKhtVote& v) {
		return v.count < m_nThreshold;
	});
	votes.erase(fncShortVotes, votes.end());

	// Then, this list is sorted in descending
	//	order according to the result of the convolution of the cells with
	//	a 3 × 3 Gaussian kernel.
	std::sort(votes.begin(), votes.end(), [](const CompVHoughKhtVote &vote1, const CompVHoughKhtVote &vote2) {
		return (vote1.count > vote2.count);
	});

	// After the sorting step, we use a sweep plane that visits each
	// cell of the list.
	for (CompVHoughKhtVotes::const_iterator i = votes.begin(); i < votes.end(); ++i) {
		pvisited = m_visited->ptr<uint8_t>(i->theta_index, i->rho_index);
		pvisited_top = (pvisited - pvisited_stride);
		pvisited_bottom = (pvisited + pvisited_stride);
		bvisited = pvisited_top[-1] || pvisited_top[0] || pvisited_top[1] ||
			pvisited[-1] || pvisited[1] ||
			pvisited_bottom[-1] || pvisited_bottom[0] || pvisited_bottom[1];
		if (!bvisited) {
			lines.push_back(CompVHoughLine(
				static_cast<compv_float32_t>(rho[i->rho_index]),
				static_cast<compv_float32_t>(COMPV_MATH_DEGREE_TO_RADIAN(theta[i->theta_index])),
				i->count));
		}
		*pvisited = 0xff;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
