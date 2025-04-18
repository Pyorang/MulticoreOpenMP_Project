#ifndef WARNINGVIDEO_H
#define WARNINGVIDEO_H

void get_frames_folder_path(char* folder_path, int max_len);
void generate_redframe(const char* folder_path, const double* outlier_values, float threshold, const char* output_folder);
void generate_redvideo(const char* target_folder);

#endif
