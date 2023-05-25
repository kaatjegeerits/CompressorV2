#pragma once
#include "JuceHeader.h"

class CompressorPlugin
{
public:
	CompressorPlugin();
	~CompressorPlugin();

	void prepare(const juce::dsp::ProcessSpec& spec);
	void updateCompressorSettings();
	void process(juce::AudioBuffer<float>& buffer);

	void setThreshold();
	float getThreshold(float threshold) { return threshold; };

	void setAttack();
	float getAttack(float attack) { return attack; };

	void setRelease();
	float getRelease(float release) { return release; };

	void setRatio();
	float getRatio(float ratio) { return ratio; };

	void setSlope();

	void setOutput();
	void getInput();

private:
	float input{};
	float output{};

	float threshold{};
	float attack{};
	float release{};
	float ratio{};
	float slope{};	
};

