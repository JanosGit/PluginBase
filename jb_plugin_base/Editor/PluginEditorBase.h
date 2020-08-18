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

/** Base class for a plugin editor with some useful basic things ready:
    - It is designed with resizable plugin editors in mind. If IsResizable::Yes is passed to the constructor
      it will set up a connection to your processors value tree to (re)store the size and will make the editor
      resizable by a bottom right resizer component
    - It inherits juce::ComponentBoundsConstrainer and applies itself as the constrainer to the editor if desired.
      For custom constraints, you can e.g. override checkBounds and can expect that all resize operations are
      constrained by it
    - It keeps a map of HighlightableWidget objects to manage highlighting of automated AAX controls
 */
template <int defaultWidth, int defaultHeight>
class PluginEditorBase : public juce::AudioProcessorEditor,
                         public juce::ComponentBoundsConstrainer,
                         private juce::Value::Listener
{
public:

    enum class IsResizable    : bool { Yes = true, No = false };
    enum class UseConstrainer : bool { Yes = true, No = false };

    template <class ParameterProvider>
    PluginEditorBase (PluginAudioProcessorBase<ParameterProvider>& proc,
                      IsResizable isResizable, UseConstrainer useConstrainer)
      : juce::AudioProcessorEditor (proc)
    {
        if (useConstrainer == UseConstrainer::Yes)
        {
            // Using a constrainer for a non-resizable plugin makes no sense
            jassert (isResizable == IsResizable::Yes);
        }

        if (isResizable == IsResizable::Yes)
        {
            setResizable (true, true);

            if (useConstrainer == UseConstrainer::Yes)
                setConstrainer (this);

            // Make the UI size restorable
            auto uiStateTree = proc.parameters.state.getChildWithName (uiStateTreeType);

            // If you hit this assert, you probably forgot to add this subtree to your parameters. To do so call
            // parameters.state.appendChild (*YourPluginEditor*::createUIStateSubtree(), nullptr);
            // in your plugin processors constructor
            jassert (uiStateTree.isValid());

            lastUIWidth .referTo (uiStateTree.getPropertyAsValue (uiStateTreeWidth,  nullptr));
            lastUIHeight.referTo (uiStateTree.getPropertyAsValue (uiStateTreeHeight, nullptr));
            lastUIWidth. addListener (this);
            lastUIHeight.addListener (this);
        }
    }

    /** Call this as the last thing in your editors constructor to restore it's size from the value tree or apply
        the default size if this is a fresh instance
     */
    void restoreSizeFromState()
    {
        setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
    }

    /** If you chose to use a non-resizable editor call this to apply the default size instead */
    void setDefaultSize()
    {
        setSize (defaultWidth, defaultHeight);
    }

    /** Override this to handle your resizing work just as you'd do in the normal resized callback. You can be sure
        that the new size matches all constrains you imposed and that the new size will be stored so that re-opening
        the editor will restore the last size
     */
    virtual void constrainedResized() = 0;

    /** Registers a control component to be highlighted on AAX parameter automation */
    void registerHighlightableWidget (const HighlightableWidget& widget)
    {
        // If you hit this assertion, a widget with that idx already exists
        jassert (highlightableWidgets.find (widget.controlParameterIdx) == highlightableWidgets.end());
        highlightableWidgets[widget.controlParameterIdx] = const_cast<HighlightableWidget*> (&widget);
    }

    /** To be called by the processor to add a subtree to the plugin state that contains GUI-related information */
    static const juce::ValueTree createUIStateSubtree()
    {
        return juce::ValueTree (uiStateTreeType,
                                {
                                    { uiStateTreeWidth,  defaultWidth },
                                    { uiStateTreeHeight, defaultHeight }
                                });
    }

private:

    void resized() override
    {
        checkComponentBounds (this);

        constrainedResized();

        // Store size
        lastUIHeight = getHeight();
        lastUIWidth  = getWidth();
    }

    void setControlHighlight (ParameterControlHighlightInfo info) override
    {
        highlightableWidgets[info.parameterIndex]->enableHighlight (info.isHighlighted, info.suggestedColour);
    }

    int getControlParameterIndex (juce::Component& component) override
    {
        if (auto* hw = dynamic_cast<jb::HighlightableWidget*> (&component))
            return hw->controlParameterIdx;

        return -1;
    }

    void valueChanged (juce::Value&) override
    {
        setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
    }

    std::map<int, HighlightableWidget*> highlightableWidgets;

    static const juce::Identifier uiStateTreeType, uiStateTreeWidth, uiStateTreeHeight;

    juce::Value lastUIWidth, lastUIHeight;
};

template <int w, int h> const juce::Identifier PluginEditorBase<w, h>::uiStateTreeType   ("uiState");
template <int w, int h> const juce::Identifier PluginEditorBase<w, h>::uiStateTreeWidth  ("width");
template <int w, int h> const juce::Identifier PluginEditorBase<w, h>::uiStateTreeHeight ("height");
}