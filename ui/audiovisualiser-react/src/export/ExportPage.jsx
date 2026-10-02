import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

function ExportPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Recorder" open={true}>
                    <p>Start</p>
                    <p>Stop</p>
                    <p>FilePath</p>
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default ExportPage;
