import styles from './Shared.module.css'
import { FiChevronDown } from 'react-icons/fi'
import { useEffect, useState } from 'react'

import { messageJUCE, JuceFunctionHandlers } from '../../services/juceHandlerService'

function NativeSelector({ options, selectedValue, setSelectedValue, onChange = () => {} }) {

    const handleClick = async () => {
        messageJUCE(JuceFunctionHandlers.setPreset, selectedValue) // basically forcing the backend to force a new render state added check.
    }

    const handleChange = async (event) => {
        const selectedOption = options.find(
            option => option.value === event.target.value
        )

        const goodToGo = await messageJUCE(JuceFunctionHandlers.setPreset, selectedOption.key)
        
        if (goodToGo === true) {
            onChange(selectedOption)
            setSelectedValue(selectedOption)
        }
    }

    return (
        <div className={styles.selectContainer}>
            <select
                className={styles.select}
                value={selectedValue.value}
                onChange={handleChange}
                onClick={handleClick}
            >
                {options.map((option) => (
                    <option key={option.key} value={option.value}>
                        {option.value}
                    </option>
                ))}
            </select>

            <FiChevronDown className={styles.selectArrow} />
        </div>
    )
}

function NativeSelectorFromBackened({ getHandleName, setHandleName }) {

    const [options, setOptions] = useState(null)
    const [selectedValue, setSelectedValue] = useState(null)

    useEffect(() => {
        const fetchOptions = async () => {
            let success = false
            try {
                const result = await messageJUCE(getHandleName)
                console.log(result)
                if (result.length == 0) {
                    const fallback = [
                        { value: 'You have nothing saved...', key: -1 },
                    ]
                    setOptions(fallback)
                    setSelectedValue(fallback[0])
                } else {
                    setOptions(result)
                    setSelectedValue(result[0])
                }
                success = true
            } catch (error) {
                console.error('Failed to get options from JUCE:', error)
            }
            if (success === false) {
                const fallback = [
                    { value: 'Please log-in...', key: -1 },
                ]

                setOptions(fallback)
                setSelectedValue(fallback[0])
            }
        }

        fetchOptions()
    }, [getHandleName])


    if (!options || !selectedValue) {
        return <div>Loading...</div>
    }


    const handleChange = async (event) => {
        const selectedOption = options.find(
            option => option.value === event.target.value
        )

        // Send the newly selected value to JUCE
        if (selectedOption.key === -1) {
            return;
        }
        const result = await messageJUCE(setHandleName, selectedOption.key)
        if (result === true) {
            setSelectedValue(selectedOption)
        }
    }


    return (
        <div className={styles.selectContainer}>
            <select
                className={styles.select}
                value={selectedValue.value}
                onChange={handleChange}
            >
                {options.map((option) => (
                    <option key={option.key} value={option.value}>
                        {option.value}
                    </option>
                ))}
            </select>

            <FiChevronDown className={styles.selectArrow} />
        </div>
    )
}

export {
    NativeSelector,
    NativeSelectorFromBackened
}