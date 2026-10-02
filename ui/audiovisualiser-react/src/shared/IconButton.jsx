function IconButton({ onClick, text, highlighted = false, logo: Logo }) {
    return (
        <div
            style={{
                cursor: 'pointer',
                display: 'flex',
                alignItems: 'center',
                gap: '6px',
                backgroundColor: highlighted ? 'lightgray' : 'transparent'
            }}
            role="button"
            tabIndex="0"
            onClick={onClick}
        >
            <Logo />
            {text}
        </div>
    )
}

export default IconButton