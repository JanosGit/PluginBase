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

/** Config: JB_INCLUDE_JSON
    Includes the json for modern cpp headers which is included as submodule to this repository.
    You can decide to not load the submodule and leave this feature turned off. This will
    however disable some features that rely on json parsing such as the SettingsManager class.
    If you enable it make sure to add Ext/json/include/ to your header searchpath or add the
    dependency in your CMakeLists.txt if you are using CMake
*/
#ifndef JB_INCLUDE_JSON
#define JB_INCLUDE_JSON 0
#endif

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <future>
#include <thread>

/** This flag is set when we link against the jb_git_version target, which contains the symbols for that struct */
#if JB_HAS_GIT_VERSION
#include "Utils/GitVersion.h"
#endif

#if JB_INCLUDE_JSON
#include <fstream>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wswitch-enum")
#include <nlohmann/json.hpp>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif // JB_INCLUDE_JSON

#include "DSP/DelayLine.h"

#include "Presets/PresetManager.h"
#include "Presets/SettingsManager.h"

#include "Utils/Memory.h"
#include "Utils/MessageOfTheDay.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Woverloaded-virtual")
#include "Processor/PluginAudioProcessorBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "Editor/HighlightableWidget.h"
#include "Editor/PluginEditorBase.h"
#include "Editor/RectangleUtils.h"

#include "Parameters/AttachedWidget.h"
