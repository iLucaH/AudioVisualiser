import { useState, useEffect } from 'react'

import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import styles from './ExportPage.module.css'

import { messageJUCE, JuceFunctionHandlers } from '../services/juceHandlerService'

function ExportPage() {
    const [recording, setRecording] = useState(false)
    const [outputPath, setOutputPath] = useState('testPath')
    
    const [recordings, setRecordings] = useState(['Loading...'])

    useEffect(() => {
        async function fetchSettings() {
            try {
                const initialRecordings = await messageJUCE(JuceFunctionHandlers.recordingList)
                if (initialRecordings)
                    setRecordings(initialRecordings)
                const recordingState = await messageJUCE(JuceFunctionHandlers.getRecordingState)
                if (recordingState)
                    setRecording(recordingState)
                const outputPath = await messageJUCE(JuceFunctionHandlers.getRecordingOutputpath)
                if (outputPath)
                    setOutputPath(outputPath)
            } catch (err) {
                console.error("Failed to load settings from JUCE:", err)
            }
        }

        fetchSettings()
    }, [])

    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Recorder" open={true}>
                    <div className={styles.buttonRow}>
                        <button type="button" onClick={() => {
                            setRecording(true)
                            messageJUCE(JuceFunctionHandlers.recordingStart)
                        }} disabled={recording}>Start</button>
                        <button type="button" onClick={() => {
                            setRecording(false)
                            messageJUCE(JuceFunctionHandlers.recordingStop)
                        }} disabled={!recording}>Stop</button>
                    </div>

                    <p className={styles.status}>Recording: {recording ? 'Yes' : 'No'}</p>

                    <div className={styles.pathRow}>
                        <label htmlFor="outputPath">Output Path:</label>
                        <input id="outputPath" type="text" value={outputPath} readOnly />
                        <button type="button" onClick={ () => messageJUCE(JuceFunctionHandlers.setRecordingOutputpath) }>Change</button>
                    </div>
                </DropDownMenu>

                <DropDownMenu title="Recordings" open={true}>
                    {recordings.length === 0 ? (
                        <p className={styles.empty}>No recordings yet.</p>
                    ) : (
                        <ul className={styles.list}>
                            {recordings.map(nameRecording => (
                                <li key={nameRecording}>{nameRecording}</li>
                            ))}
                        </ul>
                    )}
                </DropDownMenu>
            </AVRootComponent>
        </div>
    )
}

export default ExportPage