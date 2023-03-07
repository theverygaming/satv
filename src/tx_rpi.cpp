#include <raspicam/raspicam.h>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <tuple>

#define SYNC_LEVEL   (-1)
#define IMAGE_MIN    (-0.5)
#define IMAGE_MAX    (1)
#define SPS          (100000)
#define HSYNC_PIXELS (5)
#define VSYNC_LINES  (10)

// stop editing here

#define IMAGE_RANGE (IMAGE_MAX - IMAGE_MIN)

class camera {
public:
    camera() {
        cam.setWidth(640);
        cam.setHeight(480);
        cam.setCaptureSize(640, 480);
        cam.setFormat(raspicam::RASPICAM_FORMAT_RGB);
        cam.setExposure(raspicam::RASPICAM_EXPOSURE_AUTO);

        if (!cam.open()) {
            throw std::runtime_error("could not open camera");
        }

        imgdata = new unsigned char[cam.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];

        width = cam.getWidth();
        height = cam.getHeight();
    }

    ~camera() {
        delete[] imgdata;
    }

    void capture_frame() {
        cam.grab();
        cam.retrieve(imgdata);
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

    raspicam::RaspiCam cam;
};

static void putsample(double value) { // takes value -1 to 1
    if (value > 1 || value < -1) {   // TODO: debug only
        fprintf(stderr, "sample value: %f\n", value);
        throw std::runtime_error("invalid sample value");
    }
    int16_t samp = value * INT16_MAX;
    fwrite(&samp, sizeof(samp), 1, stdout); // stdout is buffered, this is still not very fine tho
}

int main(int argc, char *argv[]) {
    camera cam;

    while (true) {
        cam.capture_frame();

        for (size_t i = 0; i < VSYNC_LINES * (cam.get_width() + HSYNC_PIXELS); i++) { // VSYNC
            putsample(SYNC_LEVEL);
        }

        for (size_t y = 0; y < cam.get_height(); y++) {
            for (size_t i = 0; i < HSYNC_PIXELS; i++) { // HSYNC
                putsample(SYNC_LEVEL);
            }
            for (size_t x = 0; x < cam.get_width(); x++) {
                double avg = (double)(((double)cam.get_pixel(x, y).r * 0.2126) + ((double)cam.get_pixel(x, y).g * 0.7152) + ((double)cam.get_pixel(x, y).b * 0.0722)) / 255; // 0-1 pixel brightness
                putsample((IMAGE_RANGE * avg) + IMAGE_MIN);
            }
        }
    }
    return 0;
}
