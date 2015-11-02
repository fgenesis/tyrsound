#include "tyrsound.h"

#include <stdio.h>
#include <string.h>

static void writeLE32(void *buf, unsigned int i)
{
    unsigned char *x = (unsigned char*)buf;
    x[0] = i >> 0;
    x[1] = i >> 8;
    x[2] = i >> 16;
    x[3] = i >> 24;
}

static void writeLE16(void *buf, unsigned short i)
{
    unsigned char *x = (unsigned char*)buf;
    x[0] = i >> 0;
    x[1] = i >> 8;
}

int main(int argc, char **argv)
{
    int ret = 0;
    tyrsound_Error err;
    tyrsound_Stream in, out;
    char hdr[44];
    tyrsound_Format fmt;
    unsigned dataend;
    float maxtime = 0;

    if(argc < 3)
    {
        printf("./%s INFILE OUTFILE [MAXTIME]\n", argv[0]);
        return 0;
    }

    if(argc >= 4)
        maxtime = (float)strtod(argv[3], NULL);

    tyrsound_DeviceConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.deviceName = "null";

    /* Don't need an actual audio device, use the nulldevice */
    if(tyrsound_init(NULL, &cfg) != TYRSOUND_ERR_OK)
    {
        printf("Failed to init tyrsound.\n");
        return 1;
    }

    if(tyrsound_createFileNameStream(&in, argv[1], "rb") != TYRSOUND_ERR_OK)
    {
        printf("File not found: %s\n", argv[1]);
        return 2;
    }

    if(tyrsound_createFileNameStream(&out, argv[2], "wb") != TYRSOUND_ERR_OK)
    {
        printf("Failed to open output file: %s\n", argv[2]);
        return 3;
    }


    /* Make space for the header */
    memset(hdr, 0, sizeof(hdr));
    out.write(&hdr[0], sizeof(hdr), 1, out.user);

    /* Decode and write the data */
    memset(&fmt, 0, sizeof(fmt));
    fmt.sampleBits = 16;
    fmt.signedSamples = 1;

    err = tyrsound_decodeStreamToStream(&out, &fmt, &in, 0, maxtime);
    if(err == TYRSOUND_ERR_INFINITE)
    {
        printf("Refusing to decode %s - This is an infinite stream, must supply max. time!\n", argv[1]);
        return 5;
    }
    if(err != TYRSOUND_ERR_OK)
    {
        printf("Failed to decode %s (error: %d)\n", argv[1], err);
        return 4;
    }
    in.close(in.user);

    /* Seek back to start */
    dataend = (unsigned)out.tell(out.user);
    out.seek(out.user, 0, SEEK_SET);

    /* Write WAV/RIFF header */
#define WRITE_INT4(x) do { char buf[4]; writeLE32(buf, (x)); out.write(&buf, 4, 1, out.user); } while(0)
#define WRITE_INT2(x) do { char buf[2]; writeLE16(buf, (x)); out.write(&buf, 2, 1, out.user); } while(0)

    out.write("RIFF", 4, 1, out.user);
    WRITE_INT4(((int)out.tell(out.user)) + sizeof(hdr) - 8);
    out.write("WAVE", 4, 1, out.user);

    out.write("fmt ", 4, 1, out.user);
    WRITE_INT4(16); // length of fmt block
    WRITE_INT2(1);  // type (1 = raw PCM data)
    WRITE_INT2(fmt.channels);
    WRITE_INT4(fmt.hz);
    WRITE_INT4(fmt.hz * fmt.channels * (fmt.sampleBits/8));
    WRITE_INT2(fmt.channels * (fmt.sampleBits/8));
    WRITE_INT2(fmt.sampleBits);

    out.write("data", 4, 1, out.user);
    WRITE_INT4(dataend - sizeof(hdr));

    out.close(out.user);

    /* tyrsound_shutdown(); */
    /* Didn't start up, no need to shut down */

    return 0;
}
