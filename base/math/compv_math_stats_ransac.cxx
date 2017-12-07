/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_stats_ransac.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include <random>

#define COMPV_THIS_CLASSNAME	"CompVMathStatsRansac"

COMPV_NAMESPACE_BEGIN()


//
//	CompVMathStatsRansacGeneric
//

class CompVMathStatsRansacGeneric
{
	template <typename FloatType>
	struct CompVMathStatsRansacGenericThreadData {
		size_t numThreads = 1;
		size_t bestNumInliers = 0;
		size_t numIter = 0;
		size_t maxIter = 0;
		CompVMathStatsRansacModelParamsFloatType modelParamsBest;
		const CompVMathStatsRansacControl<FloatType>* control;
		const CompVMathStatsRansacStatus<FloatType>* status;
		COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject) = nullptr;
		COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, CompVMatPtr residual, bool& userbreak) = nullptr;
	};
public:
	template <typename FloatType>
	static COMPV_ERROR_CODE process(
		const CompVMathStatsRansacControl<FloatType>* control, CompVMathStatsRansacStatus<FloatType>* status,
		COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject),
		COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, CompVMatPtr residual, bool& userbreak)
	)
	{
		COMPV_CHECK_EXP_RETURN(!control || !status || !control->isValid() || !buildModelParams || !buildResiduals,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		if (control->minModelPoints > 3) { // Reminder for Homography (4 model points)
			COMPV_DEBUG_INFO_CODE_TODO("Add TieBreaker"); // TieBreaker callback function: when number of inliers are equal then, compare variances
		}

		/* Reset status */
		status->reset();

		/* Compute number of threads */
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;		
		const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? maxThreads : 1;

		/* Processing */
		bool anotherThreadFoundInliers = false;
		CompVMathStatsRansacGenericThreadData<FloatType> bestThreadData;
		COMPV_DEBUG_INFO_CODE_TODO("MT disabled");
		if (threadsCount > 1) {
			CompVAsyncTaskIds taskIds;
			std::vector<CompVMathStatsRansacGenericThreadData<FloatType> > vecThreadData(threadsCount);
			taskIds.reserve(threadsCount);
			auto funcPtr = [&](const size_t threadIdx) -> COMPV_ERROR_CODE {
				CompVMathStatsRansacGenericThreadData<FloatType>& threadData = vecThreadData[threadIdx];
				threadData.control = control;
				threadData.status = status;
				threadData.buildModelParams = buildModelParams;
				threadData.buildResiduals = buildResiduals;
				threadData.numThreads = threadsCount;
				COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process_thread<FloatType>(threadData, anotherThreadFoundInliers));
				return COMPV_ERROR_CODE_S_OK;
			};
			for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, threadIdx), taskIds), "Dispatching task failed");
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
			size_t indexOfBest = 0, bestNumInliers = 0, numIter = 0, maxIter = 0;
			for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
				const CompVMathStatsRansacGenericThreadData<FloatType>& threadData = vecThreadData[threadIdx];
				numIter += threadData.numIter;
				maxIter += threadData.maxIter;
				if (threadData.bestNumInliers > bestNumInliers) { // TODO(dmi): run TieBreaker when equal
					bestNumInliers = threadData.bestNumInliers;
					indexOfBest = threadIdx;
				}
			}
			bestThreadData = vecThreadData[indexOfBest];
			bestThreadData.numIter = numIter;
			bestThreadData.maxIter = maxIter;
		}
		else {
			bestThreadData.control = control;
			bestThreadData.status = status;
			bestThreadData.buildModelParams = buildModelParams;
			bestThreadData.buildResiduals = buildResiduals;
			COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process_thread<FloatType>(bestThreadData, anotherThreadFoundInliers));
		}		

		/* Update status */
		status->numInliers = bestThreadData.bestNumInliers;
		status->maxIter = bestThreadData.maxIter;
		status->numIter = bestThreadData.numIter;
		status->modelParamsBest = bestThreadData.modelParamsBest;

		COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "RANSAC status: numInliers=%zu, totalPoints=%zu, numIter=%zu, maxIter=%zu",
			status->numInliers, control->totalPoints, status->numIter, status->maxIter
		);

		/* 
			Build inliers and make a final estimation of the parameters
			The condition "bestNumInliers >= minModelPoints" is always true because at least one set of random points will be matched as inliers
			unless the input data is incorrect (e.g. contains 'inf' or 'nan' values or points are the same).
		*/
		if (status->numInliers >= control->minModelPoints) {
			const size_t bestNumInliers = status->numInliers;
			/* Build residual using best params selected in ransac loop */
			CompVMatPtr residual;
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(&residual, 1, control->totalPoints));
			FloatType* residualPtr = residual->ptr<FloatType>();
			bool userBreak = false;
			COMPV_CHECK_CODE_RETURN(buildResiduals(
				control, status->modelParamsBest, residual, userBreak
			));
			/* Re-compute inliers indices using the residual from best params */
			const FloatType threshold = control->threshold;
			const size_t totalPoints = control->totalPoints;
			CompVMathStatsRansacModelIndices indices(bestNumInliers);
			size_t i, k;
			for (i = 0, k = 0; i < totalPoints && k < bestNumInliers; ++i) {
				if (residualPtr[i] < threshold) {
					indices[k++] = i;
				}
			}
			COMPV_CHECK_EXP_RETURN(k != bestNumInliers, COMPV_ERROR_CODE_E_INVALID_CALL, "Second check for number of inliers returned different result");
			/* Build model params (will update modelParamsBest) using inliers only */
			bool userReject = false;
			COMPV_CHECK_CODE_RETURN(buildModelParams(
				control, indices, status->modelParamsBest, userReject //!\\ Use 'modelParamsBest' as input to allow callback function to start with a good guess
			));
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	private:
		template <typename FloatType>
		static COMPV_ERROR_CODE process_thread(CompVMathStatsRansacGenericThreadData<FloatType>& thread_data, bool& anotherThreadFoundInliers)
		{
			// When entering the thread, check if another thread didn't finished the work
			if (anotherThreadFoundInliers) {
				return COMPV_ERROR_CODE_S_OK;
			}

			// Get current thread's data
			const CompVMathStatsRansacControl<FloatType>* control = thread_data.control;
			CompVMathStatsRansacModelParamsFloatType& modelParamsBest = thread_data.modelParamsBest;
			COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject)
				= thread_data.buildModelParams;
			COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, CompVMatPtr residual, bool& userbreak)
				= thread_data.buildResiduals;

			// Save control values to make sure won't change while we're doing ransac processing
			const FloatType threshold = control->threshold;
			const size_t maxIter = COMPV_MATH_MAX(1, (control->maxIter / thread_data.numThreads)); // divide the work across threads
			const size_t minModelPoints = control->minModelPoints;
			const size_t totalPoints = control->totalPoints;
			const FloatType probInliersOnly = COMPV_MATH_CLIP3(0, 1, control->probInliersOnly);

			const size_t minInliersToStopIter = static_cast<size_t>(probInliersOnly * totalPoints); // minimum number of inliers to stop the iteration
			const size_t minIterBeforeUpdatingMaxIter = static_cast<size_t>(maxIter * (1 - probInliersOnly));  // minimum number of iters before starting to update the iter values using percent of outliers
			const FloatType totalPointsFloat = static_cast<FloatType>(totalPoints);
			const FloatType totalPointsFloatScale = 1 / totalPointsFloat;
			const FloatType minModelPointsFloat = static_cast<FloatType>(minModelPoints);
			size_t numIter = 0;
			size_t bestNumInliers = 0;
			size_t newMaxIter = maxIter;

			static const FloatType eps = std::numeric_limits<FloatType>::epsilon();
			const FloatType numerator = std::log(std::max(1 - probInliersOnly, eps));
			FloatType denominator, e;

			std::random_device rand_device;
			std::mt19937 prng{ rand_device() };
			std::uniform_int_distribution<size_t> unif_dist{ 0, static_cast<size_t>(totalPoints - 1) };
			CompVMathStatsRansacModelIndices indices;

			int numInliers;
			bool userBreak, userReject;

			/* Create residual */
			CompVMatPtr residual;
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(&residual, 1, totalPoints));
			FloatType* residualPtr = residual->ptr<FloatType>();

			do {
				/* Make sure to increment the number of iters in any case */
				++numIter;

				/* Build random UNIQUE indices for the model */
				indices.resize(minModelPoints);
				for (size_t i = 0; i < minModelPoints; ) {
					const size_t indice = unif_dist(prng);
					size_t j;
					for (j = 0; j < i && (indices[j] != indice); ++j);
					if (j == i) { // unique?
						indices[i++] = indice;
					}
				}

				/* Build model params */
				userReject = false;
				CompVMathStatsRansacModelParamsFloatType params;
				COMPV_CHECK_CODE_RETURN(buildModelParams(
					control, indices, params, userReject
				));
				if (userReject) {
					continue;
				}

				/* Eval model params: build residual (must be >= 0) */
				userBreak = false;
				COMPV_CHECK_CODE_RETURN(buildResiduals(
					control, params, residual, userBreak
				));

				/* Check residual and compute number of inliers */
				numInliers = 0;
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found"); // Tested with vtune and CPU intensive
				for (size_t i = 0; i < totalPoints; ++i) {
					numInliers += (residualPtr[i] < threshold);
				}
				if (numInliers > bestNumInliers) {
					bestNumInliers = static_cast<size_t>(numInliers);
					modelParamsBest = params;
				}

				/* Update RANSAC loop control data */
				e = (totalPointsFloat - numInliers) * totalPointsFloatScale; // outliers percentage
				denominator = 1 - std::pow((1 - e), minModelPointsFloat);
				if (denominator < eps) {
					// Happens when number of inliners close to number of points
					newMaxIter = 0;
					anotherThreadFoundInliers = true;
				}
				else if (numIter > minIterBeforeUpdatingMaxIter) {
					denominator = std::log(denominator);
					if (denominator < 0 && numerator >(newMaxIter * denominator)) {
						newMaxIter = COMPV_MATH_ROUNDFU_2_NEAREST_INT(numerator / denominator, size_t);
					}
				}
			} while (!anotherThreadFoundInliers && numIter < newMaxIter && bestNumInliers < minInliersToStopIter);

			thread_data.numIter = numIter;
			thread_data.maxIter = newMaxIter;
			thread_data.bestNumInliers = bestNumInliers;

			return COMPV_ERROR_CODE_S_OK;
		}
};


//
//	CompVMathStatsRansac
//


template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat32* control, CompVMathStatsRansacStatusFloat32* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat32& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelParamsFloat32& modelParams, CompVMatPtr residual, bool& userbreak)
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float32_t>(
		control, status,
		buildModelParams,
		buildResiduals
	));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat64* control, CompVMathStatsRansacStatusFloat64* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat64& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelParamsFloat64& modelParams, CompVMatPtr residual, bool& userbreak)
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float64_t>(
		control, status,
		buildModelParams,
		buildResiduals
		));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
