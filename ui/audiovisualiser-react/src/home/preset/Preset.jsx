import NativeSelector from '../shared/OptionSelector'

function Preset({options, selectedValue, setSelectedValue}) {
    return (
        <div>
            <NativeSelector options={options} selectedValue={selectedValue} setSelectedValue={setSelectedValue} />
        </div>
    )
}

export default Preset