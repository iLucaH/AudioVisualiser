import { clearUser } from '../tool/auth'

import { useNavigate } from 'react-router-dom'

import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'

function AccountPage() {
    const navigate = useNavigate()

    function logoutUser() {
        clearUser()
        navigate('/login')
    }

    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Account Settings" open={true}>
                    <p>Update Account Settings</p>
                    <button onClick={logoutUser}>Logout</button>
                </DropDownMenu>             
            </AVRootComponent>
        </div>
    )
}
export default AccountPage;