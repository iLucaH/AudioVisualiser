/*
  ==============================================================================

    GlobalSocketHandler.h
    Created: 13 Mar 2026 5:50:33pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class GlobalSocketHandler {
public:
    GlobalSocketHandler() {
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
            while (true) {
                if (!running.exchange(true)) {
                    break;
                }
                std::unique_ptr<juce::StreamingSocket> clientSocket(serverSocket.waitForNextConnection());
                if (clientSocket) {
                    DBG("Client connected! " << clientSocket->getRawSocketHandle());
                    clients.push_back(std::move(clientSocket));
                }

                for (auto& client : clients) {
                    if (client->waitUntilReady(true, 0)) {
                        char buffer[1024];
                        int bytesRead = client->read(buffer, sizeof(buffer), true);
                        if (bytesRead > 0) {
                            juce::String received(buffer, bytesRead);
                            DBG("Global socket handler received data: " << received << " from client: " << client->getRawSocketHandle() << ".");
                            resolveResponse(received);
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
            client.get()->close();
        }
    }

    juce::String getConnectionHandle() {
        auto addresses = getLocalIPv4Addresses();
        if (addresses.size() == 0 || port == -1)
            return "";
        return addresses.getFirst() + ":" + juce::String(port);
    }

private:
    int port = -1;

    std::atomic<bool> running{ false };

    juce::StreamingSocket serverSocket;
    std::vector<std::unique_ptr<juce::StreamingSocket>> clients;

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

    void resolveResponse(juce::String response) {
        juce::StringArray tokens;
        tokens.addTokens(response, ":", "");
        if (tokens.size() == 0) {
            DBG("Global Socket Handler tried to resolve a response but the response format was incorrect!");
            return;
        }
        int post, body;
        try {
            post = std::stoi(tokens[0].toStdString());
            body = std::stoi(tokens[1].toStdString());
        } catch (std::exception) {
            DBG("Global Socket Handler tried to resolve a response but the response could not be parsed as an ID and body pair!");
            return;
        }
        juce::MessageManager::callAsync([this, post, body]() {
            // plugin editor can handle the request from here on the message thread.
        });
    }
};