#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <tuple>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SYNC_LEVEL   (-1)
#define IMAGE_MIN    (-0.5)
#define IMAGE_MAX    (1)
#define HSYNC_PIXELS (5)
#define VSYNC_LINES  (10)

// stop editing here

#define IMAGE_RANGE (IMAGE_MAX - IMAGE_MIN)

class image {
public:
    image(std::string path) {
        int channels = 0;
        imgdata = stbi_load(path.c_str(), &width, &height, &channels, 3);
        if (imgdata == nullptr) {
            throw std::runtime_error("could not open image");
        }
    }

    ~image() {
        stbi_image_free(imgdata);
    }

    size_t get_width() {
        return width;
    }

    size_t get_height() {
        return height;
    }

    struct __attribute__((__packed__)) pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    inline struct pixel get_pixel(size_t x, size_t y) {
        return *(struct pixel *)((uint8_t *)imgdata + (x + width * y) * 3);
    }

private:
    unsigned char *imgdata;
    int width, height;
};

static void putsample(double value) { // takes value -1 to 1
    if (value > 1 || value < -1) {    // TODO: debug only
        fprintf(stderr, "sample value: %f\n", value);
        throw std::runtime_error("invalid sample value");
    }
    int16_t samp = value * INT16_MAX;
    fwrite(&samp, sizeof(samp), 1, stdout); // stdout is buffered, this is still not very fine tho
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input filename>\n", argv[0]);
        return 1;
    }
    image img(argv[1]);

    fprintf(stderr, "width: %zu height: %zu\n", img.get_width() + HSYNC_PIXELS, img.get_height() + VSYNC_LINES);
    fprintf(stderr, "required SR for 1 FPS %zu\n", (img.get_width() + HSYNC_PIXELS) * (img.get_height() + VSYNC_LINES));

    for (size_t i = 0; i < VSYNC_LINES * (img.get_width() + HSYNC_PIXELS); i++) { // VSYNC
        putsample(SYNC_LEVEL);
    }

    for (size_t y = 0; y < img.get_height(); y++) {
        for (size_t i = 0; i < HSYNC_PIXELS; i++) { // HSYNC
            putsample(SYNC_LEVEL);
        }
        for (size_t x = 0; x < img.get_width(); x++) {
            double avg = (double)(((double)img.get_pixel(x, y).r * 0.2126) + ((double)img.get_pixel(x, y).g * 0.7152) + ((double)img.get_pixel(x, y).b * 0.0722)) / 255; // 0-1 pixel brightness
            putsample((IMAGE_RANGE * avg) + IMAGE_MIN);
        }
    }
    return 0;
}
