import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import VerticalSlider from './slider/VerticalSlider'

function AudioPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Track Playback" open={true}>
                    <p>Width</p>
                    <p>Height</p>
                    <p>Fullscreen</p>
                </DropDownMenu>
                <DropDownMenu title="Audio Master" open={true}> 
                    <VerticalSlider />
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default AudioPage;
