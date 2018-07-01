#ifndef AUDIO_H
#define AUDIO_H

#include <math.h>

#define NUMBUFFERS	32
#define NUMSOURCES	32
#define BUFFER_SIZE 4096
#define PI			3.14159265358979323846264338327950
#define 2PI			PI * 2.0
#define rnd()		((Flt)rand() / (Flt)RAND_MAX)

class Listener {
	public:
		ALfloat pos[3], vel[3];
		float   ori[6];

		Listener() {
			for (int i = 0; i < 3; i++) {
				pos[i] = vel[i] = 0.0;
			}
			for (int i = 0; i < 3; i++) {
				if (i == 1 || i == 4)	{ ori[i] = 1.0; } 
				else					{ ori[i] = 0.0; }
			}
		}
} listener;

class Sound {
	public:
		ALfloat pos[3], vel[3], ori[3];
		ALuint	src[NUMSOURCES];
		ALuint	buf[NUMBUFFERS];
		ALsizei size, freq;
		ALenum	format;
		ALvoid	*data;
		
} sound;

ALenum  error;
ALboolean volumeFlag;
//Listener listener:
//Source  source;

void checkForErrors() {
	if ((error = alGetError()) != AL_NO_ERROR) {
		printf("%s\n", getError(error).c_str();
		fprintf(stderr, "ALUT Error: %s\n",  alutGetErrorString(alutGetError()));
		return;
	}
	//Clears the error state
	alGetError();
}

string getError(ALenum eID)
{
	if (eID == AL_NO_ERROR) {					return "";
	} else if (eID == AL_INVALID_NAME) {		return "Invalid name";	}
	} else if (eID == AL_INVALID_VALUE) {		return "Invalid value";	}
	} else if (eID == AL_INVALID_ENUM) {		return "Invalid enumerator"; }
	} else if (eID == AL_INVALID_OPERATION) {	return "Invalid operation";	}
	} else if (eID == AL_OUT_OF_MEMORY) {		return "Not enough memory";	}
	else { return "Error undefined."; }	
}

void initSounds(void)
{
	alutInit(0, NULL);
	if ((error = alGetError()) != AL_NO_ERROR) {
		printf("%s\n", getError(error).c_str();
		fprintf(stderr, "ALUT Error: %s\n",  alutGetErrorString(alutGetError()));
		return;
	}

	//Clears error state
	alGetError();
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	alListenerfv(AL_ORIENTATION, listener.orientation);
	alListenerfv(AL_VELOCITY, listener.velocity);
	alListenerf(AL_GAIN, 1.0f);

	source.buffer[0] = alutCreateBufferFromFile("sounds/song.wav\0");

	for (int i = 0; i < NUMSOURCES; i++) {
		alGenSources(1, &sound.source[i]);
		alSourcei(sound.source[i], AL_BUFFER, sound.buffer[i]);

		//Set properties for each wav sound
		alSourcef(sound.source[i], AL_GAIN,  1.0f);
		alSourcef(sound.source[i], AL_PITCH, 1.0f);
		alSourcefv(sound.source[i], AL_POSITION, sound.position);
		alSourcefv(sound.source[i], AL_VELOCITY, sound.velocity);
		alSourcei(sound.source[i], AL_LOOPING, AL_FALSE);
		checkForErrors();
	}	
}
