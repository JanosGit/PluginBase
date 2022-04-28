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
 * Helper class to query a server for a message containing useful information for
 * the user such as an update or a product announcment. Message objects are json
 * files queried from the messageURL via GET request. A message object is expected
 * to contain the following fields:
 * - GeneralMessage (optional): A nested field. If it exists, the message notifies
 *   the user about something general like a new product. Has the following
 *   sub-fields:
 *   - Version: A unique message version number > 0. When the message object is updated
 *              on the server side, this number has to be increased to make sure that we can
 *              distinguish between old and new messages
 *   - Text : The message text
 *   - Link (optional): A link for the user to click for further information
 * - Update (optional): A nested field. If it exists the message notifies the user about
 *   an update to that certain version. Has the following sub-fields:
 *   - Version: The new version available. Can be used to check if this instance is already
 *              updated
 *   - Text: A text announcing the update
 *   - Link: A link to the download website
 */
class MessageOfTheDay
{
public:
    struct Message
    {
        const int64_t version;
        const juce::String text;
        const juce::URL link;

        Message (int64_t  v, juce::String t, juce::URL l)
          : version (v),
            text (std::move (t)),
            link (std::move (l))
        {}
    };

    struct InfoAndUpdate
    {
        std::unique_ptr<Message> generalMessage;
        std::unique_ptr<Message> updateMessage;

    };

    MessageOfTheDay (juce::URL messageURL, int64_t pluginVersion)
      : url (std::move (messageURL)),
        currentPluginVersion (pluginVersion)
    {}

    /**
     * Checks if there is are new update and general messages available from the server. Only creates
     * an update message object if the corresponding version is greater than the version of this instance.
     * Only creates a general message object if the corresponding version is greater than the version number
     * passed in. This function works async on a background thread and returns a future object which will be
     * available somewhen in future. Check if either the generalMessage or updateMessage field of the
     * returned struct is null, this will indicate that there was no such message
     */
    std::future<InfoAndUpdate> checkForNewMessages (int64_t lastGeneralMessageVersion)
    {
        std::promise<InfoAndUpdate> promisedMessages;
        auto futureMessages = promisedMessages.get_future();

        // Start a background thread to ask the server for a message
        std::thread ([promisedMsg = std::move (promisedMessages), lastVersion = lastGeneralMessageVersion, this] () mutable
        {
            if (auto stream = url.createInputStream (false))
            {
                auto json = juce::JSON::parse (*stream);

                if (json.isVoid ())
                {
                    promisedMsg.set_value (InfoAndUpdate());
                    return;
                }

                auto update  = json.getProperty ("Update", juce::var());
                auto general = json.getProperty ("GeneralMessage", juce::var());

                InfoAndUpdate messages;

                if (!update.isVoid())
                {
                    int64_t updateVersion = static_cast<juce::int64>(update.getProperty ("Version", static_cast<juce::int64>(currentPluginVersion)));
                    if (updateVersion > currentPluginVersion && update.hasProperty ("Text") && update.hasProperty ("Link"))
                        messages.updateMessage = makeMessage (updateVersion, update);
                }

                if (!general.isVoid())
                {
                    int generalMessageVersion = static_cast<juce::int64>(general.getProperty ("Version", static_cast<juce::int64>(lastVersion)));
                    if (generalMessageVersion > lastVersion && general.hasProperty ("Text"))
                        messages.generalMessage = makeMessage (generalMessageVersion, general);
                }

                promisedMsg.set_value (std::move (messages));
                return;
            }

            promisedMsg.set_value (InfoAndUpdate());
            return;

        }).detach();

        // The thread is detached and runs while we already return the handle to the future message
        return futureMessages;
    }

private:
    const juce::URL url;
    const int64_t currentPluginVersion;

    static std::unique_ptr<Message> makeMessage (int64_t version, const juce::var& v)
    {
        return std::make_unique<Message> (version,
                                          v.getProperty ("Text", juce::var()).toString(),
                                          v.getProperty ("Link", juce::var()).toString());
    }
};
}