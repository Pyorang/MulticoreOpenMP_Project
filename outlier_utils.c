#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "outlier_utils.h"

void compute_outlier_serial(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array)
{
	for (int i = 0; i < totalFrames - segmentSize + 1; i++)
	{
		//평균 구하기
		double entropy_average = 0;
		
		for (int j = i; j < i+ segmentSize; j++)
		{
			entropy_average += entropy_array[j];
		}

		entropy_average /= segmentSize;

		//분산 구하기
		double entropy_variance = 0;

		for (int j = i; j < i + segmentSize; j++)
		{
			entropy_variance += pow((entropy_average - entropy_array[j]),2);
		}

		entropy_variance /= segmentSize;

		outlier_array[i] = entropy_average + K * entropy_variance;
	}
}

void compute_outlier_parallel(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array)
{
	int i;
	int j;
#pragma omp parallel for
	for (i = 0; i < totalFrames - segmentSize + 1; i++)
	{
		// 평균 계산
		double entropy_average = 0.0;

#pragma omp parallel for reduction(+:entropy_average)
		for (j = 0; j < segmentSize; j++) {
			entropy_average += entropy_array[i + j];
		}
		entropy_average /= segmentSize;

		// 분산 계산
		double entropy_variance = 0.0;

#pragma omp parallel for reduction(+:entropy_variance)
		for (j = 0; j < segmentSize; j++) {
			double diff = entropy_array[i + j] - entropy_average;
			entropy_variance += diff * diff;
		}
		entropy_variance /= segmentSize;

		// 이상치 기준값 계산 및 저장
		outlier_array[i] = entropy_average + K * entropy_variance;
	}
}