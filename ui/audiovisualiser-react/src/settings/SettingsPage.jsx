import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

function SettingsPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Rendering Settings" open={true}>
                    <p>Width</p>
                    <p>Height</p>
                    <p>Fullscreen</p>
                </DropDownMenu>
                <DropDownMenu title="Audio Settings" open={false}> 
                    <p>FFT Size</p>
                </DropDownMenu>
                <DropDownMenu title="System Settings" open={false}> 
                    <p>Socket Service Password</p>
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default SettingsPage;
