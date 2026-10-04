import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import Presets from './preset/Presets'
import Effects from './effects/Effects'

function Homepage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Presets" open={true}>
                    <Presets />
                </DropDownMenu>
                <DropDownMenu title="Effects" open={false}>
                    <Effects />
                </DropDownMenu>                
            </AVRootComponent>
        </div>
    )
}

export default Homepage
