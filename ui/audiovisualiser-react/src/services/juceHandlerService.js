import * as Juce from './juce/index.js'

export const registerHandler = {
    nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionRegister'),
    nativeEventHandle: "onRegisterEvent"
}

export const loginHandler = {
    nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionLogin'),
    nativeEventHandle: "onLoginEvent"
}

export const openWebsiteHandler = { nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionOpenWebsite') }

export const getSettingsHandler = { nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionGetSettings') }

export const changeSettingsHandler = { nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionChangeSettings') }

export const getSocketHandleHandler = { nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionGetSocketHandle') }

export const submitPromptHandler = { nativeFunctionHandle: Juce.getNativeFunction('nativeFunctionPromptSubmit') }

export function getFromNativeFunction(handler, ...args) {
    return handler.nativeFunctionHandle(...args)
}

export function waitForNativeEvent(handler) {
    return new Promise((resolve) => {
        const handle = (data) => {
            window.__JUCE__.backend.removeEventListener(handler.nativeEventHandle, handle)
            resolve(data)
        }
        window.__JUCE__.backend.addEventListener(handler.nativeEventHandle, handle)
    })
}

// Usage example:
// const eventPromise = waitForNativeEvent(registerHandler)

// const result = await getFromNativeFunction(
//     registerHandler,
//     username,
//     password
// )

// Evaluate result before awaiting promise because
// you may not have to wait for a response.

// const status = await eventPromise