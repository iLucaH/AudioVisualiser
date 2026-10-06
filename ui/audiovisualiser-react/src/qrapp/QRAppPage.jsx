import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

import QRCode from "react-qr-code";
import ContentInset from '../shared/ContentInset'

import { useEffect, useState } from 'react'

import { messageJUCE, JuceFunctionHandlers } from '../services/juceHandlerService'

function QRAppPage() {

    const [socketHandle, setSocketHandle] = useState("")

    useEffect(() => {
        const getSocketHandle = async () => {
            const handle = await messageJUCE(JuceFunctionHandlers.getSocketHandle)

            setSocketHandle(handle)
        }

        getSocketHandle()
    }, [])

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
                        gap: "15px",
                    }}>
                        <ContentInset>
                            <QRCode value={socketHandle} size={256} />
                        </ContentInset>
                        <button onClick={() => messageJUCE(JuceFunctionHandlers.getOpenWebsite, "https://github.com/iLucaH/audiovisualiser-socket-client")}>Download the App!</button>
                    </div>
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default QRAppPage;
