/*
MIT License

Copyright (c) 2020 Janos Buttgereit

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

namespace jb
{

/**
 * You need to pass in a class containing a static function called createParameterLayout() which returns the
 * ParameterLayout object representing the parameters of your plugin. Furthermore it is expected to contain
 * static String Bypass::id to identify the parameter ID of the bypass parameter to be exposed to the host.
 * Example:
 *
 * @code
 * struct MyPluginParameters
 * {
 *     struct Bypass
 *     {
 *         static const juce::String id;
 *     }
 *
 *     static juce::AudioProcessorValueTreeState::ParameterLayout MyProcessor::createParameterLayout()
 *     {
 *         return
 *         {
 *             std::make_unique<AudioParameterFloat> ("a", "Parameter A", NormalisableRange<float> (-100.0f, 100.0f), 0),
 *             std::make_unique<AudioParameterInt>   ("b", "Parameter B", 0, 5, 2)
 *         };
 *     }
 * }
 *   @endcode
 *
 */
template <class ParameterProvider>
class PluginAudioProcessorBase : public juce::AudioProcessor
{
public:
    //==============================================================================
    PluginAudioProcessorBase()
     : AudioProcessor  (createBusLayout()),
       parameters      (*this, &undoManager, getAPVTSType(), ParameterProvider::createParameterLayout()),
       bypassParameter (parameters.getParameter (ParameterProvider::Bypass::id))
    {}

    virtual ~PluginAudioProcessorBase() {}

    /** An initialization call that will concatenate prepareToPlay and numChannelsChanged */
    virtual void prepareResources (bool sampleRateChanged, bool maxBlockSizeChanged, bool numChannelsChanged) = 0;

    /** Most plugins should not need this anyway */
    virtual void releaseResources() override {}

    /** Can be overridden if MIDI is needed */
    virtual bool acceptsMidi()  const override { return JucePlugin_WantsMidiInput; }
    virtual bool producesMidi() const override { return JucePlugin_ProducesMidiOutput; }
    virtual bool isMidiEffect() const override { return JucePlugin_IsMidiEffect; }

    /** Should be overriden for plugins like reverb & delay */
    virtual double getTailLengthSeconds() const override { return 0.0; }

    /** Can be overriden if programs are supported */
    virtual int getNumPrograms()                                            override { return 1; }
    virtual int getCurrentProgram()                                         override { return 0; }
    virtual void setCurrentProgram (int index)                              override {}
    virtual const juce::String getProgramName (int index)                   override { return {}; }
    virtual void changeProgramName (int index, const juce::String& newName) override {}

    /**
     * This default implementation only reports a mono in- and output to the host. Override its return value if
     * you need more.
     */
    virtual juce::AudioProcessor::BusesProperties createBusLayout()
    {
        return BusesProperties().withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                                .withOutput ("Output", juce::AudioChannelSet::mono(), true);
    }

    double getSampleRate()         { return currentSampleRate; }

    int getMaxNumSamplesPerBlock() { return currentMaxNumSamplesPerBlock; }

    /**
     * Creates a ProcessSpec object containig the current processors sampleRate, the current processors max number of
     * samples per block and the number of channels passed to the function
     */
    juce::dsp::ProcessSpec createProcessSpec (int numChannels)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate       = currentSampleRate;
        spec.maximumBlockSize = static_cast<uint32_t> (currentMaxNumSamplesPerBlock);
        spec.numChannels      = static_cast<uint32_t> (numChannels);
        return spec;
    }

    juce::AudioProcessorValueTreeState parameters;
    juce::UndoManager undoManager;
private:

    void prepareToPlay (double newSampleRate, int maxNumSamplesPerBlock) override
    {
        auto sampleRateChanged = newSampleRate != currentSampleRate;
        auto samplesPerBlockChanged = maxNumSamplesPerBlock != currentMaxNumSamplesPerBlock;

        currentSampleRate = newSampleRate;
        currentMaxNumSamplesPerBlock = maxNumSamplesPerBlock;

        prepareResources (sampleRateChanged, samplesPerBlockChanged, false);
    }

    void numChannelsChanged() override { prepareResources (false, false, true); }

    // Don't plan to ever build a plugin without editor
    bool hasEditor() const override { return true; }

    #ifdef JucePlugin_Name
    const juce::String getName() const override { return JucePlugin_Name; }
    #endif

    juce::Identifier getAPVTSType()
    {
        return getName().retainCharacters ("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890-+_");
    }

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParameter; }

    void getStateInformation (juce::MemoryBlock& destData) override
    {
        auto state = parameters.copyState();
        auto xml = state.createXml();
        copyXmlToBinary (*xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        auto xmlState = getXmlFromBinary (data, sizeInBytes);

        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName (parameters.state.getType()))
                parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
    }

    int currentMaxNumSamplesPerBlock = 0;
    double currentSampleRate = 0.0;

    juce::AudioProcessorParameter* bypassParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorBase)
};

}