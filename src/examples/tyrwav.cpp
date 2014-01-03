#include "tyrsound.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int ret = 0, val;
    short i2;
    tyrsound_Error err;
    tyrsound_Stream in, out;
    char hdr[44];
    tyrsound_Format fmt;
    unsigned dataend;

    if(argc < 3)
    {
        printf("./%s INFILE OUTFILE\n", argv[0]);
        return 0;
    }

    /* Don't need an actual audio device, use the nulldevice */
    /*if(tyrsound_init(NULL, "null") != TYRSOUND_ERR_OK)
    {
        printf("Failed to init tyrsound.\n");
        return 1;
    }*/
    /* ... but for decoding, the sound system doesn't even need to be started up */

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

    err = tyrsound_decodeStream(out, &fmt, in, &fmt, 0);
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
#define WRITE_INT4(x) do { val = (x); out.write(&val, 4, 1, out.user); } while(0)
#define WRITE_INT2(x) do { i2 = (x); out.write(&i2, 2, 1, out.user); } while(0)

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
