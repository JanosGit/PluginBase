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

    const juce::File SettingsManager::settingsFile (StateAndPresetManager::presetDirectory.getChildFile ("Settings.json"));
    const std::string SettingsManager::settingsFileFullPath (settingsFile.getFullPathName().toStdString());

    SettingsManager::SettingsManager()
    {
        if (!settingsFile.existsAsFile())
        {
            auto result = settingsFile.create();
            jassertquiet (result.wasOk());
            return;
        }

        std::ifstream settingsFileStream (settingsFileFullPath);

        // load the settings from the file stream
        if (settingsFileStream.is_open())
        {
            try
            {
                settingsFileStream >> settings;
            }
            catch (const std::exception& e)
            {
                DBG ("Error reading settings file: " << e.what());

                settingsFileStream.close();

                settingsFile.deleteFile();
                settingsFile.create();

                return;
            }

            settingsFileStream.close();
        }
    }

    SettingsManager::~SettingsManager()
    {
        if (settingsWereWritten)
        {
            // Open in truncate mode to clear the file before writing new content
            std::ofstream settingsFileStream (settingsFileFullPath, std::ios::trunc);

            if (settingsFileStream.is_open())
            {
                try
                {
                    settingsFileStream << settings;
                }
                catch (const std::exception& e)
                {
                    DBG ("Error writing settings file: " << e.what());
                }

                settingsFileStream.close();
            }
        }

        clearSingletonInstance();
    }

    bool SettingsManager::settingExists (const juce::String &id)
    {
        return settings.contains (id.toStdString());
    }

    bool SettingsManager::getBoolSetting           (const juce::String &id, bool defaultVal)                { return getSetting (id, defaultVal); }
    int64_t SettingsManager::getInt64Setting       (const juce::String &id, int64_t defaultVal)             { return getSetting (id, defaultVal); }
    double SettingsManager::getDoubleSetting       (const juce::String &id, double defaultVal)              { return getSetting (id, defaultVal); }
    juce::String SettingsManager::getStringSetting (const juce::String &id, const juce::String &defaultVal)
    {
        const auto s = id.toStdString();

        if (settings.contains (s))
            return settings[s].get<std::string>();

        return defaultVal;
    }

    void SettingsManager::writeSetting (const juce::String &id, bool val)    { setSetting (id, val); }
    void SettingsManager::writeSetting (const juce::String &id, int64_t val) { setSetting (id, val); }
    void SettingsManager::writeSetting (const juce::String &id, double val)  { setSetting (id, val); }
    void SettingsManager::writeSetting (const juce::String &id, const juce::String& val)
    {
        settings[id.toStdString()] = val.toStdString();
        settingsWereWritten = true;
    }

    template <typename T>
    T SettingsManager::getSetting (const juce::String &id, T defaultValue)
    {
        const auto s = id.toStdString();

        if (settings.contains (s))
            return settings[s].get<T>();

        return defaultValue;
    }

    template <typename T>
    void SettingsManager::setSetting (const juce::String &id, T value)
    {
        settings[id.toRawUTF8()] = value;
        settingsWereWritten = true;
    }

    JUCE_IMPLEMENT_SINGLETON (SettingsManager)
}