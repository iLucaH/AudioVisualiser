import { useId } from 'react'

import styles from './Slider.module.css'

export default function Slider ({
    value,
    onChange,
    onCommit,
    min = 0,
    max = 100,
    step = 1,
    label,
    disabled = false,
    showValue = true,
}) {
    const id = useId()
    const percent = ((value - min) / (max - min)) * 100

    const handleChange = (e) => onChange?.(Number(e.target.value))
    const handleCommit = (e) => onCommit?.(Number(e.currentTarget.value))

    return (
        <div className={styles.slider}>
            {label && <label htmlFor={id} className={styles.label}>{label}</label>}
            <input
                id={id}
                type="range"
                className={styles.input}
                min={min}
                max={max}
                step={step}
                value={value}
                disabled={disabled}
                onChange={handleChange}
                onPointerUp={handleCommit}
                onKeyUp={handleCommit}
                style={{ '--fill': `${percent}%` }}
            />
            {showValue && <span className={styles.value}>{value}</span>}
        </div>
    )
}