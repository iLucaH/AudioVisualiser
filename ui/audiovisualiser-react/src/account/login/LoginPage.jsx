import { setUser } from '../../tool/auth'

import { useNavigate } from 'react-router-dom'

import DropDownMenu from '../../shared/DropDownMenu'
import AVRootComponent from '../../shared/AVRootComponent'

function LoginPage() {
    const navigate = useNavigate()

    function loginUser() {
        // Simulate login logic
        setUser({ token: 'dummyToken' })
        navigate('/account')
    }

    return (
        <div>
            <AVRootComponent>
                <DropDownMenu title="Login" open={true}>
                    <p>Login Form</p>
                    <button onClick={loginUser}>Login</button>
                </DropDownMenu>
                <DropDownMenu title="Register" open={false}> 
                    <p>Register Form</p>
                </DropDownMenu>                
            </AVRootComponent>
        </div>
    );
}

export default LoginPage;