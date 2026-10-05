import { useState, useRef } from 'react'

import { toHex, fromHex } from '../../../tool/colour'

export default function ColourPicker({ value, setValue }) {
    const [hex, setHex] = useState(value ? toHex(value) : '#000000')
    const latest = useRef(hex)
    const timer = useRef(null)

    if (!value) return null

    const handleChange = (e) => {
        const next = e.target.value
        setHex(next)
        latest.current = next

        if (timer.current)
            return
        timer.current = setTimeout(() => {
            timer.current = null
            setValue(fromHex(latest.current))
        }, 50)
    }

    return (
        <div>
            <input
                type="color"
                value={hex}
                onChange={handleChange}
                style={{
                    border: '2px solid #10b981',
                    borderRadius: '5px',
                    padding: '0',
                    background: 'transparent',
                    cursor: 'pointer'
                }}
            />
        </div>
    )
}