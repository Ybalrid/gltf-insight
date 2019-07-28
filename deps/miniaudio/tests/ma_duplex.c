#include <stdio.h>

#define MA_DEBUG_OUTPUT
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio.h"

#define DR_WAV_IMPLEMENTATION
#include "../extras/dr_wav.h"

#ifdef __EMSCRIPTEN__
void main_loop__em()
{
}
#endif

void log_callback(ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message)
{
    (void)pContext;
    (void)pDevice;
    (void)logLevel;
    printf("%s\n", message);
}

void stop_callback(ma_device* pDevice)
{
    (void)pDevice;
    printf("STOPPED\n");
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    /* In this test the format and channel count are the same for both input and output which means we can just memcpy(). */
    ma_copy_memory(pOutput, pInput, frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));

#if 1
    /* Also write to a wav file for debugging. */
    drwav* pWav = (drwav*)pDevice->pUserData;
    ma_assert(pWav != NULL);

    drwav_write_pcm_frames(pWav, frameCount, pInput);
#endif
}

int main(int argc, char** argv)
{
    ma_result result;

#if 1
    drwav_data_format wavFormat;
    wavFormat.container     = drwav_container_riff;
    wavFormat.format        = DR_WAVE_FORMAT_PCM;
    wavFormat.channels      = 2;
    wavFormat.sampleRate    = 44100;
    wavFormat.bitsPerSample = 16;

    drwav wav;
    if (drwav_init_file_write(&wav, "output.wav", &wavFormat) == DRWAV_FALSE) {
        printf("Failed to initialize output file.\n");
        return -1;
    }
#endif
    

    ma_backend backend = ma_backend_wasapi;

    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.logCallback = log_callback;

    ma_context context;
    result = ma_context_init(&backend, 1, &contextConfig, &context);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize context.\n");
        return result;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = NULL;
    deviceConfig.capture.format     = ma_format_s16;
    deviceConfig.capture.channels   = 2;
    deviceConfig.playback.pDeviceID = NULL;
    deviceConfig.playback.format    = ma_format_s16;
    deviceConfig.playback.channels  = 2;
    deviceConfig.playback.shareMode = ma_share_mode_shared;
    deviceConfig.sampleRate         = 44100;
    //deviceConfig.bufferSizeInMilliseconds = 60;
    deviceConfig.bufferSizeInFrames = 4096;
    //deviceConfig.periods            = 3;
    deviceConfig.dataCallback       = data_callback;
    deviceConfig.stopCallback       = stop_callback;
    deviceConfig.pUserData          = &wav;

    ma_device device;
    result = ma_device_init(&context, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        return result;
    }

#ifdef __EMSCRIPTEN__
    getchar();
#endif

    ma_device_start(&device);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop__em, 0, 1);
#else
    printf("Press Enter to quit...\n");
    getchar();
#endif

    ma_device_uninit(&device);
    drwav_uninit(&wav);

    (void)argc;
    (void)argv;
    return 0;
}
