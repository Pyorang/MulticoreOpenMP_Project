#ifndef OUTLIER_UTILS_H
#define OUTLIER_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

	// 직렬 방식 이상치 계산
	void compute_outlier_serial(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array);

	// 병렬(OpenMP) 방식 이상치 계산
	void compute_outlier_parallel(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array);

#ifdef __cplusplus
}
#endif

#endif