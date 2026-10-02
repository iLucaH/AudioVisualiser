import styles from './Shared.module.css'
import { FiChevronDown } from 'react-icons/fi'

export default function NativeSelector({options, selectedValue, setSelectedValue}) {

    const handleChange = (event) => {
        const selectedOption = options.find(
            option => option.value === event.target.value
        );

        setSelectedValue(selectedOption);
    };

    return (
        <div className={styles.selectContainer}>
            <select className={styles.select} value={selectedValue.value} onChange={handleChange}>
                {options.map((option) => (
                    <option key={option.key} value={option.value}>
                        {option.value}
                    </option>
                ))}
            </select>

            <FiChevronDown className={styles.selectArrow} />
        </div>
    );
}