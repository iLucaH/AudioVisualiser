/*
  ==============================================================================

    GlobalSocketHandler.h
    Created: 13 Mar 2026 5:50:33pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <unordered_map>

#include "PluginEditor.h"
#include "SocketCueResolver.h"

class GlobalSocketHandler {
public:
    GlobalSocketHandler(SocketCueResolver& socketCueResolver) : socketCueResolver(socketCueResolver) {
        if (serverSocket.createListener(0)) {
            port = serverSocket.getBoundPort();
            DBG("Global server socket listening on port " << juce::String(port) << ".");
            DBG("Available connection handle: " << getConnectionHandle());
        } else {
            DBG("The global server socket failed to init a listening session on port " << port << ".");
        }
    }

    void startListening() {
        juce::Thread::launch([this]() {
            running.store(true);
            while (running.load()) {
                if (serverSocket.waitUntilReady(true, 1) > 0) {
                    std::unique_ptr<juce::StreamingSocket> clientSocket(serverSocket.waitForNextConnection());
                    if (clientSocket) {
                        DBG("Client connected! " << clientSocket->getRawSocketHandle());
                        clients.push_back({ std::move(clientSocket), false });
                    }
                }

                for (auto& client : clients) {
                    if (client.socket->waitUntilReady(true, 0)) {
                        char buffer[1024];
                        int bytesRead = client.socket->read(buffer, sizeof(buffer), true);
                        if (bytesRead > 0) {
                            juce::String received(buffer, bytesRead);
                            DBG("Global socket handler received data: " << received << " from client: " << client.socket->getRawSocketHandle() << ".");
                            resolveResponse(client, received);
                        }
                    }
                }
                juce::Thread::sleep(500);
            }
        });
    }

    void stopListening() {
        running.store(false);
    }

    bool isListening() {
        return running.load();
    }

    void destroy() {
        serverSocket.close();
        for (auto& client : clients) {
            client.socket.get()->close();
            client.socket.release();
        }
        clients.clear();
    }

    juce::String getConnectionHandle() {
        auto addresses = getLocalIPv4Addresses();
        if (addresses.size() == 0 || port == -1)
            return "";
        return addresses.getFirst() + ":" + juce::String(port);
    }

private:
    SocketCueResolver& socketCueResolver;

    int port = -1;

    std::atomic<bool> running{ false };

    juce::StreamingSocket serverSocket;

    struct Client {
        std::unique_ptr<juce::StreamingSocket> socket;
        bool authenticated;
    };

    std::vector<Client> clients;

    bool isLoopback(const juce::IPAddress& addr) {
        if (!addr.isIPv6) // Only check IPv4.
            return addr.address[0] == 127;

        // For IPv6, loopback is ::1
        if (addr.isIPv6) {
            for (int i = 0; i < 15; ++i)
                if (addr.address[i] != 0)
                    return false;
            return addr.address[15] == 1;
        }

        return false;
    }

    juce::Array<juce::String> getLocalIPv4Addresses() {
        juce::Array<juce::String> result;
        auto addresses = juce::IPAddress::getAllAddresses();

        for (auto& addr : addresses) {
            if (!addr.isIPv6 && !isLoopback(addr))
                result.add(addr.toString()); // false = no port
        }

        return result;
    }

    void resolveResponse(Client& client, juce::String response) {
        juce::StringArray tokens;
        tokens.addTokens(response, ":", "");
        if (tokens.size() < 2) {
            DBG("Global Socket Handler tried to resolve a response but the response format was incorrect!");
            return;
        }
        int post;
        juce::String body = tokens[1];
        try {
            post = std::stoi(tokens[0].toStdString());
        } catch (std::exception) {
            DBG("Global Socket Handler tried to resolve a response but the response could not be parsed as an ID and body pair!");
            return;
        }
        if (client.authenticated == false) {
            if (post == COMMAND_AUTH) {
                if (body == socketCueResolver.getClientAuthPassword()) {
                    sendMessageToClient(client, RESPONSE_OK);
                    client.authenticated = true;
                    return;
                } else {
                    sendMessageToClient(client, RESPONSE_ERR);
                    return;
                }
            } else {
                DBG("User is not authenticated");
                sendMessageToClient(client, RESPONSE_ERR);
                return;
            }
        }
        auto clientHandle = client.socket->getRawSocketHandle();

        juce::MessageManager::callAsync([this, post, body, clientHandle]() {
            juce::String finalResponse = socketCueResolver.postCue(post, body);

            for (auto& client : clients) {
                if (client.socket->getRawSocketHandle() == clientHandle) { // ensure the client still exists before trying to send off the message.
                    sendMessageToClient(client, finalResponse);
                    break;
                }
            }
        });
    }

    void sendMessageToClient(Client& client, const juce::String& message) {
        if (client.socket == nullptr)
            return;

        if (!client.socket->isConnected()) {
            DBG("Cannot send message: client is not connected.");
            return;
        }

        auto messageData = message.toRawUTF8();
        int messageSize = static_cast<int>(std::strlen(messageData));

        int bytesWritten = client.socket->write(messageData, messageSize);

        if (bytesWritten != messageSize) {
            DBG("Failed to send complete message to client.");
            return;
        }

        DBG("Sent message to client: " << message);
    }
};