import * as Juce from "./juce/index.js";

window.__JUCE__.backend.addEventListener("onRegisterEvent", (statusInt) => {
		console.log("Register status: " + statusInt);
	}
);

const nativeFunctionRegisterHandle = Juce.getNativeFunction("nativeFunctionRegister");

document.addEventListener("DOMContentLoaded", () => {
	var register = document.getElementById("nativeFunctionRegisterButton");
	register.addEventListener("click", () => {
		const formData = new FormData(document.getElementById("registerForm"));
		var username = formData.get("username");
		var password = formData.get("password");
		var password_confirmed = formData.get("confirm password");
		if (password == password_confirmed) {
			nativeFunctionRegisterHandle(username, password).then((result) => {
				if (result == 0) {
						
				}
			});
		} else {
			console.log("Passwords do not match!");
		}
	});
});