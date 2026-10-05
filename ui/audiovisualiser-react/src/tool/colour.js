const toHexChannel = (v) => Math.round(Math.min(1, Math.max(0, v)) * 255).toString(16).padStart(2, '0')

export const toHex = ({ red, green, blue }) => `#${toHexChannel(red)}${toHexChannel(green)}${toHexChannel(blue)}`

export const fromHex = (hex) => ({
    red: parseInt(hex.slice(1, 3), 16) / 255,
    green: parseInt(hex.slice(3, 5), 16) / 255,
    blue: parseInt(hex.slice(5, 7), 16) / 255,
})