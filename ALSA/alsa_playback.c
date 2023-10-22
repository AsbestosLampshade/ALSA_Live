#define ALSA_PCM_NEW_HW_PARAMS_API
//#define DEVICE "default"

#include<alsa/asoundlib.h>

int main(){
	long loops;
	int rc;
	int size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir;
	snd_pcm_uframes_t frames;
	char *buffer;
	
	//open PCM devices for playback

	rc = snd_pcm_open(&handle,"default",SND_PCM_STREAM_PLAYBACK,0);
	if(rc<0){
		fprintf(stderr,"Unable to open device: %s\n",snd_strerror(rc));
		exit(1);
	}

	//Allocating hardware params

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle,params);//might wanna change stuff here
	
	snd_pcm_hw_params_set_access(handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
	
	snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S16_LE);

	snd_pcm_hw_params_set_channels(handle,params,2);//change this

	val=44100;//sampling rate
	snd_pcm_hw_params_set_rate_near(handle,params,&val,&dir);

	frames=32;
	snd_pcm_hw_params_set_period_size_near(handle,params,&frames,&dir);

	rc=snd_pcm_hw_params(handle,params);
	if(rc<0){
		fprintf(stderr,"Unable to set hw parameters: %s\n",snd_strerror(rc));
		exit(1);
	}

	//using a buffer large enough to store one period
	snd_pcm_hw_params_get_period_size(params,&frames,&dir);
	size=frames * 4; //2 bytes/sample, 2 channels
	buffer = (char *) malloc(size);

	//5 sec loop
	snd_pcm_hw_params_get_period_time(params,&val,&dir);

	//5secs(in micros)/val
	loops = 5000000/val;

	while(loops>0){
		loops--;
		rc = read(0,buffer,size);
		if(rc == 0){
			fprintf(stderr,"EOF on input\n");
			break;
		}
		else if(rc != size){
			fprintf(stderr,"Short read %d bytes\n",rc);
		}
		rc = snd_pcm_writei(handle,buffer,frames);
		if(rc == -EPIPE){
			//Underrun
			fprintf(stderr,"Underrun\n");
			snd_pcm_prepare(handle);
		}
		else if(rc<0){
			fprintf(stderr,"Error from writei: %s\n",snd_strerror(rc));
		}
	}
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	return 0;
}
