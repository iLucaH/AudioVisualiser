import * as Juce from "./juce/index.js";

console.log(window.__JUCE__.backend);

const nativeFunctionChangeSettingsHandle = Juce.getNativeFunction("nativeFunctionChangeSettings");
const nativeFunctionGetSettingsHandle = Juce.getNativeFunction("nativeFunctionGetSettings");

document.addEventListener("DOMContentLoaded", () => {
	
	nativeFunctionGetSettingsHandle(0).then((result) => {
		console.log("Getting setting SETTINGS_DIMENSION_W and received result:");
		console.log(result);
		if (result != -1) {
			document.getElementById("width").value = result;
		}
	});
	
	nativeFunctionGetSettingsHandle(1).then((result) => {
		console.log("Getting setting SETTINGS_DIMENSION_H and received result:");
		console.log(result);
		if (result != -1) {
			document.getElementById("height").value = result;
		}
	});
	
	nativeFunctionGetSettingsHandle(3).then((result) => {
		console.log("Getting setting SETTINGS_FFT_SIZE and received result:");
		console.log(result);
		if (result != -1) {
			document.getElementById("fft").value = result;
		}
	});
	
	var widthHeightButton = document.getElementById("nativeFunctionWidthHeightButton");
	widthHeightButton.addEventListener("click", () => {
		const formData = new FormData(document.getElementById("whForm"));
		const width = formData.get("width");
		const height = formData.get("height");
		
		if (width < 100 || width > 1920 || height < 100 || height > 1080 || width % 2 != 0 || height % 2 != 0) {
				widthHeightButton.style.backgroundColor = "#faa";
			return;
		}
		
		const SETTINGS_DIMENSION_WH = 2;
		nativeFunctionChangeSettingsHandle(SETTINGS_DIMENSION_WH, width, height).then((result) => {
			console.log(result);
			if (!result) {
				widthHeightButton.style.backgroundColor = "#faa";
				alert("There was an error changing this setting!");
			} else {
				widthHeightButton.style.backgroundColor = "#afa";
				setTimeout(() => {
					widthHeightButton.style.backgroundColor = "#fff";
				}, 2000);
			}
		});
	});
	
	var fftSelector = document.getElementById("fft");
	fftSelector.addEventListener("change", () => {
		const selectedValue = document.querySelector('select[name="fft"]').value;
		
		const SETTINGS_FFT_SIZE = 3;
		
		nativeFunctionChangeSettingsHandle(SETTINGS_FFT_SIZE, selectedValue).then((result) => {
			if (!result) {
				alert("There was an error changing this setting!");
			}
		});
	});
});