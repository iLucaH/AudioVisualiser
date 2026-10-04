import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import VerticalSlider from './slider/VerticalSlider'

import styles from './AudioPage.module.css'

import { messageJUCE, JuceFunctionHandlers } from '../services/juceHandlerService'

function AudioPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Track Playback" open={true}>
                    <div className={styles.playBackContainer}>
                        <button onClick={() => { messageJUCE(JuceFunctionHandlers.audioSourceOpen) }}>Open</button>
                        <button onClick={() => { messageJUCE(JuceFunctionHandlers.setAudioPlaying) }}>Play</button>
                        <button onClick={() => { messageJUCE(JuceFunctionHandlers.setAudioStopping) }}>Stop</button>
                    </div>
                </DropDownMenu>
                <DropDownMenu title="Audio Master" open={true}> 
                    <VerticalSlider />
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default AudioPage;
