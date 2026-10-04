import { useState, useEffect } from 'react'
import styles from './VerticalSlider.module.css'

import { messageJUCE, JuceFunctionHandlers } from '../../services/juceHandlerService'

function VerticalSlider() {
    const [value, setValue] = useState(100)

    useEffect(() => {
        async function fetchSettings() {
            try {
                const initialWidth = await messageJUCE(JuceFunctionHandlers.getAudioSourceMasterScalar)
                if (initialWidth !== undefined)
                    setValue(Math.round(initialWidth * 100))
            } catch (err) {
                console.error("Failed to load settings from JUCE:", err)
            }
        }

        fetchSettings()
    }, [])

    const updateState = (newValue) => {
        setValue(newValue)
        messageJUCE(JuceFunctionHandlers.setAudioSourceMasterScalar, newValue / 100.0)
    }

    return (
        <div className={styles.container}>
            <input
                className={styles.slider}
                type="range"
                min="0"
                max="200"
                value={value}
                onChange={(e) => updateState(e.target.value)}
            />

            <span className={styles.value}>{value} %</span>
        </div>
    )
}

export default VerticalSlider