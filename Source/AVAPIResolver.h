/*
  ==============================================================================

    AVAPIResolver.h
    Created: 18 Feb 2026 4:35:31pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define REGISTER_API_ERROR -1
#define REGISTER_SUCCESS 0
#define REGISTER_INVALID_USERNAME_NULL 1
#define REGISTER_INVALID_USERNAME_MIN_CHARS 2
#define REGISTER_INVALID_USERNAME_MAX_CHARS 3
#define REGISTER_INVALID_PASSWORD_NULL 4
#define REGISTER_USERNAME_TAKEN 5
#define REGISTER_PASSWORD_UNSAFE 6

// Blocking operation.
inline juce::String postPromptResponse(const juce::String& jwt, const juce::String& prompt) {
    juce::URL url("http://localhost:8080/prompt");
    url = url.withPOSTData("prompt=" + prompt);

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("POST")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to input api prompt payload!");
        return "";
    }

    juce::String response = stream->readEntireStreamAsString();
    
    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return "";
    }

    juce::var parsed = juce::JSON::parse(response);
    if (parsed.isVoid()) {
        DBG("Failed to parse API JSON Prompt Response! Status code: " << statusCode);
        return "";
    }
    juce::DynamicObject* obj = parsed.getDynamicObject();
    if (obj == nullptr) {
        DBG("API JSON Prompt Response is a nullptr! Status code: " << statusCode);
        return "";
    }
    if (obj->getProperty("success").isVoid()) {
        DBG("No success status could be resolved from API JSON Prompt Response! Status code: " << statusCode);
        return "";
    }
    if (!obj->getProperty("success")) {
        DBG("A failed status was resolved from API JSON Prompt Response! Status code: " << statusCode);
        return "";
    }
    if (obj->getProperty("prompt").isVoid()) {
        DBG("No success status could be resolved from API JSON Prompt Response! Status code: " << statusCode);
        return "";
    }
    juce::String parsedResponse = obj->getProperty("prompt").toString();
    DBG("API JSON Prompt Response resolved to: " << parsedResponse);
    return parsedResponse;
}

inline juce::String api_login(const juce::String& username, const juce::String& password) {
    juce::URL url("http://localhost:8080/auth/token");

    juce::String credentials = username + ":" + password;
    juce::String encoded = juce::Base64::toBase64(credentials.toRawUTF8(),
        credentials.getNumBytesAsUTF8());

    juce::String headers = "Authorization: Basic " + encoded + "\r\n";

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("POST")
        .withExtraHeaders(headers)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    // Stream may timeout or the service may be unreachable.
    if (stream == nullptr) {
        DBG("URL stream is null trying to input api login payload!");
        return "";
    }
    DBG("login payload complete. Status code: " << statusCode);

    return stream->readEntireStreamAsString();
}

/*
    Returns:
        REGISTER_API_ERROR if error with API call or validation.
*/
inline int api_register(const juce::String& username, const juce::String& password) {
    juce::URL url("http://localhost:8080/auth/register");

    juce::String credentials = "username=" + juce::URL::addEscapeChars(username, true) + "&password=" + juce::URL::addEscapeChars(password, true);

    url = url.withPOSTData(credentials);

    juce::String headers = "Content-Type: application/x-www-form-urlencoded\r\n";

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inPostData)
        .withHttpRequestCmd("POST")
        .withExtraHeaders(headers)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);

    if (stream == nullptr) {
        DBG("Register request failed: stream is null");
        return REGISTER_API_ERROR;
    }

    juce::String response = stream->readEntireStreamAsString();
    DBG(response);

    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return REGISTER_API_ERROR;
    }

    juce::var parsed = juce::JSON::parse(response);
    if (parsed.isVoid()) {
        DBG("Failed to parse API JSON Prompt Response! Status code: " << statusCode);
        return REGISTER_API_ERROR;
    }
    juce::DynamicObject* obj = parsed.getDynamicObject();
    if (obj == nullptr) {
        DBG("API JSON Prompt Response is a nullptr! Status code: " << statusCode);
        return REGISTER_API_ERROR;
    }
    if (obj->getProperty("success").isVoid()) {
        DBG("No success status could be resolved from API JSON Prompt Response! Status code: " << statusCode);
        return REGISTER_API_ERROR;
    }
    juce::var parsedResponse = obj->getProperty("success");
    if (!parsedResponse.isInt()) {
        DBG("No success status could be resolved from API JSON Prompt Response! Status code: " << statusCode);
        return REGISTER_API_ERROR;
    }
    int success = std::atoi(parsedResponse.toString().toRawUTF8());

    return success;
}