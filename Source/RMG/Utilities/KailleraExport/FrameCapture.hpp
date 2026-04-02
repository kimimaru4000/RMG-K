#pragma once

#include "EmulatorProxy.hpp"
#include "FfmpegEncoder.hpp"

namespace KailleraExport
{

void InitializeFrameCapture(EmulatorProxy* emulator,
                            FfmpegEncoder* encoder,
                            const FfmpegEncoderConfig& encoderConfig,
                            int expectedFrameCount);
void FrameCaptureCallback(unsigned int frameIndex);
void FlushFrameCapture();
int GetCapturedFrameCount();

} // namespace KailleraExport
