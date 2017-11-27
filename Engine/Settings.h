#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{
private:
	static Settings instance;
	float m_mouseSense;
	float m_FOV;
	float m_masterSound;
    float m_ambience;
	float m_SFX;
	float m_music;
	bool m_windowed;
    std::string m_name;
public:
	Settings();
	~Settings();
	static Settings& getInstance();
	void readFromFile();
	void writeToFile();
	float* getMouseSensePTR();
    float getMouseSense();
	void setMouseSense(float mouseSense);
	float* getFOVPTR();
    float getFOV();
	void setFOV(float FOV);
	float* getMasterSoundPTR();
    float getMasterSound();
	void setMasterSound(float masterSound);
    float* getAmbiencePTR();
    float getAmbience();
    void setAmbience(float ambience);
	float* getSFXPTR();
    float getSFX();
	void setSFX(float SFX);
	float* getMusicPTR();
    float getMusic();
	void setMusic(float music);
	bool* getWindowedPTR();
    bool getWindowed();
	void setWindowed(bool windowed);
    std::string getName();
    void setName(std::string name);
};

#endif