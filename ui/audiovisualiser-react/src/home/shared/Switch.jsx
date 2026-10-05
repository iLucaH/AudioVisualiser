import styles from './Shared.module.css'

export default function Switch({ label, state, onToggle, inRow = true }) {

    const handleToggle = async () => {
        await onToggle(!state)
    }

    const switchButtons = (
        <div className={styles.switch}>
            <button
                className={state ? styles.switchEnabled : styles.switchDisabled}
                type="button"
                onClick={handleToggle}
                disabled={state}
            >
                On
            </button>

            <button
                className={!state ? styles.switchEnabled : styles.switchDisabled}
                type="button"
                onClick={handleToggle}
                disabled={!state}
            >
                Off
            </button>
        </div>
    )

    if (inRow) {
        return (
            <div className={styles.swtichRow}>
                {label}
                {switchButtons}
            </div>
        )
    }

    return switchButtons
}