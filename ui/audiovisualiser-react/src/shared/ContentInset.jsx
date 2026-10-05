import styles from './Shared.module.css'

export default function ContentInset({expanded = false, children}) {
    return (
        <div className={expanded ? styles.contentInsetExpanded : styles.contentInset}>
            {children}
        </div>
    )
}