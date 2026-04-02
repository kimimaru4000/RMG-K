#include "FrameCapture.hpp"

#include "PifReplay.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

namespace KailleraExport
{

static EmulatorProxy* s_Emulator = nullptr;
static FfmpegEncoder* s_Encoder = nullptr;
static FfmpegEncoderConfig s_EncoderConfig;
static bool s_EncoderOpened = false;
static int s_CapturedFrames = 0;
static int s_ExpectedFrameCount = 0;
static bool s_SpeedConfigured = false;
static std::vector<uint8_t> s_PixelBuffer;
static std::vector<uint8_t> s_FlippedBuffer;

void InitializeFrameCapture(EmulatorProxy* emulator,
                            FfmpegEncoder* encoder,
                            const FfmpegEncoderConfig& encoderConfig,
                            int expectedFrameCount)
{
    s_Emulator = emulator;
    s_Encoder = encoder;
    s_EncoderConfig = encoderConfig;
    s_EncoderOpened = false;
    s_CapturedFrames = 0;
    s_ExpectedFrameCount = expectedFrameCount;
    s_SpeedConfigured = false;
    s_PixelBuffer.clear();
    s_FlippedBuffer.clear();
}

int GetCapturedFrameCount()
{
    return s_CapturedFrames;
}

void FlushFrameCapture()
{
    s_FlippedBuffer.clear();
}

void FrameCaptureCallback(unsigned int)
{
    if (s_Emulator == nullptr || s_Encoder == nullptr)
    {
        return;
    }

    ResetPifReplayFrameSync();
    if (IsPifReplayFinished())
    {
        s_Emulator->stop();
        return;
    }

    if (s_ExpectedFrameCount > 0 &&
        s_CapturedFrames >= (s_ExpectedFrameCount + 120))
    {
        fprintf(stderr,
                "Replay export reached safety frame limit (%d captured, %d expected), stopping.\n",
                s_CapturedFrames,
                s_ExpectedFrameCount);
        s_Emulator->stop();
        return;
    }

    int width = 0;
    int height = 0;
    s_Emulator->readScreen(nullptr, &width, &height);
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const size_t frameSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 3U;
    if (s_PixelBuffer.size() < frameSize)
    {
        s_PixelBuffer.resize(frameSize);
    }

    s_Emulator->readScreen(s_PixelBuffer.data(), &width, &height);
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const size_t actualFrameSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 3U;
    if (!s_EncoderOpened)
    {
        s_EncoderConfig.width = width;
        s_EncoderConfig.height = height;

        std::string errorMessage;
        if (!s_Encoder->open(s_EncoderConfig, &errorMessage))
        {
            fprintf(stderr, "%s\n", errorMessage.c_str());
            s_Emulator->stop();
            return;
        }
        s_EncoderOpened = true;
    }

    if (!s_SpeedConfigured)
    {
        int speedFactor = s_Encoder->isHardwareAccelerated() ? 500 : 300;
        fprintf(stderr, "Using replay export speed target: %d%%\n", speedFactor);
        s_Emulator->coreDoCommand(M64CMD_CORE_STATE_SET, M64CORE_SPEED_FACTOR, &speedFactor);
        s_SpeedConfigured = true;
    }

    if (s_FlippedBuffer.size() < actualFrameSize)
    {
        s_FlippedBuffer.resize(actualFrameSize);
    }

    const int stride = width * 3;
    for (int y = 0; y < height; ++y)
    {
        memcpy(s_FlippedBuffer.data() + y * stride,
               s_PixelBuffer.data() + (height - 1 - y) * stride,
               static_cast<size_t>(stride));
    }

    std::string errorMessage;
    if (!s_Encoder->writeFrame(s_FlippedBuffer.data(), width, height, &errorMessage))
    {
        fprintf(stderr, "%s\n", errorMessage.c_str());
        s_Emulator->stop();
        return;
    }

    s_CapturedFrames++;

    if ((s_CapturedFrames % 60) == 0)
    {
        if (s_ExpectedFrameCount > 0)
        {
            fprintf(stderr,
                    "Captured %d / %d frames...\n",
                    s_CapturedFrames,
                    s_ExpectedFrameCount);
        }
        else
        {
            fprintf(stderr, "Captured %d frames...\n", s_CapturedFrames);
        }
    }
}

} // namespace KailleraExport
