import { createContext, useContext, useState } from 'react'

const AppContext = createContext()

export function AppProvider({ children }) {
    const [selectedValue, setSelectedValue] = useState({
            value: 'Loading...',
            key: '0',
            subcontent: []
    })
    const [selectedEffect, setSelectedEffect] = useState({
            name: 'Loading...',
            id: -1,
    })
    const [dropdownSelectorOpen, setDropdownSelectorOpen] = useState([])
    const [inputValues, setInputValues] = useState({})
    const [presetInformationMessage, setPresetInformationMessage] = useState({})

    return (
        <AppContext.Provider value={{ selectedValue, setSelectedValue, dropdownSelectorOpen, setDropdownSelectorOpen, selectedEffect, setSelectedEffect, inputValues, setInputValues, presetInformationMessage, setPresetInformationMessage }}>
            {children}
        </AppContext.Provider>
    )
}

export function useApp() {
    return useContext(AppContext)
}

export default AppProvider