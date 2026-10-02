import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import Presets from './preset/Presets'

function Homepage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Presets" open={true}>
                    <Presets />
                </DropDownMenu>
                <DropDownMenu title="Effects" open={false}> 
                    <p>Child 1</p>
                    <p>Child 2</p>
                    <p>Child 3</p>
                    <p>Child 1</p>
                    <p>Child 2</p>
                    <p>Child 3</p>
                    <p>Child 1</p>
                    <p>Child 2</p>
                    <p>Child 3</p>
                    <p>Child 1</p>
                    <p>Child 2</p>
                    <p>Child 3</p>
                    <p>Child 1</p>
                    <p>Child 2</p>
                    <p>Milo</p>
                </DropDownMenu>                
            </AVRootComponent>
        </div>
    )
}

export default Homepage
