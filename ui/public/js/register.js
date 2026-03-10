import * as Juce from "./juce/index.js";

window.__JUCE__.backend.addEventListener("onRegisterEvent", (statusInt) => {
	var registerErrorMsg = document.getElementById("loginMessage");
	var registerButton = document.getElementById("nativeFunctionRegisterButton");
	console.log("Register status: " + statusInt);
	
	registerErrorMsg.style.opacity = 1;
	registerButton.style.backgroundColor = "#faa";
	
	if (statusInt == -1) { // REGISTER_API_ERROR
		console.log("REGISTER_API_ERROR");
		registerErrorMsg.textContent = "There was an error accessing the register service!";
	} else if (statusInt == 0) { // REGISTER_SUCCESS
		console.log("REGISTER_SUCCESS");
		
		registerErrorMsg.style.opacity = 0;
		registerButton.style.backgroundColor = "#fff";
		alert("You have successfully registered an account. Return to login!");
	} else if (statusInt == 1) { // REGISTER_INVALID_USERNAME_NULL
		console.log("REGISTER_INVALID_USERNAME_NULL");
		registerErrorMsg.textContent = "Please enter a valid username!";
	} else if (statusInt == 2) { // REGISTER_INVALID_USERNAME_MIN_CHARS
		console.log("REGISTER_INVALID_USERNAME_MIN_CHARS");
		registerErrorMsg.textContent = "Please enter a username with at least 3 characters!";
	} else if (statusInt == 3) { // REGISTER_INVALID_USERNAME_MAX_CHARS
		console.log("REGISTER_INVALID_USERNAME_MAX_CHARS");
		registerErrorMsg.textContent = "Please enter a username with less than 20 characters!!";
	} else if (statusInt == 4) { // REGISTER_INVALID_PASSWORD_NULL
		console.log("REGISTER_INVALID_PASSWORD_NULL");
		registerErrorMsg.textContent = "Please enter a valid password!";
	} else if (statusInt == 5) { // REGISTER_USERNAME_TAKEN
		console.log("REGISTER_USERNAME_TAKEN");
		registerErrorMsg.textContent = "This username has already been taken!";
	} else if (statusInt == 6) { // REGISTER_PASSWORD_UNSAFE
		console.log("REGISTER_PASSWORD_UNSAFE");
		registerErrorMsg.innerHTML = "Please make sure your password meets the followign criteria:<br> * One upper case and one lowercase letter.<br> * One digit and one special character.<br> * Between 8 and 18 characters in length.";
	} else {
		console.log("Unknown native register event return value!");
		registerErrorMsg.textContent = "There was an error communciating with the register service!";
	}
});

const nativeFunctionRegisterHandle = Juce.getNativeFunction("nativeFunctionRegister");

document.addEventListener("DOMContentLoaded", () => {
	var registerErrorMsg = document.getElementById("loginMessage");
	var registerButton = document.getElementById("nativeFunctionRegisterButton");
	registerButton.addEventListener("click", () => {
		const formData = new FormData(document.getElementById("loginForm"));
		var username = formData.get("username");
		var password = formData.get("password");
		var password_confirmed = formData.get("confirm password");

		if (password == password_confirmed) {
			nativeFunctionRegisterHandle(username, password).then((result) => {
				if (result == 20) {
					registerErrorMsg.style.opacity = 0;
					console.log("REGISTER_ATTEMPT_ARGS_OK");
					registerButton.style.backgroundColor = "#ddd";
				} else {
					registerErrorMsg.style.opacity = 1;
					registerButton.style.backgroundColor = "#faa";
					if (result == 21) {
						console.log("REGISTER_ATTEMPT_NOT_ENOUGH_ARGS");
						registerErrorMsg.textContent = "Please enter a username and password!";
					} else if (result == 22) {
						console.log("REGISTER_ATTEMPT_INVALID_USERNAME");
						registerErrorMsg.textContent = "Please enter a valid username!";
					} else if (result == 23) {
						console.log("REGISTER_ATTEMPT_INVALID_PASSWORD");
						registerErrorMsg.textContent = "Please enter a valid password!";
					} else {
						console.log("Unknown native register function return value!");
					}
				}
			});
		} else {
			registerErrorMsg.textContent = "Passwords do not match!";
			console.log("Passwords do not match!");
		}
	});
	var returnButton = document.getElementById("returnButton");
	returnButton.addEventListener("click", () => {
		history.back();
	});
});