#pragma once
#include <JuceHeader.h>

namespace Gui
{
	class LevelMeter : public juce::Component
	{
	public:
		void paint(juce::Graphics& g) override
		{
			auto bounds = getLocalBounds().toFloat();
			g.setColour(juce::Colours::white.withBrightness(0.8f));
			g.fillRoundedRectangle(bounds, 5.f);

			g.setColour(juce::Colours::seagreen);
			const auto scaledY = juce::jmap(level, -60.f, 12.f, 0.f, (float)getHeight());
			g.fillRoundedRectangle(bounds.removeFromBottom(scaledY), 5.f);
		}

		void setLevel(const float value) { level = value; }
	private:
		float level = -60.f;
	};
}