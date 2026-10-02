import styles from './AVRootComponent.module.css'

import QuickMenu from '../shared/QuickMenu'

function AVRootComponent({children}) {
    return (
        <div>
            <div className={styles.content}>
                <div className={styles.cluster}>
                    {children}
                </div>
                <QuickMenu />
            </div>
        </div>
    )
}

export default AVRootComponent
