import { useState, useEffect } from 'react'

import DropDownMenu from '../../shared/DropDownMenu'
import Slider from './slider/Slider'

import { JuceFunctionHandlers, messageJUCE } from '../../services/juceHandlerService'

import styles from './Effects.module.css'

export default function Effects () {
    
    const [messages, setMessages] = useState({})
    const setMessage = (name, message) => setMessages(previous => ({ ...previous, [name]: message }))
    const getMessage = (name) => messages[name] ?? ''

    const [ effects, setEffects ] = useState([{ name: 'Loading...', id: -1 }])
    const [ globalSwitchState, setGlobalSwitchState ] = useState(true)

    const [ prioritySliderValues, setPrioritySliderValues ] = useState({})
    const getPrioritySliderValue = (name, defaultt = 0) => prioritySliderValues[name] ?? defaultt
    const handlePriorityDrag = (effect, value) => {
        setPrioritySliderValues(previous => ({ ...previous, [effect.id]: value }))
    }

    // Called once on release: send to JUCE
    const handlePriorityCommit = async (effect, value) => {
        const success = await messageJUCE(
            JuceFunctionHandlers.effectUpdate,
            effect.name, effect.id, value, effect.enabled
        )
        if (success) {
            setEffects(previous => previous.map(e => e.id === effect.id ? { ...e, priority: value } : e))
        } else {
            setMessage(effect.id, "There was an error updating the priority of this effect!")
        }
    }

    useEffect(() => {
        async function fetchSettings() {
            try {
                const response = await messageJUCE(JuceFunctionHandlers.effectsGetAll)
                setEffects(response)

                const allEnabled = await messageJUCE(JuceFunctionHandlers.effectGlobalEnabled)
                setGlobalSwitchState(allEnabled)
            } catch (err) {
                console.error("Failed to load effects from JUCE:", err)
            }
        }
        fetchSettings()
    }, [])

    return (
        <div>
            <div className={styles.buttonRow}>
                PostProcessing:
                <button type="button" onClick={async () => {
                    const newState = await messageJUCE(JuceFunctionHandlers.switchPostProcessorEnabled)
                    setGlobalSwitchState(newState)
                }} disabled={globalSwitchState}>On</button>
                <button type="button" onClick={async () => {
                    const newState = await messageJUCE(JuceFunctionHandlers.switchPostProcessorEnabled)
                    setGlobalSwitchState(newState)
                }} disabled={!globalSwitchState}>Off</button>
            </div>

            <div style={{ 
                height: 0,
                width: '100%',
                borderTop: '3px solid black',
                marginTop: '10px',
                marginBottom: '10px',
            }}/>
            <div className={styles.content}>
                {effects.map((effect) => (
                    <div key={effect.id}>
                        <DropDownMenu title={effect.name} open={false}>
                            <div>
                                {getMessage(effect.id)}
                            </div>
                            <div className={styles.effectName}>
                                Name: {effect.name}
                            </div>
                            <div className={styles.effectPriority}>
                                Priority:
                                <Slider
                                    value={getPrioritySliderValue(effect.id, effect.priority)}
                                    onChange={(val) => handlePriorityDrag(effect, val)}
                                    onCommit={(val) => handlePriorityCommit(effect, val)}
                                />
                            </div>
                            <div className={styles.effectEnabled}>
                                Enabled: 
                                <button type="button" onClick={async () => {
                                    const success = await messageJUCE(JuceFunctionHandlers.effectUpdate, effect.name, effect.id, effect.priority, true)
                                    if (success) {
                                        setEffects(previous => previous.map(e => e.id === effect.id ? { ...e, enabled:true } : e))
                                    } else {
                                        setMessage(effect.id, "There was an error enabling this effect!")
                                    }
                                }} disabled={effect.enabled}>On</button>
                                <button type="button" onClick={async () => {
                                    const success = await messageJUCE(JuceFunctionHandlers.effectUpdate, effect.name, effect.id, effect.priority, false)
                                    if (success) {
                                        setEffects(previous => previous.map(e => e.id === effect.id ? { ...e, enabled:false } : e))
                                    } else {
                                        setMessage(effect.id, "There was an error enabling this effect!")
                                    }
                                }} disabled={!effect.enabled}>Off</button>
                            </div>
                            <button onClick={async () => {
                                setMessage(effect.id, await messageJUCE(JuceFunctionHandlers.effectUpdate, effect.name, effect.id, effect.priority, effect.enabled) 
                                    ? effect.name + " was updated!" 
                                    : "Failed to update " + effect.name)
                            }}>Update</button>
                        </DropDownMenu>
                    </div>
                ))}
            </div>
        
        </div>
    )
}