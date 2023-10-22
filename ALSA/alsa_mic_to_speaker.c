#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

int main() {
  int rc_rec,rc_play;
  int size;
  snd_pcm_t *handle_rec,*handle_play;
  snd_pcm_hw_params_t *params_rec,*params_play;
  unsigned int val;
  int dir_rec,dir_play;
  snd_pcm_uframes_t frames;
  char *buffer;

  /* Open PCM device for recording (capture). */
  rc_rec = snd_pcm_open(&handle_rec, "default",
                    SND_PCM_STREAM_CAPTURE, 0);
  rc_play = snd_pcm_open(&handle_play, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc_rec < 0) {
    fprintf(stderr,
            "unable to open pcm record device: %s\n",
            snd_strerror(rc_rec));
    exit(1);
  }
  
  if (rc_play <0) {
  	fprintf(stderr,
		"unable to open pcm play device: %s\n",
            	snd_strerror(rc_play));
    	exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params_rec);
  snd_pcm_hw_params_alloca(&params_play);//not sure if it's required

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle_rec, params_rec);
  snd_pcm_hw_params_any(handle_play, params_play);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle_rec, params_rec,
                      SND_PCM_ACCESS_RW_INTERLEAVED);
                      
  snd_pcm_hw_params_set_access(handle_play, params_play,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle_rec, params_rec,
                              SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_format(handle_play, params_play,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle_rec, params_rec, 1);
  snd_pcm_hw_params_set_channels(handle_play, params_play, 1);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle_rec, params_rec,
                                  &val, &dir_rec);
  snd_pcm_hw_params_set_rate_near(handle_play, params_play,
                                  &val, &dir_play);

  /* Set period size to 32 frames. */
  frames = 8;
  snd_pcm_hw_params_set_period_size_near(handle_rec,
                              params_rec, &frames, &dir_rec);
  snd_pcm_hw_params_set_period_size_near(handle_play,
                              params_play, &frames, &dir_play);

  /* Write the parameters to the driver */
  rc_rec = snd_pcm_hw_params(handle_rec, params_rec);
  if (rc_rec < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc_rec));
    exit(1);
  }
  
  rc_play = snd_pcm_hw_params(handle_play, params_play);
  if (rc_play < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc_play));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params_rec,
                                      &frames, &dir_rec);
  snd_pcm_hw_params_get_period_size(params_play,
                                      &frames, &dir_play);
  size = frames * 2; /* 2 bytes/sample, 1 channels */
  buffer = (char *) malloc(size);

  while (1) {
    rc_rec = snd_pcm_readi(handle_rec, buffer, frames);
    rc_play = snd_pcm_writei(handle_play,buffer,frames);
    if (rc_rec == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle_rec);
    } else if (rc_rec < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc_rec));
    } else if (rc_rec != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc_rec);
    }
   // rc_rec = write(1, buffer, size);
   // if (rc_rec != size)
     // fprintf(stderr,
       //       "short write: wrote %d bytes\n", rc_rec);
       
       //Please add error checking
  }

  snd_pcm_drain(handle_rec);
  snd_pcm_close(handle_rec);
  snd_pcm_drain(handle_play);
  snd_pcm_close(handle_play);
  free(buffer);

  return 0;
}
