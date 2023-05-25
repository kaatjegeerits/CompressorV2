#include "CompressorPlugin.h"
#include "JuceHeader.h"

CompressorPlugin::CompressorPlugin()
{
	threshold = 0.f ;
	attack = 0.f;
	release = 0.f;
	ratio = 4.f;
}

CompressorPlugin::~CompressorPlugin()
{

}

void CompressorPlugin::prepare(const juce::dsp::ProcessSpec& spec)
{

}

void CompressorPlugin::updateCompressorSettings()
{
	setThreshold();
	setAttack();
	setRelease();
	setRatio();
}

void CompressorPlugin::process(juce::AudioBuffer<float>& buffer)
{
	auto block = juce::dsp::AudioBlock<float>(buffer);
	auto replace = juce::dsp::ProcessContextReplacing<float>(block);
	//compressor.process(replace);
}

void CompressorPlugin::setThreshold()
{

}

void CompressorPlugin::setAttack()
{

}

void CompressorPlugin::setRelease()
{

}

void CompressorPlugin::setRatio()
{
	if (ratio > 19.9f) 
	{ 
		ratio = std::numeric_limits<float>::infinity(); 
	}
}

void CompressorPlugin::setSlope()
{
	slope = 1 - (1 / ratio);
}

void CompressorPlugin::setOutput()
{
	output = (input - threshold) / ratio + threshold;
}

void CompressorPlugin::getInput()
{

}
