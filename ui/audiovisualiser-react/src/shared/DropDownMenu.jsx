import { useState } from 'react';

import styles from './Shared.module.css'

import { FiChevronDown } from 'react-icons/fi';

function DropDownMenu({title, open = true, children}) {
    const [isOpen, setIsOpen] = useState(open);

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