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
 * Base class to group a GUI widget and the corresponding AudioProcessorValueTreeState attachment together.
 * It will inherit WidgetType so that it can be used just like being the widget type itself. For all current
 * attachment types available by JUCE (ButtonAttachment, SliderAttachment, ComboBoxAttachment) there is a
 * specialised version, that will chose the matching AttachmentType based on the widget you passed in. Use them
 * like
 *
 * @code
 * // Will inherit juce::TextButton and invoke the button constructor, providing the button name
 * AttachedWidget<juce::TextButton> button (parameters, id, "TextButtonName");
 *
 * // Use it with your own widgets that derive from the JUCE widget, e.g. for a slider inheriting juce::Slider
 * AttachedWidget<MySlider> slider (parameters, id, mySliderArg1, mySliderArg2)
 * @endcode
 *
 *
 * Only use this base type with two template parameters in case you have a custom AttachmentType. Using it with a
 * widget type not derived from juce::Button, juce::Slider or juce::ComboBox will trigger a compilation assertion.
 *
 * To make an attached highlight visible in context of AAX automation, you can chose a layout option for a possible
 * highlighting box
 */
template <typename WidgetType, HighlightableWidget::BoxLayout boxLayout = HighlightableWidget::BoxLayout::NoBox, typename AttachmentType = void>
struct AttachedWidget : public WidgetType, public HighlightableWidget
{
    template <typename ...WidgetConstructorArgs>
    AttachedWidget (juce::AudioProcessorValueTreeState& parameters, const juce::String& paramID, WidgetConstructorArgs... args)
     :  WidgetType          (std::forward<WidgetConstructorArgs> (args)...),
        HighlightableWidget (parameters, paramID, *this, boxLayout),
        attachment          (parameters, paramID, *this)
    {
        static_assert (std::is_void<AttachmentType>::value, "No attachment type known for widget type");
    }

    AttachmentType attachment;
};

// SFINAE helper
template <typename WidgetType, typename Base>
using EnableAttachmentIfWidgetTypeDerivesFrom = typename std::enable_if<std::is_base_of<Base, WidgetType>::value, void>::type;

// Specialisation for a button with a juce::AudioProcessorValueTreeState::ButtonAttachment
template <typename ButtonType, HighlightableWidget::BoxLayout boxLayout>
struct AttachedWidget<ButtonType, boxLayout, EnableAttachmentIfWidgetTypeDerivesFrom<ButtonType, juce::Button>> : public ButtonType,
                                                                                                                  public HighlightableWidget
{
    template <typename ...ButtonConstructorArgs>
    AttachedWidget (juce::AudioProcessorValueTreeState& parameters, const juce::String& paramID, ButtonConstructorArgs&&... args)
     : ButtonType          (std::forward<ButtonConstructorArgs> (args)...),
       HighlightableWidget (parameters, paramID, *this, boxLayout),
       attachment          (parameters, paramID, *this)
    {}

    juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
};

// Specialisation for a slider with a juce::AudioProcessorValueTreeState::SliderAttachment
template <typename SliderType, HighlightableWidget::BoxLayout boxLayout>
struct AttachedWidget<SliderType, boxLayout, EnableAttachmentIfWidgetTypeDerivesFrom<SliderType, juce::Slider>> : public SliderType,
                                                                                                                  public HighlightableWidget
{
    template <typename ...SliderConstructorArgs>
    AttachedWidget (juce::AudioProcessorValueTreeState& parameters, const juce::String& paramID, SliderConstructorArgs&&... args)
     : SliderType          (std::forward<SliderConstructorArgs> (args)...),
       HighlightableWidget (parameters, paramID, *this, boxLayout),
       attachment          (parameters, paramID, *this)
    {}

    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

// Specialisation for a button with a juce::AudioProcessorValueTreeState::ComboBoxAttachment
template <typename ComboBoxType, HighlightableWidget::BoxLayout boxLayout>
struct AttachedWidget<ComboBoxType, boxLayout, EnableAttachmentIfWidgetTypeDerivesFrom<ComboBoxType, juce::ComboBox>> : public ComboBoxType,
                                                                                                                        public HighlightableWidget
{
    template <typename ...ComboBoxConstructorArgs>
    AttachedWidget (juce::AudioProcessorValueTreeState& parameters, const juce::String& paramID, ComboBoxConstructorArgs&&... args)
     : ComboBoxType        (std::forward<ComboBoxConstructorArgs> (args)...),
       HighlightableWidget (parameters, paramID, *this, boxLayout),
       attachment          (parameters, paramID, *this)
    {}

    juce::AudioProcessorValueTreeState::ComboBoxAttachment attachment;
};

}
