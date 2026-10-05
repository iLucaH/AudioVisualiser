import Slider from '../slider/Slider'
import ColourPicker from '../picker/ColourPicker'
import Switch from '../../shared/Switch'

import styles from './Uniform.module.css'

export default function Uniform({ uniformVar, updateEffect }) {

    const updateValue = (newValue) => {
        updateEffect(newValue)
    }

    return (
        <div className={styles.uniformRow}>
            {uniformVar.name}

            {uniformVar.type === 'floatinput' ? (
                <NumberUniform
                    value={uniformVar.value ?? 0}
                    setValue={updateValue}
                    isFloat={true}
                />
            ) : uniformVar.type === 'intinput' ? (
                <NumberUniform
                    value={uniformVar.value ?? 0}
                    setValue={updateValue}
                    isFloat={false}
                />
            ) : uniformVar.type === 'floatslider' ? (
                <NumberSlider
                    value={uniformVar.value ?? 0}
                    setValue={updateValue}
                    min={uniformVar.min ?? 0}
                    max={uniformVar.max ?? 100}
                    isFloat={true}
                />
            ) : uniformVar.type === 'intslider' ? (
                <NumberSlider
                    value={uniformVar.value ?? 0}
                    setValue={updateValue}
                    min={uniformVar.min ?? 0}
                    max={uniformVar.max ?? 100}
                    isFloat={false}
                />
            ) : uniformVar.type === 'rgbpicker' ? (
                <RGBPicker
                    value={uniformVar.value}
                    setValue={updateValue}
                />
            ) : uniformVar.type === 'boolean' ? (
                <BooleanButton
                    value={uniformVar.value ?? false}
                    setValue={updateValue}
                />
            ) : null}
        </div>
    )
}

function NumberUniform({ value, setValue, isFloat }) {
    return (
        <div>
            <input
                type="number"
                value={value}
                step={isFloat ? 0.01 : 1}
                onChange={(e) => {
                    const text = e.target.value

                    if (text === '') {
                        return
                    }

                    const newValue = isFloat
                        ? parseFloat(text)
                        : parseInt(text, 10)

                    if (!Number.isNaN(newValue)) {
                        setValue(newValue)
                    }
                }}
                onKeyDown={(e) => {
                    if (e.key === 'Enter') {
                        e.currentTarget.blur()
                    }
                }}
            />
        </div>
    )
}

function NumberSlider({ value, setValue, min, max, isFloat }) {
    return (
        <div>
            <Slider
                value={value}
                onChange={setValue}
                min={min}
                max={max}
                step={isFloat ? 0.01 : 1}
                toFixed={isFloat ? 2 : 0}
            />
        </div>
    )
}

function RGBPicker({ value, setValue }) {
    return (
        <div>
            <ColourPicker value={value} setValue={setValue} />
        </div>
    )
}

function BooleanButton({ value, setValue }) {
    return (
        <div>
        <Switch
            label=""
            state={value}
            onToggle={() => setValue(!value)}
            inRow={false}
        />
            {/* <button type="button" onClick={async () => {
                setValue(true)
            }} disabled={value}>On</button>
            <button type="button" onClick={async () => {
                setValue(false)
            }} disabled={!value}>Off</button> */}
        </div>
    )
}