import { BrowserRouter, Navigate, Route, Routes } from 'react-router-dom';

import './App.css'

import { useEffect, useState } from 'react'

import usePreventZoom from './tool/scrollManager'

import { getUser } from './tool/auth'
import { JuceFunctionHandlers, messageJUCE, waitForNativeEvent } from './services/juceHandlerService'

import Homepage from './home/HomePage'

import AudioPage from './audio/AudioPage'
import AccountPage from './account/AccountPage'
import LoginPage from './account/login/LoginPage'
import ExportPage from './export/ExportPage'
import SettingsPage from './settings/SettingsPage'
import QRAppPage from './qrapp/QRAppPage'

function App() {
  const [ appRefreshHandle, setAppRefreshHandle ] = useState(0)

  usePreventZoom();

  if (getUser()) {
    messageJUCE(JuceFunctionHandlers.setAuthTokenAlreadyExists, getUser().token)
  }

  useEffect(() => {
    function refreshComponent() {
      setAppRefreshHandle(key => key + 1)
    }

    let cancelled = false

    const listenForNewPresets = async () => {
        while (!cancelled) {
            await waitForNativeEvent(JuceFunctionHandlers.globalUpdateAll)
            if (!cancelled) {
                console.log("Refreshing all components. A global update all event has occurred!")
                refreshComponent()
            }
        }
    }
    listenForNewPresets()
    return () => { cancelled = true }
    
  }, [])

  return (
    <BrowserRouter onContextMenu={(e) => e.preventDefault()}>
      <Routes key={appRefreshHandle}>
        <Route path="/" element={<Homepage />} />
        <Route path="/qrapp" element={<QRAppPage />} />
        <Route path="/audio" element={<AudioPage />} />
        <Route path="/login" element={<LoginPage />} />
        <Route path="/account" element={<AccountPage />} />
        <Route path="/export" element={<ExportPage />} />
        <Route path="/settings" element={<SettingsPage />} />
      </Routes>
    </BrowserRouter>
  )
}

export default App
