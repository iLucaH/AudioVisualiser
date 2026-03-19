/*
  ==============================================================================

    AVAPIResolver.h
    Created: 18 Feb 2026 4:35:31pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

#define REGISTER_API_ERROR -1
#define REGISTER_SUCCESS 0
#define REGISTER_INVALID_USERNAME_NULL 1
#define REGISTER_INVALID_USERNAME_MIN_CHARS 2
#define REGISTER_INVALID_USERNAME_MAX_CHARS 3
#define REGISTER_INVALID_PASSWORD_NULL 4
#define REGISTER_USERNAME_TAKEN 5
#define REGISTER_PASSWORD_UNSAFE 6

struct RenderStateStruct {
    int id;
    juce::String name;
    juce::String renderState;
};

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

// Blocking operation.
// Returns uuid for the new render state if successful, otherwise returns an empty string.
inline juce::String postAddRenderState(const juce::String& jwt, const juce::String& name, const juce::String& renderState) {
    juce::var postBodyJson = new juce::DynamicObject();
    postBodyJson.getDynamicObject()->setProperty("name", name);
    postBodyJson.getDynamicObject()->setProperty("renderState", renderState);

    juce::URL url("http://localhost:8080/renderState/add");

    url = url.withPOSTData("jsonrsbody=" + juce::URL::addEscapeChars(juce::JSON::toString(postBodyJson, true), true));

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inPostData)
        .withHttpRequestCmd("POST")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to add a renderState!");
        return "";
    }

    juce::String response = stream->readEntireStreamAsString();

    if (response.length() == 0) {
        DBG("Failed to receive an API call render state id Response! Status code: " << statusCode);
        return "";
    }
    DBG("API post add render state id response resolved to: " << response);
    return response;
}

// Blocking operation.
inline std::vector<struct RenderStateStruct> getGetAllRenderStates(const juce::String& jwt) {
    juce::URL url("http://localhost:8080/renderState/getAll");

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("GET")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to get all renderStates!");
        return {};
    }

    juce::String response = stream->readEntireStreamAsString();
    DBG("A response for getAllRenderStates has been received as stream string: " << response);

    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return {};
    }

    juce::var parsed = juce::JSON::parse(response);
    if (parsed.isVoid()) {
        DBG("Failed to parse API JSON Prompt Response! Status code: " << statusCode);
        return {};
    }
    if (!parsed.isArray()) {
        DBG("The get all render states response was not an array!");
        return {};
    }
	std::vector<struct RenderStateStruct> renderStates;
    auto* arr = parsed.getArray();
    for (auto& rs : *arr) {
        auto* obj = rs.getDynamicObject();

        int id_parsed;
        juce::String name_parsed;
        juce::String renderState_parsed;

        if (obj == nullptr) {
            DBG("API JSON get all id Response is a nullptr! Status code: " << statusCode);
            DBG("parsed: " << rs.toString());
            continue;
        }
        if (obj->getProperty("id").isVoid()) {
            DBG("No success status could be resolved from API JSON get all Response! Status code: " << statusCode);
            DBG("parsed: " << rs.toString());
            continue;
        }
        juce::var id = obj->getProperty("id");
        if (!id.isInt()) {
            DBG("No id could be resolved from API JSON get all Response! Status code: " << statusCode);
            DBG("parsed: " << rs.toString());
            continue;
        }
        id_parsed = std::atoi(id.toString().toRawUTF8());

        if (obj->getProperty("name").isVoid()) {
            DBG("No name could be resolved from API JSON get all Response! Status code: " << statusCode);
            DBG("parsed: " << rs.toString());
            continue;
        }
        juce::var name = obj->getProperty("name");
        name_parsed = name.toString();

        if (obj->getProperty("renderState").isVoid()) {
            DBG("No renderState could be resolved from API JSON get all Response! Status code: " << statusCode);
            DBG("name parsed: " << name_parsed << " id parsed: " << id_parsed);
            continue;
        }
        juce::var renderState = obj->getProperty("renderState");
        renderState_parsed = renderState.toString();

        renderStates.push_back({
            .id = id_parsed,
            .name = name_parsed,
            .renderState = renderState_parsed
            });
    }
    return renderStates;
}

// Blocking operation.
inline struct RenderStateStruct getGetRenderState(const juce::String& jwt, int renderStateId) {
    juce::URL url("http://localhost:8080/renderState/get");
    url = url.withPOSTData("id=" + renderStateId);

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("GET")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to get a renderState with id: " << renderStateId);
        return {};
    }

    juce::String response = stream->readEntireStreamAsString();

    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return {};
    }

    juce::var parsed = juce::JSON::parse(response);
    if (parsed.isVoid()) {
        DBG("Failed to parse API JSON Prompt Response! Status code: " << statusCode);
        return {};
    }
    int id_parsed;
    juce::String name_parsed;
    juce::String renderState_parsed;

    juce::DynamicObject* obj = parsed.getDynamicObject();
    if (obj == nullptr) {
        DBG("API JSON get rs Response is a nullptr! Status code: " << statusCode);
        return {};
    }
    if (obj->getProperty("id").isVoid()) {
        DBG("No success status could be resolved from API JSON get rs Response! Status code: " << statusCode);
        return {};
    }
    juce::var id = obj->getProperty("id");
    if (!id.isInt()) {
        DBG("No id could be resolved from API JSON get rs Response! Status code: " << statusCode);
        return {};
    }
    id_parsed = std::atoi(id.toString().toRawUTF8());

    if (obj->getProperty("name").isVoid()) {
        DBG("No name could be resolved from API JSON get rs Response! Status code: " << statusCode);
        return {};
    }
    juce::var name = obj->getProperty("name");
    name_parsed = name.toString();

    if (obj->getProperty("renderState").isVoid()) {
        DBG("No renderState could be resolved from API JSON get rs Response! Status code: " << statusCode);
        return {};
    }
    juce::var renderState = obj->getProperty("renderState");
    renderState_parsed = renderState.toString();

    return {
        .id = id_parsed,
        .name = name_parsed,
        .renderState = renderState_parsed
        };
}

// Blocking operation.
inline int deleteDeleteRenderState(const juce::String& jwt, int renderStateId) {
    juce::URL url("http://localhost:8080/renderState/delete");
    url = url.withPOSTData("id=" + renderStateId);

    int statusCode = 0;

    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("DELETE")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);

    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to delete a renderState with id: " << renderStateId);
        return {};
    }

    juce::String response = stream->readEntireStreamAsString();

    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return {};
    }

    int status = std::atoi(response.toRawUTF8());

    return status;
}

inline void deleteDeleteAllRenderStates(const juce::String& jwt) {
    juce::URL url("http://localhost:8080/renderState/deleteAll");
    int statusCode = 0;
    auto options = juce::URL::InputStreamOptions(
        juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("DELETE")
        .withConnectionTimeoutMs(120000)
        .withExtraHeaders("Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer " + jwt)
        .withStatusCode(&statusCode);
    auto stream = url.createInputStream(options);
    if (stream == nullptr) { // Stream may timeout or the service may be unreachable.
        DBG("URL stream is null trying to delete all renderStates!");
        return;
    }
    juce::String response = stream->readEntireStreamAsString();
    if (response.length() == 0) {
        DBG("Failed to receive an API JSON Prompt Response! Status code: " << statusCode);
        return;
    }
}