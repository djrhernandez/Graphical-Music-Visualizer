#ifndef AUDIO_H
#define AUDIO_H

#include <math.h>

#define NUMBUFFERS 32
#define NUMSOURCES 32

class Listener {
	public:
		ALfloat position[3], velocity[3];
		float   orientation[6];


	Listener() {
		for (int i = 0; i < 3; i++) {
			position[i] = velocity[i] = 0.0;
		}

		for (int i = 0; i < 3; i++) {
			if (i == 1 || i == 4)	{ orientation[i] = 1.0; } 
			else					{ orientation[i] = 0.0; }
		}
	}
} listener;

class Sound {
	public:
		ALfloat position[3], velocity[3], oritentation[3];
		ALuint	source[NUMSOURCES];
		ALuint	buffer[NUMBUFFERS];

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

void toggleMusic(bool play)
{

}

void playMusic(void) 
{

}

void cleanSounds(void)
{
	
}




