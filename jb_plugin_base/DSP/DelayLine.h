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
 * A simple delay line implementation, designed primarily for the delayed bypass for plugins that introduce latency
 */
template<typename SampleType>
class MultichannelDelayLine
{
public:
    explicit MultichannelDelayLine (int numSamples, int nChannels = 1)
      : memory      (nChannels, numSamples),
        indices     (size_t (nChannels), 0),
        memoryPtr   (memory.getArrayOfWritePointers()),
        length      (numSamples),
        numChannels (nChannels)
    {
        memory.clear ();
    }

    /** Pushes a new sample into the delay line. This will overwrite the oldest sample */
    void push (SampleType valueToPush, int channel) noexcept
    {
        const auto c = static_cast<size_t> (channel);

        auto& idx = indices[c];
        memoryPtr[c][idx] = valueToPush;
        --idx;
        if (idx < 0) idx = 0;
    }

    /** Returns the oldest sample in the delay line */
    SampleType back (int channel) const noexcept
    {
        const auto c = static_cast<size_t> (channel);

        return memoryPtr[c][indices[c]];
    }

    /**
     * Reads the src buffer and writes the delayed signal into the dest buffer. Both buffers must not point to the
     * same memory.
     */
    void processBuffer (const SampleType* src, SampleType* dest, int bufferLength, int channel)
    {
        jassert (src != dest);

        for (int sample = 0; sample < bufferLength; ++sample)
        {
            dest[sample] = back (channel);
            push (src[sample], channel);
        }
    }

    /**
     * Reads the source block and writes the delayed signal into the destination block. The block must have the number
     * of channels and must not point to the same memory
     */
    void processBlock (const juce::dsp::AudioBlock<SampleType>& srcBlock, juce::dsp::AudioBlock<SampleType>& destBlock)
    {
        jassert (numChannels == static_cast<int> (srcBlock.getNumChannels()));
        jassert (numChannels == static_cast<int> (destBlock.getNumChannels()));
        jassert (srcBlock.getNumSamples() == destBlock.getNumSamples());

        auto numSamples = static_cast<int> (srcBlock.getNumSamples ());

        for (uint16_t channel = 0; channel < numChannels; ++channel)
            processBuffer (srcBlock.getChannelPointer (channel), destBlock.getChannelPointer (channel), numSamples, channel);
    }

    /** Clears the delay lines history */
    void reset()
    {
        std::fill (indices.begin(), indices.end(), 0);
        memory.clear();
    }

private:
    juce::AudioBuffer<SampleType> memory;
    std::vector<int> indices;

    SampleType** memoryPtr;
    const int length;
    const int numChannels;
};

}
