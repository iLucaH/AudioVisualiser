import { useState } from 'react'
import styles from './VerticalSlider.module.css'

function VerticalSlider() {
    const [value, setValue] = useState(100)

    const updateState = (newValue) => {
        setValue(newValue)
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