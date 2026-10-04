import { createContext, useContext, useState } from 'react'

const AppContext = createContext()

export function AppProvider({ children }) {
    const [selectedValue, setSelectedValue] = useState({
            value: 'Loading...',
            key: '0',
            subcontent: []
    })
    const [dropdownSelectorOpen, setDropdownSelectorOpen] = useState([])

    return (
        <AppContext.Provider value={{ selectedValue, setSelectedValue, dropdownSelectorOpen, setDropdownSelectorOpen }}>
            {children}
        </AppContext.Provider>
    )
}

export function useApp() {
    return useContext(AppContext)
}

export default AppProvider