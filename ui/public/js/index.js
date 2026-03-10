import * as Juce from "./juce/index.js";

console.log(window.__JUCE__.backend);

window.__JUCE__.backend.addEventListener("onLoginEvent", (successBool) => {
		console.log("Login success: " + successBool);
		document.getElementById("nativeFunctionLoginButton").style.backgroundColor = "#fff";
		if (successBool) {
			alert("You are successfully logged in!");
		} else {
			document.getElementById("loginMessage").textContent = "Incorrect username or password!";
			document.getElementById("loginMessage").style.opacity = 1;
		}
	}
);

const nativeFunctionLoginHandle = Juce.getNativeFunction("nativeFunctionLogin");

document.addEventListener("DOMContentLoaded", () => {
	var loginErrorMsg = document.getElementById("loginMessage");
	
	var loginButton = document.getElementById("nativeFunctionLoginButton");
	loginButton.addEventListener("click", () => {
		console.log("Login button clicked");
		const formData = new FormData(document.getElementById("loginForm"));
		nativeFunctionLoginHandle(formData.get("username"), formData.get("password")).then((result) => {
			if (result == 10) {
				loginErrorMsg.style.opacity = 0;
				console.log("LOGIN_ATTEMPT_ARGS_OK");
				loginButton.style.backgroundColor = "#ddd";
			} else {
				loginErrorMsg.style.opacity = 1;
				loginButton.style.backgroundColor = "#faa";
				if (result == 11) {
					console.log("LOGIN_ATTEMPT_NOT_ENOUGH_ARGS");
					loginErrorMsg.textContent = "Please enter a username and password!";
				} else if (result == 12) {
					console.log("LOGIN_ATTEMPT_INVALID_USERNAME");
					loginErrorMsg.textContent = "Please enter a valid username!";
				} else if (result == 13) {
					console.log("LOGIN_ATTEMPT_INVALID_PASSWORD");
					loginErrorMsg.textContent = "Please enter a valid password!";
				} else {
					console.log("Unknown native login return value!");
				}
			}
		});
	});
	var registerButton = document.getElementById("registerButton");
	registerButton.addEventListener("click", () => {
		console.log("Register button clicked");
		window.location.href = "register.html";
	});
});