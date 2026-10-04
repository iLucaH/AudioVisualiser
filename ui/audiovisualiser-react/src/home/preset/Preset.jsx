import {NativeSelector} from '../shared/OptionSelector'

function Preset({options, selectedValue, setSelectedValue, onChange = () => {} }) {
    return (
        <div>
            <NativeSelector options={options} selectedValue={selectedValue} setSelectedValue={setSelectedValue} onChange={onChange} />
        </div>
    )
}

export default Preset