import { useState, useEffect } from 'react'

import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'
import styles from './SettingsPage.module.css'
import ContentInset from '../shared/ContentInset'

import { JuceFunctionHandlers, messageJUCE, waitForNativeEvent } from '../services/juceHandlerService'

const clamp = (value, min, max) => Math.min(max, Math.max(min, value))

function SettingsPage() {
    const [width, setWidth] = useState('1280')
    const [height, setHeight] = useState('720')
    const [fullscreen, setFullscreen] = useState(false)

    const [fftSize, setFftSize] = useState('9')

    const [socketPassword, setSocketPassword] = useState('')
    const [showPassword, setShowPassword] = useState(false)

    // Load initial settings state from JUCE on component mount
    useEffect(() => {
        async function fetchSettings() {
            try {
                const initialWidth = await messageJUCE(JuceFunctionHandlers.getSettingsWidth)
                if (initialWidth !== undefined) setWidth(String(initialWidth))

                const initialHeight = await messageJUCE(JuceFunctionHandlers.getSettingsHeight)
                if (initialHeight !== undefined) setHeight(String(initialHeight))

                const initialFullscreen = await messageJUCE(JuceFunctionHandlers.getSettingsFullscreen)
                if (initialFullscreen !== undefined) setFullscreen(Boolean(initialFullscreen))

                const initialFft = await messageJUCE(JuceFunctionHandlers.getSettingsFFTSize)
                if (initialFft !== undefined) setFftSize(String(initialFft))

                const initialPass = await messageJUCE(JuceFunctionHandlers.getSettingsSocketPassword)
                if (initialPass !== undefined) setSocketPassword(String(initialPass))
            } catch (err) {
                console.error("Failed to load settings from JUCE:", err)
            }
        }

        fetchSettings()

        let cancelled = false

        const listenForNewPresets = async () => {
            while (!cancelled) {
                await waitForNativeEvent(JuceFunctionHandlers.settingsUpdatedPromiseEvent)
                if (!cancelled) {
                    console.log("message received")
                    await fetchSettings()
                }
            }
        }
        listenForNewPresets()
        return () => { cancelled = true }
    }, [])

    // Input blur handlers with clamp & JUCE state sync
    const commitWidth = () => {
        const clampedVal = clamp(Number(width) || 1280, 100, 1920)
        setWidth(String(clampedVal))
        messageJUCE(JuceFunctionHandlers.setSettingsWidth, clampedVal)
    }

    const commitHeight = () => {
        const clampedVal = clamp(Number(height) || 720, 100, 1080)
        setHeight(String(clampedVal))
        messageJUCE(JuceFunctionHandlers.setSettingsHeight, clampedVal)
    }

    const handleToggleFullscreen = () => {
        setFullscreen(prev => {
            const nextState = !prev
            messageJUCE(JuceFunctionHandlers.setSettingsFullscreen, nextState)
            return nextState
        })
    }

    const handleFftChange = (e) => {
        const newSize = e.target.value
        setFftSize(newSize)
        messageJUCE(JuceFunctionHandlers.setSettingsFFTSize, Number(newSize))
    }

    const handleSocketPasswordChange = (e) => {
        const newPassword = e.target.value
        setSocketPassword(newPassword)
    }

    const commitSocketPassword = () => {
        messageJUCE(JuceFunctionHandlers.setSettingsSocketPassword, socketPassword)
    }

    return (
        <div>
            <AVRootComponent>

                {/* Rendering Settings */}
                <DropDownMenu title="Rendering Settings" open={true}>
                    <ContentInset>
                        <div className={styles.formRowParent}>
                            <div className={styles.formRow}>
                                <label>Width</label>
                                <input
                                    id="width"
                                    name="width"
                                    type="number"
                                    min="100"
                                    max="1920"
                                    step="2"
                                    placeholder="Width"
                                    value={width}
                                    onChange={(e) => setWidth(e.target.value)}
                                    onBlur={commitWidth}
                                />
                            </div>

                            <div className={styles.formRow}>
                                <label>Height</label>
                                <input
                                    id="height"
                                    name="height"
                                    type="number"
                                    min="100"
                                    max="1080"
                                    step="2"
                                    placeholder="Height"
                                    value={height}
                                    onChange={(e) => setHeight(e.target.value)}
                                    onBlur={commitHeight}
                                />
                            </div>

                            <div className={styles.formRow}>
                                <label htmlFor="fullscreen">Fullscreen:</label>
                                <button
                                    id="fullscreen"
                                    type="button"
                                    onClick={handleToggleFullscreen}
                                >
                                    Enable
                                </button>
                            </div>
                        </div>
                    </ContentInset>
                </DropDownMenu>


                {/* Audio Settings */}
                <DropDownMenu title="Audio Settings" open={false}>

                    <ContentInset>
                        <div className={styles.formRow}>
                            <label>FFT Size</label>
                            <select
                                id="fft"
                                name="fft"
                                value={fftSize}
                                onChange={handleFftChange}
                            >
                                <option value="9">512</option>
                                <option value="10">1024</option>
                                <option value="11">2048</option>
                                <option value="12">4096</option>
                            </select>
                        </div>
                    </ContentInset>

                </DropDownMenu>


                {/* System Settings */}
                <DropDownMenu title="System Settings" open={false}>
                    <ContentInset>
                        <label htmlFor="socketPassword">Socket Service Password</label>
                        <div className={styles.passwordContainer}>
                            <input
                                id="socketPassword"
                                type={showPassword ? 'text' : 'password'}
                                value={socketPassword}
                                onChange={handleSocketPasswordChange}
                                onBlur={commitSocketPassword}
                                className={styles.socketPassword}
                            />

                            <button
                                type="button"
                                onClick={() => setShowPassword(previous => !previous)}
                                className={styles.passwordButton}
                            >
                                {showPassword ? 'Hide' : 'Show'}
                            </button>
                        </div>
                    </ContentInset>
                </DropDownMenu>

            </AVRootComponent>
        </div>
    )
}

export default SettingsPage