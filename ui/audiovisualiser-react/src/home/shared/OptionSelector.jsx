import styles from './Shared.module.css'
import { FiChevronDown } from 'react-icons/fi'
import { useEffect, useState } from 'react'

import { messageJUCE, JuceFunctionHandlers } from '../../services/juceHandlerService'

function NativeSelector({ options, selectedValue, setSelectedValue }) {

    const handleChange = async (event) => {
        const selectedOption = options.find(
            option => option.value === event.target.value
        )

        const goodToGo = await messageJUCE(JuceFunctionHandlers.setPreset, selectedOption.key)
        
        if (goodToGo === true) {
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

function NativeSelectorFromBackened({ getHandleName, setHandleName }) {

    const [options, setOptions] = useState(null)
    const [selectedValue, setSelectedValue] = useState(null)

    useEffect(() => {
        const getOptions = getNativeFunctionHandle(getHandleName)

        if (!getOptions) {
            setOptions([
                { value: 'Error', key: '0' },
                { value: 'Contact Administrator', key: '1' }
            ])
            return
        }

        const fetchOptions = async () => {
            try {
                const result = await getOptions()

                setOptions(result)
                setSelectedValue(result[0])
            } catch (error) {
                console.error('Failed to get options from JUCE:', error)

                const fallback = [
                    { value: 'Error', key: '0' },
                    { value: 'Contact Administrator', key: '1' }
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

        setSelectedValue(selectedOption)

        // Send the newly selected value to JUCE
        await messageJUCE(JuceFunctionHandlers.getSelectorOptions, selectedOption.value)
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