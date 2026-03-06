import * as Juce from "./juce/index.js";

console.log(window.__JUCE__.backend);

window.__JUCE__.backend.addEventListener("onLoginEvent", (successBool) => {
		console.log("Login success: " + successBool);
	}
);

const nativeFunctionLoginHandle = Juce.getNativeFunction("nativeFunctionLogin");

document.addEventListener("DOMContentLoaded", () => {
	var loginButton = document.getElementById("nativeFunctionLoginButton");
	loginButton.addEventListener("click", () => {
		console.log("Login button clicked");
		const formData = new FormData(document.getElementById("loginForm"));
		nativeFunctionLoginHandle(formData.get("username"), formData.get("password")).then((result) => {
			if (result == 10) {
				console.log("LOGIN_ATTEMPT_ARGS_OK");
				loginButton.style.backgroundColor = "#cfc";
			}
			if ([11,12,13].includes(result)) {
				loginButton.style.backgroundColor = "#faa";
			}
		});
	});
});