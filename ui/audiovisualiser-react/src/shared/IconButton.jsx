import styles from './Shared.module.css'

function IconButton({ onClick, text, highlighted = false, logo: Logo }) {
    return (
        <div 
            className={highlighted ? styles.highlighted : styles.unhighlited}
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