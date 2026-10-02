import Preset from './Preset'
import { useState } from 'react'

import { submitPromptHandler, getFromNativeFunction } from '../../services/juceHandlerService'

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
                {type: 'button', label: 'Generate Design', clickhandler:'testclickHandler2', link: 'Prompt'}
            ]
        }
    ]
    const [selectedValue, setSelectedValue] = useState(options[0]);

    const [inputValues, setInputValues] = useState({})

    const handleInputChange = (key, value) => {
        setInputValues(previous => ({ ...previous, [key]: value }))
    }

    return (
        <div>
            <Preset options={options} selectedValue={selectedValue} setSelectedValue={setSelectedValue} />
            <p>Preset: {selectedValue.value}</p>
            {selectedValue.subcontent.map((subcontentItem, index) => (
                subcontentItem.type === 'button' ? (
                    <button key={index} onClick={() => {
                        if (inputValues[subcontentItem.link]) {
                            const value = inputValues[subcontentItem.link]
                            getFromNativeFunction(submitPromptHandler, value)
                        }
                    }}>
                        {subcontentItem.label}
                    </button>
                ) : subcontentItem.type === 'textfieldlong' ? (
                    <div key={index}>
                        <label>{subcontentItem.label}</label>
                        <input type="text" onChange={(e) => handleInputChange(subcontentItem.label, e.target.value)} />
                    </div>
                ) : null
            ))}
            <p>Child 3</p>
            <p>Child 1</p>
            <p>Child 2</p>
            <p>Child 3</p>
            <p>Child 1</p>
            <p>Child 2</p>
            <p>Child 3</p>
            <p>Ella</p>
            <p>Luca</p>
            <p>Gumbo</p>
            <p>Child 1</p>
            <p>Child 2</p>
            <p>Child 3</p>
        </div>
    )
}

export default Presets