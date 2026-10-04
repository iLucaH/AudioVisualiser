import { useApp } from '../AppContext'
import { useEffect } from 'react'

import styles from './Shared.module.css'

import { FiChevronDown } from 'react-icons/fi';

function DropDownMenu({title, open = true, children}) {
    const { dropdownSelectorOpen, setDropdownSelectorOpen } = useApp();
    useEffect(() => {
        const component = dropdownSelectorOpen.find(component => component.name === title)
        if (!component) {
            setDropdownSelectorOpen(previous => [
                ...previous, {
                    name: title,
                    open: open
                }
            ])
        }
    }, [dropdownSelectorOpen, setDropdownSelectorOpen])

    const setIsOpen = (open) => {
        setDropdownSelectorOpen(previous =>
            previous.map(component =>
                component.name === title
                    ? { ...component, open }
                    : component
            )
        )
    }

    const dropdown = dropdownSelectorOpen.find(component => component.name === title)
    const isOpen = dropdown?.open ?? open

    if (isOpen) {
        return (
            <div className={styles.dropdownOpen}>
                <div className={styles.dropdownContent} onClick={() => setIsOpen(false)}>
                    <button className={styles.dropdownButton}>
                        <FiChevronDown className={`${styles.arrowIcon} ${isOpen ? styles.rotate180 : ''}`} />
                    </button>
                    <div className={styles.dropdownTitle}>
                        <p>{title}</p>
                    </div>
                </div>
                <div className={styles.dropdownChildren}>
                    {children}
                </div>
            </div>
        )
    } else {
        return (
            <div className={styles.dropdownClosed}>
                <div className={styles.dropdownContent} onClick={() => setIsOpen(true)}>
                    <button className={styles.dropdownButton}>
                        <FiChevronDown className={`${styles.arrowIcon} ${isOpen ? styles.rotate180 : ''}`} />
                    </button>
                    <div className={styles.dropdownTitle}>
                        <p>{title}</p>
                    </div>
                </div>
            </div>
        )
    }
}

export default DropDownMenu