import Preset from './Preset'
import { NativeSelectorFromBackened } from '../shared/OptionSelector'
import { useState, useEffect } from 'react'
import { useApp } from '../../AppContext'

import styles from './Presets.module.css'

import { messageJUCE, JuceFunctionHandlers, waitForNativeEvent } from '../../services/juceHandlerService'

function Presets() {
    const [presets, setPresets] = useState([
        {
            value: 'Loading...',
            key: '0',
            subcontent: []
        }
    ])

    const fetchPresets = async () => {
        const result = await messageJUCE(JuceFunctionHandlers.getPresets)
        setPresets(result)
        const currentPreset = await messageJUCE(JuceFunctionHandlers.getCurrentPreset)
        const selected = result.find(item => item.key === currentPreset);

        setSelectedValue(selected);
    }

    useEffect(() => {
        // Fetch presets when the component starts
        fetchPresets()

        let cancelled = false

        const listenForNewPresets = async () => {
            while (!cancelled) {
                await waitForNativeEvent(JuceFunctionHandlers.newPresetPromiseEvent)
                if (!cancelled) {
                    await fetchPresets()
                }
            }
        }
        listenForNewPresets()
        return () => { cancelled = true } // Stop listening when the component is unmounted
    }, [])
    
    const { selectedValue, setSelectedValue } = useApp()
    // Only select a preset if one isn't already selected
    useEffect(() => {
        if (selectedValue === null && presets.length > 0) {
            setSelectedValue(presets[0])
        }
    }, [presets, selectedValue, setSelectedValue])

    const [inputValues, setInputValues] = useState({})
    const [hiddenContent, setHiddenContent] = useState(null)
    const [informationMessage, setInformationMessage] = useState("")

    const handleInputChange = (key, value) => {
        setInputValues(previous => ({ ...previous, [key]: value }))
    }

    const toggleHiddenContent = (subcontent) => {
        setHiddenContent(subcontent.label === hiddenContent?.label ? null : subcontent)
    }

    const evaluateOption = (subcontent) => {
        if (subcontent == null) {
            return null;
        }
        return subcontent.map((subcontentItem, index) => {
            return (
                subcontentItem.type === 'button' ? (
                    <button key={index} onClick={async () => {
                        const value = inputValues[subcontentItem.link]
                        setInformationMessage(value ? await messageJUCE(subcontentItem.clickhandler, value) : await messageJUCE(subcontentItem.clickhandler))
                    }}>
                        {subcontentItem.label}
                    </button>
                ) : subcontentItem.type === 'textfieldlong' ? (
                    <div key={index}>
                        <label>{subcontentItem.label}</label>
                        <textarea
                            className={styles.textfieldlong}
                            onChange={(e) =>
                                handleInputChange(subcontentItem.label, e.target.value)
                            }
                        />
                    </div>
                ) : subcontentItem.type === 'textfieldshort' ? (
                    <div key={index}>
                        <label>{subcontentItem.label}</label>
                        <textarea
                            className={styles.textfieldshort}
                            onChange={(e) =>
                                handleInputChange(subcontentItem.label, e.target.value)
                            }
                        />
                    </div>
                ) : subcontentItem.type === 'spacer' ? (
                    <div key={index}>
                        <div style={{ height: 0, width: '100%', borderTop: '1px solid #045f41', marginTop: subcontentItem.paddingTop, marginBottom: subcontentItem.paddingBottom }}/>
                    </div>
                ) : subcontentItem.type === 'row' ? (
                    <div key={index} className={styles.customrow}>
                        {evaluateOption(subcontentItem.subcontent)}
                    </div>
                ) : subcontentItem.type === 'expandbutton' ? (
                    <div key={index}>
                        <button key={index} onClick={() => { toggleHiddenContent(subcontentItem) }}>
                            {subcontentItem.label}
                        </button>
                    </div>
                ) : subcontentItem.type === 'waitingbutton' ? (
                    <button key={index} onClick={async () => {
                            const eventPromise = waitForNativeEvent(subcontentItem.waitingpromisehandle)
                            const value = inputValues[subcontentItem.link]

                            const response = value ? await messageJUCE(subcontentItem.clickhandler, value) : await messageJUCE(subcontentItem.clickhandler)
                            console.log(response)

                            setInformationMessage(response[1])
                            if (response[0] === false) {
                                return
                            }
                            const message = await eventPromise
                            setInformationMessage(message[0])
                        }}
                    >{subcontentItem.label}</button>
                ) : subcontentItem.type === 'picker' ? (
                    <div key={index}>
                        <NativeSelectorFromBackened getHandleName={subcontentItem.gethandle} setHandleName={subcontentItem.sethandle}/>
                    </div>
                ) : null
            )
        })
    }

    return (
        <div>
            <Preset options={presets} selectedValue={selectedValue} setSelectedValue={setSelectedValue} onChange={ () => {
                setHiddenContent(null)
                setInformationMessage("")
                } } />
            {informationMessage !== "" ? 
                <div className={styles.informationMessage}>
                    {informationMessage}
                </div>
            : null }
            {evaluateOption(selectedValue.subcontent)}
            {hiddenContent !== null ? (
                <div className={styles.hiddenContentBlock}>
                    {evaluateOption(hiddenContent?.subcontent)}
                    <button onClick={ () => {toggleHiddenContent(hiddenContent)} }>Cancel</button>
                </div>
            ) : null}
        </div>
    )
}

export default Presets