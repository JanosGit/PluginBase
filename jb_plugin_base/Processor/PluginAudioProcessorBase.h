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
 * You need to pass in a class containing two static functions:
 * - juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() which returns the
 *   ParameterLayout object representing the parameters of your plugin.
 * - juce::StringArray getPresetMangagerParameters() which returns an array of parameter ids that
 *   trigger the preset manager to mark a preset as "dirty"
 *
 * Furthermore it has to contain the netsted struct "Bypass", containing a static "id" string to identify
 * the parameter ID of the bypass parameter to be exposed to the host.
 *
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
 *     static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
 *     {
 *         return
 *         {
 *             std::make_unique<AudioParameterFloat> ("a", "Parameter A", NormalisableRange<float> (-100.0f, 100.0f), 0),
 *             std::make_unique<AudioParameterInt>   ("b", "Parameter B", 0, 5, 2)
 *         };
 *     }
 *
 *     static juce::StringArray getPresetMangagerParameters()
 *     {
 *         return { "a", "b" };
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
    PluginAudioProcessorBase ()
     : AudioProcessor        (createBusLayout()),
       parameters            (*this, &undoManager, getAPVTSType(), ParameterProvider::createParameterLayout()),
       stateAndPresetManager (*this, parameters, ParameterProvider::getPresetMangagerParameters(), undoManager),
       bypassParameter       (parameters.getParameter (ParameterProvider::Bypass::id))
    {
        // The bypass parameter id in your ParameterProvider class is not valid
        jassert (bypassParameter != nullptr);
    }

    /** An initialization call that will concatenate prepareToPlay and numChannelsChanged */
    virtual void prepareResources (bool sampleRateChanged, bool maxBlockSizeChanged, bool numChannelsChanged) = 0;
    virtual void processBlock (juce::dsp::AudioBlock<float>& block) = 0;

    /**
     * The audio processor has a processBlock overload with double buffers. This declaration silences shadowing warnings
     * and allows us to still override the double version if we want to.
     */
    using AudioProcessor::processBlock;

    /** Most plugins should not need this anyway */
    virtual void releaseResources() override {}

    /** Can be overridden if MIDI is needed */
    virtual bool acceptsMidi()  const override { return JucePlugin_WantsMidiInput; }
    virtual bool producesMidi() const override { return JucePlugin_ProducesMidiOutput; }
    virtual bool isMidiEffect() const override { return JucePlugin_IsMidiEffect; }

    /** Should be overriden for plugins like reverb & delay */
    virtual double getTailLengthSeconds() const override { return 0.0; }

    /** Can be overriden if programs are supported */
    virtual int getNumPrograms()                              override { return 1; }
    virtual int getCurrentProgram()                           override { return 0; }
    virtual void setCurrentProgram (int)                      override {}
    virtual const juce::String getProgramName (int)           override { return {}; }
    virtual void changeProgramName (int, const juce::String&) override {}

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
     * Creates a dsp::ProcessSpec object containig the current processors sampleRate, the current processors max number
     * of samples per block and the number of channels passed to the function
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
    juce::UndoManager                  undoManager;
    StateAndPresetManager              stateAndPresetManager;
private:

    void prepareToPlay (double newSampleRate, int maxNumSamplesPerBlock) override
    {
        auto sampleRateChanged = newSampleRate != currentSampleRate;
        auto samplesPerBlockChanged = maxNumSamplesPerBlock != currentMaxNumSamplesPerBlock;

        currentSampleRate = newSampleRate;
        currentMaxNumSamplesPerBlock = maxNumSamplesPerBlock;

        prepareResources (sampleRateChanged, samplesPerBlockChanged, false);

        prepareBypassDelayLine();
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer) override
    {
        // If process block with bypass enabled is called, call processBlockBypassed
        if (bypassParameter->getValue() > 0.5f)
        {
            processBlockBypassed (buffer, midiBuffer);
            return;
        }

        // If the last block was bypassed, a fade should occur
        if (lastBlockWasBypassed)
        {
            processWithBypassFade<false> (buffer);
            lastBlockWasBypassed = false;
        }
        else
        {
            juce::dsp::AudioBlock<float> inOutBlock (buffer);
            processBlock (inOutBlock);
        }
    }

    void processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        // If the last block was not bypassed, a fade should occur
        if (!lastBlockWasBypassed)
        {
            processWithBypassFade<true> (buffer);
            lastBlockWasBypassed = true;
        }
        else if (delayLine != nullptr)
        {
            bypassTempBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

            juce::dsp::AudioBlock<float> inOutBlock (buffer);
            juce::dsp::AudioBlock<float> bypassBlock (bypassTempBuffer);

            delayLine->processBlock (inOutBlock, bypassBlock);
            inOutBlock.copyFrom (bypassBlock);
        }
    }

    template <bool fadeIntoBypass>
    void processWithBypassFade (juce::AudioBuffer<float>& buffer)
    {
        bypassTempBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

        juce::dsp::AudioBlock<float> inOutBlock (buffer);
        juce::dsp::AudioBlock<float> bypassBlock (bypassTempBuffer);

        if (delayLine == nullptr)
        {
            bypassBlock.copyFrom (inOutBlock);
        }
        else
        {
            if (fadeIntoBypass) delayLine->reset();

            delayLine->processBlock (inOutBlock, bypassBlock);
        }

        processBlock (inOutBlock);

        auto rampLength = std::min (buffer.getNumSamples(), bypassRampLen);

        constexpr float a = fadeIntoBypass ? 1.0f : 0.0f;
        constexpr float b = fadeIntoBypass ? 0.0f : 1.0f;

        buffer          .applyGainRamp (0, rampLength, a, b);
        bypassTempBuffer.applyGainRamp (0, rampLength, b, a);

        inOutBlock.add (bypassBlock);
    }

    void numChannelsChanged() override
    {
        if (currentSampleRate == 0.0)
            currentSampleRate = 50e3;

        prepareResources (false, false, true);
        prepareBypassDelayLine();
    }

    void prepareBypassDelayLine()
    {
        if (auto delayLineDepth = getLatencySamples())
        {
            auto numChans = getTotalNumOutputChannels();
            delayLine = std::make_unique<jb::MultichannelDelayLine<float>> (delayLineDepth, numChans);
            bypassTempBuffer.setSize (numChans, currentMaxNumSamplesPerBlock);
        }
        else
        {
            delayLine.reset (nullptr);
        }
    }

    // I don't ever plan to build a plugin without editor
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
        stateAndPresetManager.getStateInformation (destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        stateAndPresetManager.setStateInformation (data, sizeInBytes);
    }

    int    currentMaxNumSamplesPerBlock = 0;
    double currentSampleRate = 0.0;

    // Bypass handling
    juce::AudioProcessorParameter*                bypassParameter;
    std::unique_ptr<MultichannelDelayLine<float>> delayLine;
    juce::AudioBuffer<float>                      bypassTempBuffer;
    bool                                          lastBlockWasBypassed;
    int                                           bypassRampLen = 128;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorBase)
};

}
