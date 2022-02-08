/*
  Copyright (C) 2022  The Blosc Developers <blosc@blosc.org>
  https://blosc.org
  License: BSD 3-Clause (see LICENSE.txt)

  Example program demonstrating the use of a Blosc from C code.

  To compile this program:

  $ gcc -O instrument_codec.c -o instrument_codec -lblosc2

  To run:

  $ ./instrument_codec
  Blosc version info: 2.0.0a2 ($Date:: 2016-01-08 #$)
  Compression: 40000000 -> 999393 (40.0x)
  Correctly extracted 5 elements from compressed chunk!
  Decompression successful!
  Successful roundtrip!

*/

#include <stdio.h>
#include "blosc2.h"

#define SIZE (100 * 1000)
#define NTHREADS 2


int main(void) {
  static float data[SIZE];
  static float data_out[SIZE];
  static float data_dest[SIZE];
  float data_subset[5];
  float data_subset_ref[5] = {5, 6, 7, 8, 9};
  int isize = SIZE * sizeof(float), osize = SIZE * sizeof(float);
  int dsize = SIZE * sizeof(float), csize;
  int i, ret;
  blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
  blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
  blosc2_context *cctx, *dctx;

  /* Initialize dataset */
  for (i = 0; i < SIZE; i++) {
    data[i] = (float)i;
  }

  printf("Blosc version info: %s (%s)\n",
         BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Create a context for compression */
  cparams.typesize = sizeof(float);
  cparams.compcode = BLOSC_BLOSCLZ;
  cparams.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_SHUFFLE;
  cparams.clevel = 5;
  cparams.nthreads = NTHREADS;
  cparams.instrument_codec = true;
  cctx = blosc2_create_cctx(cparams);

  /* Do the actual compression */
  csize = blosc2_compress_ctx(cctx, data, isize, data_out, osize);
  blosc2_free_ctx(cctx);
  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
    printf("Compression error.  Error code: %d\n", csize);
    return csize;
  }

  printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1. * isize) / csize);

  /* Create a context for decompression */
  dparams.nthreads = NTHREADS;
  dctx = blosc2_create_dctx(dparams);

  /* Decompress  */
  dsize = blosc2_decompress_ctx(dctx, data_out, csize, data_dest, dsize);
  blosc2_free_ctx(dctx);

  if (dsize < 0) {
    printf("Decompression error.  Error code: %d\n", dsize);
    return dsize;
  }

  printf("Decompression successful!\n");

  for (i = 0; i < SIZE; i++) {
    if (data[i] != data_dest[i]) {
      printf("Decompressed data differs from original!\n");
      return -1;
    }
  }
  printf("Successful roundtrip!\n");

  return 0;
}