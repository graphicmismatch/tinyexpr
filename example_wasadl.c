#include "tinyexpr.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WASADL.h"

typedef struct SequenceElement {
  long startSampleNumber;
  long sampleCount;
  te_real *data;
} SequenceElement;

typedef struct Sequence {
  long sequenceCount;
  long lastSampleCount;
  SequenceElement *sequenceData;
} Sequence;

typedef struct SoundData {
  int samplerate;
  double seconds;
  int channels;
  long sampleCount;
  double *data;
} SoundData;

typedef enum NormalizationMode {
  PEAKS,
  LUFS,
  RMS,
  HARD_CLIPPING,
  SOFT_CLIPPING_TANH,
  SOFT_CLIPPING_CUBIC
} NormalizationMode;

void createSoundData(SoundData *out, int samplerate, double seconds,
                     int channels, long sampleCount, double *data) {
  out->samplerate = samplerate;
  out->seconds = seconds;
  out->channels = channels;
  out->sampleCount = sampleCount;
  out->data = data;
}

void createSequenceElement(SequenceElement *out, long sampleCount,
                           long startSampleNumber, te_real *data) {
  out->startSampleNumber = startSampleNumber;
  out->sampleCount = sampleCount;
  out->data = data;
}

void sequenceToSoundData(Sequence *seq, te_real *out_buffer) {
  memset(out_buffer, 0, (size_t)seq->lastSampleCount * sizeof(*out_buffer));
  for (long i = 0; i < seq->sequenceCount; i++) {
    SequenceElement se = seq->sequenceData[i];
    const long end = se.startSampleNumber + se.sampleCount;
    for (long j = se.startSampleNumber; j < end; j++) {
      out_buffer[j] += se.data[j - se.startSampleNumber];
    }
  }
}

te_real getTimeFromSamples(long nsamples, int samplerate) {
  return (te_real)nsamples / (te_real)samplerate;
}

long findPeakIndex(const te_real *samples, long sampleCount, bool inverse) {
  long ind = 0;
  te_real val = samples[0];
  for (long i = 1; i < sampleCount; i++) {
    if ((!inverse && samples[i] > val) || (inverse && samples[i] < val)) {
      ind = i;
      val = samples[i];
    }
  }
  return ind;
}

void peaksNormalization(te_real *samples, long sampleCount, te_real threshold,
                        long peakIndex) {
  const te_real peakVal = samples[peakIndex];
  if (peakVal > 0.0L) {
    const te_real scale = threshold / peakVal;
    for (long i = 0; i < sampleCount; i++) {
      samples[i] *= scale;
    }
  }
}

void normalizeSound(te_real *samples, long sampleCount, te_real threshold,
                    NormalizationMode mode) {
  (void)mode;
  const long peakInd = findPeakIndex(samples, sampleCount, false);
  const long revPeakInd = findPeakIndex(samples, sampleCount, true);
  const te_real peak = fabsl(samples[peakInd]);
  const te_real revPeak = fabsl(samples[revPeakInd]);
  const long truePeakIndex = (revPeak > peak) ? revPeakInd : peakInd;
  peaksNormalization(samples, sampleCount, threshold, truePeakIndex);
}

int write_wav_from_doubles(const char *filename, const double *samples,
                           sf_count_t numFrames, int channels, int samplerate) {
  SF_INFO sfinfo;
  SNDFILE *file;

  sfinfo.samplerate = samplerate;
  sfinfo.frames = numFrames;
  sfinfo.channels = channels;
  sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  file = sf_open(filename, SFM_WRITE, &sfinfo);
  if (!file) {
    fprintf(stderr, "Error: %s\n", sf_strerror(NULL));
    return 1;
  }

  sf_count_t written = sf_writef_double(file, samples, numFrames);
  if (written != numFrames) {
    fprintf(stderr, "Error writing samples: %s\n", sf_strerror(file));
    sf_close(file);
    return 1;
  }

  sf_close(file);
  return 0;
}

int writeSoundDataToWav(const char *filename, SoundData data) {
  return write_wav_from_doubles(filename, data.data,
                                (sf_count_t)(data.samplerate * data.seconds),
                                data.channels, data.samplerate);
}

static double *convert_to_double_buffer(const te_real *src, long long count) {
  if (count <= 0) {
    return NULL;
  }

  double *out = (double *)malloc((size_t)count * sizeof(*out));
  if (!out) {
    return NULL;
  }

  for (long long i = 0; i < count; i++) {
    out[i] = (double)src[i];
  }
  return out;
}

int main(void) {
  printf("WASADL %s\n", VERSION);

  const char *c = "floor(sin((440*2*pi)*t))";
  long long count = 0;
  int err = 0;
  const te_real step = 1.0L / 44100.0L;

  te_real *r = te_evalfunc(c, 0.0L, 10.0L, step, &err, &count);
  if (!r) {
    fprintf(stderr, "te_evalfunc failed, err=%d\n", err);
    return EXIT_FAILURE;
  }

  normalizeSound(r, count, 0.95L, PEAKS);

  double *render_data = convert_to_double_buffer(r, count);
  free(r);
  if (!render_data) {
    fprintf(stderr, "Failed to allocate output conversion buffer\n");
    return EXIT_FAILURE;
  }

  SoundData ss;
  createSoundData(&ss, 44100, 10.0, 1, (long)count, render_data);

  const int write_res = writeSoundDataToWav("test2.wav", ss);
  free(render_data);
  return (write_res == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
