#pragma once
#include <JuceHeader.h>

namespace Gui
{
	class SliderButton : public juce::Component
	{
	public:
		void paint(juce::Graphics& g) override
		{
			g.setColour(juce::Colours::black);
			g.fillRect(getLocalBounds());
		}
	};

	
}
