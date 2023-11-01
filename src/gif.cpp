#include "AnimatedGIF.h"
#include "fs.hpp"
#include "util.h"
#include <vector>
#include <memory>
#include "gif.h"

typedef struct draw_context_t {
    std::array<uint8_t, 576> *frame;
} draw_context_t;

void* gif_open(const char* path, int32_t *pSize) {
    auto file = get_fs().open(path);
    if (!file) {
        return nullptr;
    }
    *pSize = file.size();
    return new File(file);
}

void gif_close(void* handle) {
    auto file = (File*)handle;
    file->close();
    delete file;
}

int32_t gif_read(GIFFILE *pFile, uint8_t* buf, int32_t read_size) {
    auto file = static_cast<File*>(pFile->fHandle);
    int32_t bytes_read = read_size;
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < read_size)
        bytes_read = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (bytes_read <= 0)
        return 0;
    bytes_read = (int32_t)file->read(buf, bytes_read);
    pFile->iPos = file->position();
    return bytes_read;
}

int32_t gif_seek(GIFFILE *pFile, int32_t position) {
    auto file = static_cast<File*>(pFile->fHandle);
    file->seek(position);
    pFile->iPos = file->position();
    return pFile->iPos;
}

void* gif_alloc(size_t size) {
    return malloc(size);
}

void gif_free(void* buffer) {
    free(buffer);
}

void gif_draw(GIFDRAW *pDraw) {
    auto context = static_cast<draw_context_t*>(pDraw->pUser);
    // frame is assumed to be initialized with zeroes
    // (except for previously drawn lines)
    auto frame = context->frame;
    for (size_t i = 0; i < pDraw->iWidth; i++) {
        // normalize RGB565 to RGB888
        uint8_t r = ((uint16_t)(pDraw->pPixels[i*2] >> 3) * 527 + 23 ) >> 6;
        uint8_t g = ((uint16_t)((pDraw->pPixels[i*2] & 0b00000111) << 3 | (pDraw->pPixels[i*2 + 1] & 0b11100000) >> 5) * 259 + 33) >> 6;
        // we don't need the blue channel atm.
        uint8_t b = ((uint16_t)(pDraw->pPixels[i*2 + 1] & 0b00011111) * 527 + 23) >> 6;
        size_t byte_offset = pDraw->y * 12 + i / 4;
        size_t bit_offset = (3-(i % 4)) * 2;
        if (r >= 0xE0) {
            (*frame)[byte_offset] |= 0b10 << bit_offset;
        }
        if (g >= 0xE0) {
            (*frame)[byte_offset] |= 0b01 << bit_offset;
        }
        if (r >= 0xF0 && g >= 0xF0 && b >= 0xF0) {
            // treat "white" as background (turn off both LEDs)
            (*frame)[byte_offset] &= ~(0b11 << bit_offset);
        }
    }
}

std::unique_ptr<std::vector<frame>> load_gif(String &path) {
    // frame format: 48x48 pixels, 2 bits per pixel (red / yellow)
    // as the WeNiPol can handle 64 frames max, and each frame is 576 bytes,
    // we need 64 * 576 = 36864 bytes of memory at most, which should fit in ESP32 RAM easily.
    auto frames = std::unique_ptr<std::vector<frame>>(new std::vector<frame>());
    AnimatedGIF gif = AnimatedGIF();
    logf("loading gif from %s\n", path.c_str());
    gif.begin(GIF_PALETTE_RGB565_BE);
    if (!gif.open(path.c_str(), gif_open, gif_close, gif_read, gif_seek, gif_draw)) {
        logf("failed to open gif: %s\n", path.c_str());
        return nullptr;
    }
    if (gif.getCanvasWidth() != 48 || gif.getCanvasHeight() != 48) {
        gif.close();
        logf("invalid gif dimensions: %dx%d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
        return nullptr;
    }
    gif.allocFrameBuf(gif_alloc);
    gif.setDrawType(GIF_DRAW_COOKED);

    for (uint16_t frame_counter = 0; frame_counter < 64; frame_counter++) {
        logf("loading frame %d\n", frame_counter);
        frame frame_data = {
            .pixels = {}, // initialize with zeroes
            .delay_ms = 0
        };
        draw_context_t draw_context = {
            .frame = &frame_data.pixels
        };
        int delay_ms = 0;
        auto result = gif.playFrame(false, &delay_ms, &draw_context);
        if (result >= 0 && gif.getLastError() == GIF_SUCCESS) {
            frame_data.delay_ms = delay_ms;
            frames->push_back(frame_data);
        } else if (result < 0) {
            logf("failed to load frame %d: %d", frame_counter, gif.getLastError());
            break;
        }
        if (result == 0) {
            //done
            break;
        }
    }

    logf("extracted %d frames\n", frames->size());
    gif.freeFrameBuf(gif_free);
    gif.close();
    return frames;
}
