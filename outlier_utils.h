#ifndef OUTLIER_UTILS_H
#define OUTLIER_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

	// ���� ��� �̻�ġ ���
	void compute_outlier_serial(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array);

	// ����(OpenMP) ��� �̻�ġ ���
	void compute_outlier_parallel(int segmentSize, double K, int totalFrames, double* entropy_array, double* outlier_array);

#ifdef __cplusplus
}
#endif

#endif