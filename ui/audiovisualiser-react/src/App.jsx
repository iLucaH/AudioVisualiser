import { BrowserRouter, Navigate, Route, Routes } from 'react-router-dom';

import './App.css'

import usePreventZoom from './tool/scrollManager'

import Homepage from './home/HomePage'

import AudioPage from './audio/AudioPage'
import AccountPage from './account/AccountPage'
import LoginPage from './account/login/LoginPage'
import ExportPage from './export/ExportPage'
import SettingsPage from './settings/SettingsPage'
import QRAppPage from './qrapp/QRAppPage'

function App() {
  usePreventZoom();
  return (
    <BrowserRouter onContextMenu={(e) => e.preventDefault()}>
      <Routes>
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
