import * as Juce from './juce/index.js'

const nativeFunctionHandle = Juce.getNativeFunction('nativeFunctionMessage')

export const JuceFunctionHandlers = {
    getPresets: 'receive.preset.getall',
    setPreset: 'receive.preset.set',
    getSelectorOptions: 'receive.preset.selector.options',

    getOpenWebsite: 'receive.qr.opensite',

    registerNewUser: 'receive.register.new.user',
    loginUser: 'receive.login.user',
    loginPromiseEvent: 'send.login.complete',
    registerPromiseEvent: 'send.register.complete'
}

export function messageJUCE(eventName, ...args) {
    return nativeFunctionHandle(eventName, ...args);
}

export function waitForNativeEvent(handler) {
    return new Promise((resolve) => {
        const handle = (data) => {
            console.log("Received JUCE event:", handler, data)
            window.__JUCE__.backend.removeEventListener(handler, handle)
            resolve(data)
        }
        console.log("Waiting for JUCE event:", handler)
        window.__JUCE__.backend.addEventListener(handler, handle)
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