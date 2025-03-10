#include "data/data.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

data_t **get_dataset(const char *filename, unsigned int *dataset_length, unsigned int *inputs_length) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening %s for reading data set\n", filename);
        return NULL;
    }

    fread(dataset_length, sizeof(unsigned int), 1, file);
    data_t **dataset = malloc(sizeof(data_t*) * *dataset_length);

    fread(inputs_length, sizeof(unsigned int), 1, file);
    for (unsigned int i = 0; i < *dataset_length; i++) {
        data_t *data = malloc(sizeof(data_t));
        data->inputs = malloc(sizeof(float) * *inputs_length);
        size_t read_count = fread(data->inputs, sizeof(float), *inputs_length, file);
        if (read_count != *inputs_length) {
            printf("Error: Failed to read data. Maybe you haven't run 'git lfs pull'?\n");
        }
        fread(&(data->neuron_index), sizeof(unsigned int), 1, file);

        dataset[i] = data;
    }

    fclose(file);

    return dataset;
}

void free_dataset(data_t **dataset, unsigned int dataset_length) {
    for (unsigned int i = 0; i < dataset_length; i++) {
        free_data(dataset[i]);
    }
    free(dataset);
}

void free_data(data_t *data) {
    free(data->inputs);
}

data_t *get_data_copy(data_t* data, unsigned int inputs_length) {
    data_t *copy = malloc(sizeof(data_t));

    copy->neuron_index = data->neuron_index;

    size_t input_size = sizeof(float) * inputs_length;
    copy->inputs = malloc(input_size);
    memcpy(copy->inputs, data->inputs, input_size);

    return copy;
}

void rotate_data(data_t *data, int width, int height, float angle) {
    float rad = angle * M_PI / 180.0f;
    float cos_angle = cos(rad);
    float sin_angle = sin(rad);
    float *new_inputs = malloc(sizeof(float) * width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int center_x = width / 2;
            int center_y = height / 2;
            int src_x = (int)round((x - center_x) * cos_angle - (y - center_y) * sin_angle + center_x);
            int src_y = (int)round((x - center_x) * sin_angle + (y - center_y) * cos_angle + center_y);

            if (src_x >= 0 && src_x < width && src_y >= 0 && src_y < height) {
                new_inputs[y * width + x] = data->inputs[src_y * width + src_x];
            } else {
                new_inputs[y * width + x] = 0.0f;  // Set background color to black
            }
        }
    }

    free(data->inputs);
    data->inputs = new_inputs;
}

void scale_data(data_t *data, int width, int height, float scale) {
    int scale_width = width * scale;
    int scale_height = height * scale;
    float *scale_inputs = malloc(sizeof(float) * scale_width * scale_height);
    float *new_inputs = malloc(sizeof(float) * width * height);

    for (int y = 0; y < scale_height; y++) {
        for (int x = 0; x < scale_width; x++) {
            int src_x = (int)round(x / scale);
            int src_y = (int)round(y / scale);

            if (src_x >= 0 && src_x < width && src_y >= 0 && src_y < height) {
                scale_inputs[y * scale_width + x] = data->inputs[src_y * width + src_x];
            } else {
                scale_inputs[y * scale_width + x] = 0.0f;  // Set background color to black
            }
        }
    }
    int off_set_x = (scale_width - width) / 2;
    int off_set_y = (scale_height - height) / 2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int scale_x = x + off_set_x;
            int scale_y = y + off_set_y;
            if (scale_x >= 0 && scale_x < scale_width && scale_y >= 0 && scale_y < scale_height) {
                new_inputs[y * width + x] = scale_inputs[scale_y * scale_width + scale_x];
            } else {
                new_inputs[y * width + x] = 0.0f;
            }
        }
    }

    free(scale_inputs);
    free(data->inputs);
    data->inputs = new_inputs;
}

void offset_data(data_t *data, int width, int height, float offset_x, float offset_y) {
    float *new_inputs = malloc(sizeof(float) * width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float new_x = x + offset_x;
            float new_y = y + offset_y;

            int src_x = (int)round(new_x);
            int src_y = (int)round(new_y);
            if (src_x >= 0 && src_x < width && src_y >= 0 && src_y < height) {
                new_inputs[y * width + x] = data->inputs[src_y * width + src_x];
            } else {
                new_inputs[y * width + x] = 0.0f;  // Set background color to black
            }
        }
    }
    free(data->inputs);
    data->inputs = new_inputs;
}

void noise_data(data_t *data, unsigned int inputs_length, float noise_factor, float probability) {
    for (unsigned int i = 0; i < inputs_length; i++) {
        float random_value = (float)rand() / (float)RAND_MAX;
        if (random_value <= probability) {
            float noise = ((float)rand() / (float)RAND_MAX * noise_factor);
            float new_value = data->inputs[i] + noise;

            data->inputs[i] = fmin(new_value, 1.0f);
        }
    }
}

