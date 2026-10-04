import * as Juce from './juce/index.js'

const nativeFunctionHandle = Juce.getNativeFunction('nativeFunctionMessage')

export const JuceFunctionHandlers = {
    newPresetPromiseEvent: 'send.visualiser.preset.newpreset',
    getCurrentPreset: 'receive.preset.current',
    getPresets: 'receive.preset.getall',
    setPreset: 'receive.preset.set',
    getSelectorOptions: 'receive.preset.selector.options',

    getOpenWebsite: 'receive.qr.opensite',

    setAuthTokenAlreadyExists: 'receive.auth.token.already.exists',
    registerNewUser: 'receive.register.new.user',
    loginUser: 'receive.login.user',
    loginPromiseEvent: 'send.login.complete',
    registerPromiseEvent: 'send.register.complete',
    
    getSettingsWidth: 'settings.width.get',
    setSettingsWidth: 'settings.width.set',
    getSettingsHeight: 'settings.height.get',
    setSettingsHeight: 'settings.height.set',
    getSettingsFullscreen: 'settings.fullscreen.get',
    setSettingsFullscreen: 'settings.fullscreen.set',

    getSettingsFFTSize: 'settings.fftsize.get',
    setSettingsFFTSize: 'settings.fftsize.set',

    getSettingsSocketPassword: 'settings.socketpassword.get',
    setSettingsSocketPassword: 'settings.socketpassword.set',
    
    settingsUpdatedPromiseEvent: 'send.settings.updated',
    
    audioSourceOpen: 'audio.source.open',
    setAudioPlaying: 'audio.play',
    setAudioStopping: 'audio.stop',
    getAudioSourceMasterScalar: 'audio.source.master.scalar.get',
    setAudioSourceMasterScalar: 'audio.source.master.scalar.set',

    recordingStart: 'recording.start',
    recordingStop: 'recording.stop',
    getRecordingState: 'recording.state',
    getRecordingOutputpath: 'recording.outputpath.get',
    setRecordingOutputpath: 'recording.outputpath.set',
    recordingList: 'recording.list',

    effectsGetAll: 'receive.effect.getall',
    switchPostProcessorEnabled: 'receive.effect.postprocessor.enabled',
    effectUpdate: 'receive.effect.update',
    effectGlobalEnabled: 'receive.effect.global.enabled',

    globalUpdateAll: 'send.global.socket.update.all'

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