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

#if JB_INCLUDE_JSON

class SettingsManager : public juce::DeletedAtShutdown
{
public:
    SettingsManager();

    ~SettingsManager();

    bool         getBoolSetting   (const juce::String& id, bool defaultVal);
    int64_t      getInt64Setting  (const juce::String& id, int64_t defaultVal);
    double       getDoubleSetting (const juce::String& id, double defaultVal);
    juce::String getStringSetting (const juce::String& id, const juce::String& defaultVal);

    void writeSetting (const juce::String& id, bool val);
    void writeSetting (const juce::String& id, int64_t val);
    void writeSetting (const juce::String& id, double val);
    void writeSetting (const juce::String& id, const juce::String& val);

    JUCE_DECLARE_SINGLETON (SettingsManager, false)
private:
    static const juce::File settingsFile;
    static const std::string settingsFileFullPath;

    nlohmann::json settings;
    bool settingsWereWritten = false;

    template <typename T> T    getSetting (const juce::String& id, T defaultValue);
    template <typename T> void setSetting (const juce::String& id, T value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsManager)
};

#endif // JB_INCLUDE_JSON
}