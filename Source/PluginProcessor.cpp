/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
CompressorV2AudioProcessor::CompressorV2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	threshold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Threshold"));
	ratio = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Ratio"));
	knee = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Knee"));
	attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Attack"));
	release = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Release"));
	gain = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Gain"));
	dryWetMix = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Mix"));

	
}

CompressorV2AudioProcessor::~CompressorV2AudioProcessor()
{

}

//==============================================================================
const juce::String CompressorV2AudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool CompressorV2AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool CompressorV2AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool CompressorV2AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double CompressorV2AudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int CompressorV2AudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int CompressorV2AudioProcessor::getCurrentProgram()
{
	return 0;
}

void CompressorV2AudioProcessor::setCurrentProgram(int index)
{
}

const juce::String CompressorV2AudioProcessor::getProgramName(int index)
{
	return {};
}

void CompressorV2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorV2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec{};
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = getTotalNumOutputChannels();
	spec.sampleRate = sampleRate;
	compressor.prepare(spec);

}

void CompressorV2AudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorV2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void CompressorV2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	auto numSamples = buffer.getNumSamples();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());
	if (gain->get() != 0)
	{
		buffer.applyGain(gain->get());
	}

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);

		for (int sample = 0; sample < numSamples; ++sample)
		{
			float level = std::abs(channelData[sample]);

			if (level > inputLevel)
			{
				inputLevel = level;
			}

			float drySignal = channelData[sample];
			float wetSignal = drySignal * gainReduction;

			channelData[sample] = (drySignal *  (1 - dryWetMix->get())) + (wetSignal * (0 + dryWetMix->get()));
		}
	}

	setKnee();
	compressor.setThreshold(threshold->get());
	compressor.setRatio(ratio->get());
	compressor.setAttack(attack->get());
	compressor.setRelease(release->get());

	auto block = juce::dsp::AudioBlock<float>(buffer);
	auto replace = juce::dsp::ProcessContextReplacing<float>(block);
	compressor.process(replace);
}

//==============================================================================
bool CompressorV2AudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorV2AudioProcessor::createEditor()
{
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void CompressorV2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	juce::MemoryOutputStream memory(destData, true);
	apvts.state.writeToStream(memory);
}

void CompressorV2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
	if (tree.isValid())
	{
		apvts.replaceState(tree);
	}
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainSettings settings;

	settings.threshold = apvts.getRawParameterValue("Threshold")->load();
	settings.knee = apvts.getRawParameterValue("Knee")->load();
	settings.ratio = apvts.getRawParameterValue("Ratio")->load();
	settings.attack = apvts.getRawParameterValue("Attack")->load();
	settings.release = apvts.getRawParameterValue("Release")->load();
	settings.mix = apvts.getRawParameterValue("Mix")->load();
	settings.gain = apvts.getRawParameterValue("Gain ")->load();

	return settings;
}
juce::AudioProcessorValueTreeState::ParameterLayout CompressorV2AudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>("Threshold", "Threshold", juce::NormalisableRange<float>(-60.f, 12.f, 1.0f, 1.0f), 0.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Knee", "Knee", juce::NormalisableRange<float>(0.f, 1.f, 0.1f, 1.0f), 0.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Ratio", "Ratio", juce::NormalisableRange<float>(1.f, 20.0f, 1.0f, 1.0f), 1.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Attack", "Attack", juce::NormalisableRange<float>(20.f, 500.f, 0.5f, 1.0f), 20.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Release", "Release", juce::NormalisableRange<float>(20.f, 500.f, 0.5f, 1.0f), 20.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Mix", "Mix", juce::NormalisableRange<float>(0.f, 1.f, 0.1f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Gain", "Gain", juce::NormalisableRange<float>(0.f, 36.0f, 1.0f, 1.0f), 0.f));

	return layout;
}

void CompressorV2AudioProcessor::setKnee()
{
	float lowerKneeBound = threshold->get() - (knee->get() / 2.f);
	float upperKneeBound = threshold->get() + (knee->get() / 2.f);

	slope = 1 - (ratio->get());

	if (ratio->get() == 1)
	{
		gainReduction = 1.0f;
	}
	else
	{
		if (knee > 0 && inputLevel > lowerKneeBound && inputLevel < upperKneeBound)
		{
			slope *= (((inputLevel - lowerKneeBound) / knee->get()) * 0.5f);
			gainReduction = slope * (lowerKneeBound - inputLevel);
		}
		else
		{
			gainReduction = slope * (threshold->get() - inputLevel);
		}

	}

}

//========================s======================================================
// This creates new instances of the plugin..


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new CompressorV2AudioProcessor();
}


