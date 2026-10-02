export function getUser() {
    const user = localStorage.getItem('avuser')

    if (!user) {
        return null
    }

    return JSON.parse(user)
}

export function setUser({token}) {
    localStorage.setItem('avuser', JSON.stringify({token}))
}

export function clearUser() {
    localStorage.removeItem('avuser')
}