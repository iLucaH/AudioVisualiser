import { setUser } from '../../tool/auth'

import { useNavigate } from 'react-router-dom'

import { useState } from 'react'

import DropDownMenu from '../../shared/DropDownMenu'
import AVRootComponent from '../../shared/AVRootComponent'

import { messageJUCE, JuceFunctionHandlers, waitForNativeEvent } from '../../services/juceHandlerService'

import styles from './LoginPage.module.css'

function LoginPage() {
    const navigate = useNavigate()

    // Login form state
    const [loginUsername, setLoginUsername] = useState("")
    const [loginPassword, setLoginPassword] = useState("")
    const [loginMessage, setLoginMessage] = useState("")

    // Register form state
    const [registerUsername, setRegisterUsername] = useState("")
    const [registerPassword, setRegisterPassword] = useState("")
    const [registerMessage, setRegisterMessage] = useState("")

    const [isLoading, setIsLoading] = useState(false)

    const loginUser = async () => {
        setLoginMessage("")
        setIsLoading(true)

        try {
            const eventPromise = waitForNativeEvent(JuceFunctionHandlers.loginPromiseEvent)

            const token = await messageJUCE(JuceFunctionHandlers.loginUser, loginUsername, loginPassword)
            console.log(token)
            if (token === 1) {
                setLoginMessage("Your details are incorrect! Please try again.")
            } else if (token === 2) {
                setLoginMessage("Invalid username.")
            } else if (token === 3) {
                setLoginMessage("Invalid password.")
            } else {
                setLoginMessage("Logging in...")
                const status = await eventPromise
                if (status[0]) {
                    setUser({ token: "Logged-In" }) // mark the account as logged in
                    navigate('/account')
                } else {
                    setLoginMessage("Your details are incorrect! Please try again.")
                }
            }
        } catch (err) {
            console.error(err)
            setLoginMessage("Your details are incorrect! Please try again.")
        } finally {
            setIsLoading(false)
        }
    }

    const registerUser = async () => {
        setRegisterMessage("")
        setIsLoading(true)

        try {
            const eventPromise = waitForNativeEvent(JuceFunctionHandlers.registerPromiseEvent)

            const response = await messageJUCE(JuceFunctionHandlers.registerNewUser, registerUsername, registerPassword)

            if (response === 1) {
                setRegisterMessage("Invalid username.")
            } else if (response === 2) {
                setRegisterMessage("Invalid password.")
            } else {
                setRegisterMessage("Registering your account...")
                const statusInt = await eventPromise
                if (statusInt == -1) { // REGISTER_API_ERROR
                    console.log("REGISTER_API_ERROR");
                    setRegisterMessage("There was an error registering your account...")
                } else if (statusInt == 0) { // REGISTER_SUCCESS
                    console.log("REGISTER_SUCCESS");
                    setRegisterMessage("Registration successful. You can now log in.")
                    setRegisterUsername("")
                    setRegisterPassword("")
                } else if (statusInt == 1) { // REGISTER_INVALID_USERNAME_NULL
                    console.log("REGISTER_INVALID_USERNAME_NULL");
                    setRegisterMessage("Please enter a valid username!")
                } else if (statusInt == 2) { // REGISTER_INVALID_USERNAME_MIN_CHARS
                    console.log("REGISTER_INVALID_USERNAME_MIN_CHARS");
                    setRegisterMessage("Please enter a username with at least 3 characters!")
                } else if (statusInt == 3) { // REGISTER_INVALID_USERNAME_MAX_CHARS
                    console.log("REGISTER_INVALID_USERNAME_MAX_CHARS");
                    setRegisterMessage("Please enter a username with less than 20 characters!")
                } else if (statusInt == 4) { // REGISTER_INVALID_PASSWORD_NULL
                    console.log("REGISTER_INVALID_PASSWORD_NULL");
                    setRegisterMessage("Please enter a valid password!")
                } else if (statusInt == 5) { // REGISTER_USERNAME_TAKEN
                    console.log("REGISTER_USERNAME_TAKEN");
                    setRegisterMessage("This username has already been taken!")
                } else if (statusInt == 6) { // REGISTER_PASSWORD_UNSAFE
                    console.log("REGISTER_PASSWORD_UNSAFE");
                    setRegisterMessage("Please make sure your password meets the followign criteria:<br> * One upper case and one lowercase letter.<br> * One digit and one special character.<br> * Between 8 and 18 characters in length.")
                } else {
                    console.log("Unknown native register event return value!");
                    setRegisterMessage("There was an error registering your account...")
                }
            }
        } catch (err) {
            console.error(err)
            setRegisterMessage("Could not reach the application. Please try again.")
        } finally {
            setIsLoading(false)
        }
    }

    return (
        <div className={styles.page}>
            <AVRootComponent>
                <DropDownMenu title="Login" open={true}>
                    <div className={styles.form}>
                        <p className={styles.heading}>Login</p>

                        <div className={styles.field}>
                            <label className={styles.label} htmlFor="login-username">Username</label>
                            <input
                                className={styles.input}
                                id="login-username"
                                type="text"
                                value={loginUsername}
                                onChange={(e) => setLoginUsername(e.target.value)}
                                autoComplete="username"
                            />
                        </div>

                        <div className={styles.field}>
                            <label className={styles.label} htmlFor="login-password">Password</label>
                            <input
                                className={styles.input}
                                id="login-password"
                                type="password"
                                value={loginPassword}
                                onChange={(e) => setLoginPassword(e.target.value)}
                                autoComplete="current-password"
                                onKeyDown={(e) => { if (e.key === 'Enter') loginUser() }}
                            />
                        </div>

                        <button className={styles.button} onClick={loginUser} disabled={isLoading}>
                            Login
                        </button>
                        {loginMessage && <p className={styles.message}>{loginMessage}</p>}
                    </div>
                </DropDownMenu>

                <DropDownMenu title="Register" open={false}>
                    <div className={styles.form}>
                        <p className={styles.heading}>Create an account</p>

                        <div className={styles.field}>
                            <label className={styles.label} htmlFor="register-username">Username</label>
                            <input
                                className={styles.input}
                                id="register-username"
                                type="text"
                                value={registerUsername}
                                onChange={(e) => setRegisterUsername(e.target.value)}
                                autoComplete="username"
                            />
                        </div>

                        <div className={styles.field}>
                            <label className={styles.label} htmlFor="register-password">Password</label>
                            <input
                                className={styles.input}
                                id="register-password"
                                type="password"
                                value={registerPassword}
                                onChange={(e) => setRegisterPassword(e.target.value)}
                                autoComplete="new-password"
                                onKeyDown={(e) => { if (e.key === 'Enter') registerUser() }}
                            />
                        </div>

                        <button className={styles.button} onClick={registerUser} disabled={isLoading}>
                            Click to register
                        </button>
                        {registerMessage && <p className={styles.message}>{registerMessage}</p>}
                    </div>
                </DropDownMenu>
            </AVRootComponent>
        </div>
    );
}

export default LoginPage;