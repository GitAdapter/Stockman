#ifndef NOISEMACHINE_H
#define NOISEMACHINE_H

#include <Engine\Constants.h>	// Paths to folders
#include <fmod_errors.h>
#include <iostream>
#include <stdarg.h>
#include <string>
#include <btBulletCollisionCommon.h>
#include <Misc\Sound\NoiseStructs.h>
#include <Misc\Sound\SoundSource.h>

#define SOUND_SFX_PATH "Resources/Sound/SFX/"
#define SOUND_MUSIC_PATH "Resources/Sound/Music/"

namespace Logic
{
	class NoiseMachine
	{
	public:
		static NoiseMachine& Get()
		{
			static NoiseMachine noiceMachine;
			return noiceMachine;
		}

		void init();
		void clear();
		void update(ListenerData& listener);

		void playSFX(SFX sfx, SoundSource* soundSource, bool overdrive);
		void playMusic(MUSIC music, SoundSource* soundSource, bool overdrive);

		void setGroupVolume(CHANNEL_GROUP group, float inVolume);
		void setGroupPitch(CHANNEL_GROUP group, float inPitch);
		float getGroupVolume(CHANNEL_GROUP group);
		float getGroupPitch(CHANNEL_GROUP group);

	private:
		void initGroups();
		int initSFX(LOAD_MODE loadMode);
        int initMusic(LOAD_MODE loadMode);

		void checkIfPlay(Sound* sound, SoundSource* soundSource, bool overdrive);
		void play(Sound* sound, SoundSource* soundSource);

		FMOD_RESULT createSound(LOAD_MODE loadMode, SFX sfx, CHANNEL_GROUP group, std::string path, FMOD_MODE mode);
		FMOD_RESULT createSound(LOAD_MODE loadMode, MUSIC music, CHANNEL_GROUP group, std::string path, FMOD_MODE mode);

		FMOD::System*		m_system;
		FMOD::ChannelGroup*	m_group[THRESHOLD::MAX_GROUPS];
		Sound*				m_sfx[THRESHOLD::MAX_SFX];
		Sound*				m_music[THRESHOLD::MAX_SONGS];

		// Debugging and Errors
		void CRASH_EVERYTHING(const char *format, ...);
		void ERRCHECK(FMOD_RESULT result);
	};
}

#endif // !NOICEMACHINE_H
