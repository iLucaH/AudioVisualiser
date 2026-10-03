import Preset from './Preset'
import {NativeSelectorFromBackened} from '../shared/OptionSelector'
import { useState, useEffect } from 'react'

import styles from './Presets.module.css'

import { messageJUCE, JuceFunctionHandlers } from '../../services/juceHandlerService'

function Presets() {
    const [presets, setPresets] = useState([
        {
            value: 'Loading...',
            key: '0',
            subcontent: []
        }
    ])

    useEffect(() => {
        const fetchPresets = async () => {
            const result = await messageJUCE(JuceFunctionHandlers.getPresets)
            setPresets(result)
        }

        fetchPresets()
    }, [])

    console.log(presets)
    
    // Test Options
    // const options = [
    //     {
    //         value: 'preset1', 
    //         key: '0', 
    //         subcontent: [
    //             {type: 'button', label: 'buttontest', clickhandler:'testclickHandler'},
    //             {type: 'button', label: 'buttontest2', clickhandler:'testclickHandler2'}
    //         ]
    //     },
    //     { 
    //         value: 'preset2', 
    //         key: '1', 
    //         subcontent: [
    //             {type: 'button', label: 'buttontest', clickhandler:'testclickHandler'},
    //         ]
    //     },
    //     { 
    //         value: 'AI Generator', 
    //         key: '2', 
    //         subcontent: [
    //             {type: 'textfieldlong', label: 'Prompt', clickhandler:'submitPrompt'},
    //             {type: 'button', label: 'Generate Design', clickhandler:'submitPrompt', link: 'Prompt'},
    //             {type: 'spacer', paddingTop: 15, paddingBottom: 15},
    //             {type: 'row', subcontent: [
    //                 {type: 'expandbutton', label: 'Save Preset', subcontent: [
    //                     {type: 'button', label: 'To File', clickhandler:'testclickHandler2', link: 'Prompt'},
    //                     {type: 'expandbutton', label: 'To Account', subcontent: [
    //                         {type: 'textfieldshort', label: 'Name your preset', clickhandler:'submitPreset'},
    //                         {type: 'button', label: 'Save', clickhandler:'submitPreset', link: 'Name your preset'},
    //                     ]}
    //                 ]},
    //                 {type: 'expandbutton', label: 'Load Preset', subcontent: [
    //                     {type: 'button', label: 'From File', clickhandler:'testclickHandler2', link: 'Prompt'},
    //                     {type: 'expandbutton', label: 'From Account', subcontent: [
    //                         {type: 'picker', label: 'Select Preset...', gethandle: 'getFromAccount', sethandle: 'setFromAccount'},                            
    //                     ]}
    //                 ]},
    //             ]},
    //         ]
    //     }
    // ]
    const [selectedValue, setSelectedValue] = useState(presets[0]);

    const [inputValues, setInputValues] = useState({})
    const [hiddenContent, setHiddenContent] = useState(null)

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
                    <button key={index} onClick={() => {
                        if (inputValues[subcontentItem.link]) {
                            const value = inputValues[subcontentItem.link]
                            
                            messageJUCE(subcontentItem.clickhandler, value)
                        }
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
                        <div style={{ 
                            height: 0,
                            width: '100%',
                            borderTop: '3px solid black',
                            marginTop: subcontentItem.paddingTop,
                            marginBottom: subcontentItem.paddingBottom,
                        }}/>
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
            <Preset options={presets} selectedValue={selectedValue} setSelectedValue={setSelectedValue} />
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