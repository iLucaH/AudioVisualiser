import styles from './Shared.module.css'

import { useNavigate, useLocation } from 'react-router-dom'
import { getUser } from '../tool/auth'

import IconButton from '../shared/IconButton'
import { FiMusic, FiSettings, FiDownload, FiLogIn, FiHome, FiSmartphone } from 'react-icons/fi'

function QuickMenu() {
    const navigate = useNavigate();
    const location = useLocation();

    return (
        <div>
            <div className={styles.quickmenu}>
                <IconButton onClick={() =>  navigate('/')} text="Home" logo={FiHome} highlighted={location.pathname === '/'} />
                <IconButton onClick={() =>  navigate('/audio')} text="Audio" logo={FiMusic} highlighted={location.pathname === '/audio'} />
                <IconButton onClick={() =>  navigate('/settings')} text="Settings" logo={FiSettings} highlighted={location.pathname === '/settings'} />
                <IconButton onClick={() =>  navigate('/export')} text="Export" logo={FiDownload} highlighted={location.pathname === '/export'} />
                <IconButton onClick={() =>  navigate(getUser()?.token ? '/account' : '/login')} text={getUser()?.token ? 'Account' : 'Login'} logo={FiLogIn} highlighted={location.pathname === '/account' || location.pathname === '/login'} />
                <IconButton onClick={() =>  navigate('/qrapp')} text="Open In App" logo={FiSmartphone} highlighted={location.pathname === '/qrapp'}/>
            </div>
        </div>
    )
}

export default QuickMenu;