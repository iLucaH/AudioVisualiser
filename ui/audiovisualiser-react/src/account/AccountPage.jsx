import { clearUser } from '../tool/auth'

import { useNavigate } from 'react-router-dom'

import DropDownMenu from '../shared/DropDownMenu'
import AVRootComponent from '../shared/AVRootComponent'
import ContentInset from '../shared/ContentInset'

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
                    <h2>Welcome, User</h2>
                    <p style={{
                        marginBottom: '5px',
                    }}>Manage your account</p>
                    <ContentInset expanded={true}>
                        <button onClick={logoutUser}>Logout</button>
                    </ContentInset>
                </DropDownMenu>             
            </AVRootComponent>
        </div>
    )
}
export default AccountPage;