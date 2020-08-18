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
 * Class that creates a box over a widget component that can be used to indicate
 * parameter automation especially under Pro Tools. Add it to your editor deriving
 * PluginEditorBase with registerHighlightableWidget and everything works. If you use
 * the AttachedWidget class, the attached widget will inherit this class too
 */
class HighlightableWidget : private juce::ComponentListener
{
public:
    enum class BoxLayout : int
    {
        NoBox,
        FollowParentBounds,
        SquareCenteredAboveParent
    };

    HighlightableWidget (juce::AudioProcessorValueTreeState& parameters,
                         const juce::String& paramID,
                         juce::Component& parentWidget,
                         BoxLayout boxLayout)
      : controlParameterIdx (parameters.getParameter (paramID)->getParameterIndex()),
        parent (parentWidget),
        layout (boxLayout)
    {
        box.setInterceptsMouseClicks (false, false);
        parent.addAndMakeVisible (box);
        parent.addComponentListener (this);
    }

    ~HighlightableWidget() override
    {
        parent.removeChildComponent (&box);
        parent.removeComponentListener (this);
    }

    void enableHightlight (bool shouldBeEnabled)
    {
        if (box.isEnabled == shouldBeEnabled)
            return;

        box.isEnabled = shouldBeEnabled;
        box.repaint();
    }

    void enableHighlight (bool shouldBeEnabled, juce::Colour highlightColour)
    {
        if (shouldBeEnabled)
            box.colour = highlightColour;

        enableHightlight (shouldBeEnabled);
    }

    const int controlParameterIdx;

private:
    void componentMovedOrResized (juce::Component &component, bool, bool wasResized) override
    {
        if (layout != BoxLayout::NoBox && wasResized)
        {
            auto boxBounds = component.getBounds();

            if (layout == BoxLayout::SquareCenteredAboveParent)
            {
                auto len = std::min (boxBounds.getWidth(), boxBounds.getHeight());
                boxBounds = boxBounds.withSizeKeepingCentre (len, len);
            }

            box.setBounds (boxBounds);
        }

    }

    struct Box : juce::Component
    {
        void paint (juce::Graphics& g) override
        {
            if (isEnabled)
            {
                g.setColour (colour);
                g.drawRect (getLocalBounds());
            }
        }

        juce::Colour colour = juce::Colours::red;
        bool isEnabled = false;
    };

    Box box;
    juce::Component& parent;
    BoxLayout layout;
};
}