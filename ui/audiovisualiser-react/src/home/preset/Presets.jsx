import Preset from './Preset'
import { useState } from 'react'

import styles from './Presets.module.css'

import { getNativeFunctionHandle, getFromNativeFunction } from '../../services/juceHandlerService'

function Presets() {
    const options = [
        {
            value: 'preset1', 
            key: '0', 
            subcontent: [
                {type: 'button', label: 'buttontest', clickhandler:'testclickHandler'},
                {type: 'button', label: 'buttontest2', clickhandler:'testclickHandler2'}
            ]
        },
        { 
            value: 'preset2', 
            key: '1', 
            subcontent: [
                {type: 'button', label: 'buttontest', clickhandler:'testclickHandler'},
            ]
        },
        { 
            value: 'AI Generator', 
            key: '2', 
            subcontent: [
                {type: 'textfieldlong', label: 'Prompt', clickhandler:'submitPrompt'},
                {type: 'button', label: 'Generate Design', clickhandler:'testclickHandler2', link: 'Prompt'},
                {type: 'spacer', paddingTop: 15, paddingBottom: 15},
                {type: 'row', subcontent: [
                    {type: 'button', label: 'Save Preset', clickhandler:'testclickHandler2'},
                    {type: 'button', label: 'Load Preset', clickhandler:'testclickHandler2'},
                ]},
            ]
        }
    ]
    const [selectedValue, setSelectedValue] = useState(options[0]);

    const [inputValues, setInputValues] = useState({})

    const handleInputChange = (key, value) => {
        setInputValues(previous => ({ ...previous, [key]: value }))
    }

    const evaluateOption = (subcontent) => {
        return subcontent.map((subcontentItem, index) => (
            subcontentItem.type === 'button' ? (
                <button key={index} onClick={() => {
                    if (inputValues[subcontentItem.link]) {
                        const value = inputValues[subcontentItem.link]
                        getFromNativeFunction(getNativeFunctionHandle(subcontentItem.clickhandler), value)
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
            ) : null
        ))
    }

    return (
        <div>
            <Preset options={options} selectedValue={selectedValue} setSelectedValue={setSelectedValue} />
            {evaluateOption(selectedValue.subcontent)}
        </div>
    )
}

export default Presets