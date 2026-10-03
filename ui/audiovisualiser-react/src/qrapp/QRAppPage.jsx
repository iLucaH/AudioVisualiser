import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import QRCode from "react-qr-code";

import { messageJUCE, JuceFunctionHandlers } from '../services/juceHandlerService'

function QRAppPage() {
    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Open In App" open={true}>
                    <div style={{
                        display: "flex",
                        flexDirection: "column",
                        justifyContent: "center",
                        alignItems: "center",
                        height: "100%",
                        gap: "15px"
                    }}>
                        <QRCode value="https://example.com" size={256} />
                        <button onClick={() => messageJUCE(JuceFunctionHandlers.getOpenWebsite, "https://github.com/iLucaH/audiovisualiser-socket-client")}>Download the App!</button>
                    </div>
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default QRAppPage;
