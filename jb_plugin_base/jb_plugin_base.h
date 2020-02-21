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

/*
 ==============================================================================
 
 BEGIN_JUCE_MODULE_DECLARATION
 
 ID:            jb_plugin_base
 vendor:        Janos Buttgereit
 version:       1.0.0
 name:          PluginBase
 description:   Building blocks for audio plugins
 dependencies:  juce_audio_processors, juce_data_structures
 website:       https://github.com/JanosGit/PluginBase
 license:       MIT
 
 END_JUCE_MODULE_DECLARATION
 
 ==============================================================================
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Utils/Memory.h"

#include "DSP/DelayLine.h"

#include "Parameters/AttachedWidget.h"

#include "Processor/PluginAudioProcessorBase.h"


