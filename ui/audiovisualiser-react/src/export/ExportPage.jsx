import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

function ExportPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Recorder" open={true}>
                    <button>Start</button>
                    <button>Stop</button>
                    
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default ExportPage;
