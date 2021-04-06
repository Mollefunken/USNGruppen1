function login() {
    const email = document.getElementById("email").value;
    const password = document.getElementById("password").value;
    // console.log(email, password)

    var XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
        "<login " +
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " +
        "xsi:noNamespaceSchemaLocation=\"http://81.167.182.254:8888/Schema.xsd\">" +
        "<email>" + email + "</email>" +
        "<password>" + password + "</password>" +
        "</login>";

    // console.log(XML);

    // var xhttp = new XMLHttpRequest(),
    //     method = "POST",
    //     url = "http://81.167.182.254:1337/api";
    // xhttp.onreadystatechange = function () {
    //     if (xhttp.readyState === XMLHttpRequest.DONE) {
    //         var status = xhttp.status;
    //         if (status === 0 || status >= 200 && status < 400) {
    //             console.log(xhttp.responseText);
    //             if (xhttp.responseText != "Passwords do not match.") {
    //                 const sessionID = "sessionID=" + this.responseXML.getElementsByTagName("sessionID")[0].childNodes[0].nodeValue;
    //                 console.log(sessionID);
    //                 document.cookie = sessionID + "; path /;";
    //                 window.location.href = 'http://81.167.182.254:8888/test.html'
    //             }
    //             else {
    //                 document.getElementById("responseText").value = "Incorrect username or password.";
    //             }
    //         }
    //         else {
    //             console.log("Error.")
    //         }
    //     }
    // }
    // xhttp.open(method, url);
    // xhttp.setRequestHeader('Content-type', 'text/xml');
    // xhttp.send(XML);

	fetch("http://81.167.182.254:1337/api", {
		method: "POST",
		body: XML,
		headers: {
			"Content-type": "application/xml"
		},
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			if (data == 'You are already logged in. If you want to log in with another user, log out first.\n' || data == 'Passwords do not match.\n') {
				document.getElementById("responseText").innerHTML = data;
			} else {
				const parser = new DOMParser();
				const xml = parser.parseFromString(data, "application/xml");
				const sessionID = "sessionID=" + xml.getElementsByTagName("sessionID")[0].childNodes[0].nodeValue;
				// console.log(sessionID);
				document.cookie = sessionID + "; path /;";
				window.location.href = 'http://81.167.182.254:8888/jsClient.html';
			}
		}).catch(console.error)
}

function logout() {
    // var xhttp = new XMLHttpRequest(),
    //     method = "GET",
    //     url = "http://81.167.182.254:1337/api/logout";
    // xhttp.onreadystatechange = function () {
    //     if (this.readyState == 4 && this.status == 200) {
    //         document.getElementById("res").innerHTML = this.responseText;
    //         document.cookie = "sessionID=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
    //         validate();
    //     }
    // };
    // xhttp.open(method, url);
    // xhttp.withCredentials = true;
    // xhttp.send();

	fetch("http://81.167.182.254:1337/api/logout", {
		method: "GET",
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			document.getElementById("res").innerHTML = data;
            document.cookie = "sessionID=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
			validate();
		}).catch(console.error)
}

function validate() {
    // var xhttp = new XMLHttpRequest(),
    //     method = "GET",
    //     url = "http://81.167.182.254:1337/api/validate";
    // xhttp.onreadystatechange = function () {
    //     if (xhttp.readyState === XMLHttpRequest.DONE) {
    //         var status = xhttp.status;
    //         if (status === 0 || status >= 200 && status < 400) {
    //             displayLoginStatus(this.responseXML.getElementsByTagName("status")[0].childNodes[0].nodeValue);
    //         }
    //     };
    // }
    // xhttp.open(method, url);
    // xhttp.withCredentials = true;
    // xhttp.send();

	fetch("http://81.167.182.254:1337/api/validate", {
		method: "GET",
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			const parser = new DOMParser();
			const xml = parser.parseFromString(data, "application/xml");
			// console.log(xml);
			displayLoginStatus(xml.getElementsByTagName("status")[0].childNodes[0].nodeValue);
		}).catch(console.error)
}

function displayLoginStatus(login_status) {
    const targetContainer = document.getElementById("login_status");
    if (login_status == 0) {
        targetContainer.innerHTML = "<img src='http://81.167.182.254:8888/css/images/check.svg' class='statusImage check'>You are logged in";
    }
    else {
        targetContainer.innerHTML = "<img src='http://81.167.182.254:8888/css/images/cross.svg' class='statusImage cross'>You are not logged in. Some functions will be unavailable.";
    }
}


function get(id) {
    // var xhttp = new XMLHttpRequest(),
    //     method = "GET",
    //     url = "http://81.167.182.254:1337/api/poem/" + id;
    // xhttp.onreadystatechange = function () {
    //     if (this.readyState == 4 && this.status == 200) {
    //         parsePoem(this);
    //     }
    // };
    // xhttp.open(method, url, true);
    // xhttp.send();

	fetch("http://81.167.182.254:1337/api/poem/" + id, {
		method: "GET"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			parsePoem(data);
		}).catch(console.error)
}

function parsePoem(xml) {
    var i;
    const parser = new DOMParser();
	const xmlDoc = parser.parseFromString(xml, "application/xml");
	console.log(xmlDoc);
	var table = "<table><tr><th>ID</th><th>Name</th></tr>";
    var x = xmlDoc.getElementsByTagName("poem");
    for (i = 0; i < x.length; i++) {
        table += "<tr><td>" +
            x[i].getElementsByTagName("id")[0].childNodes[0].nodeValue +
            "</td><td>" +
            x[i].getElementsByTagName("name")[0].childNodes[0].nodeValue +
            "</td></tr>";
    }
    table += "</table>";
    document.getElementById("res").innerHTML = table;
}

function add(name) {
    // console.log(name);
    var XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
        "<addPoem>" +
        "<name>" + name + "</name>" +
        "</addPoem>";
    // var xhttp = new XMLHttpRequest(),
    //     method = "POST",
    //     url = "http://81.167.182.254:1337/api/poem/";
    // xhttp.onreadystatechange = function () {
    //     if (this.readyState == 4 && this.status == 200) {
    //         document.getElementById("res").innerHTML = this.responseText;
    //     }
    // };
    // xhttp.open(method, url);
    // xhttp.setRequestHeader('Content-type', 'text/xml');
    // xhttp.withCredentials = true;
    // xhttp.send(XML);

	fetch("http://81.167.182.254:1337/api/poem/", {
		method: "POST",
		body: XML,
		headers: {
			"Content-type": "application/xml"
		},
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			document.getElementById("res").innerHTML = data;
		}).catch(console.error)
}

function update(id, name) {
    var XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
        "<updatePoem>" +
        "<id>" + id + "</id>" +
        "<name>" + name + "</name>" +
        "</updatePoem>";
    // var xhttp = new XMLHttpRequest(),
    //     method = "PUT",
    //     url = "http://81.167.182.254:1337/api/poem/" + id;
    // xhttp.onreadystatechange = function () {
    //     if (this.readyState == 4 && this.status == 200) {
    //         document.getElementById("res").innerHTML = this.responseText;
    //     }
    // };
    // xhttp.open(method, url);
    // xhttp.setRequestHeader('Content-type', 'text/xml');
    // xhttp.withCredentials = true;
    // xhttp.send(XML);

	fetch("http://81.167.182.254:1337/api/poem/" + id, {
		method: "PUT",
		body: XML,
		headers: {
			"Content-type": "application/xml"
		},
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			document.getElementById("res").innerHTML = data;
		}).catch(console.error)
}


function remove(id) {
    // var xhttp = new XMLHttpRequest(),
    //     method = "DELETE",
    //     url = "http://81.167.182.254:1337/api/poem/" + id;
    // xhttp.onreadystatechange = function () {
    //     if (this.readyState == 4 && this.status == 200) {
    //         document.getElementById("res").innerHTML = this.responseText;
    //     }
    // };
    // xhttp.open(method, url);
    // xhttp.withCredentials = true;
    // xhttp.send();

	fetch("http://81.167.182.254:1337/api/poem/" + id, {
		method: "DELETE",
		headers: {
			"Content-type": "application/xml"
		},
		credentials: "include"})
		.then((response) => response.text())
		.then(data => {
			// console.log(data);
			document.getElementById("res").innerHTML = data;
		}).catch(console.error)
}

function validateInputAdd(name) {
    if (name != "") {
        add(name);
    }
    else {
        alert("Poem name needs to be specified.");
    }
}

function validateInputUpdate() {
    const id = document.getElementById("updatePoemID").value;
    const name = document.getElementById("updatePoemName").value;

    if (name != "" && id != "") {
        update(id, name);
    }
    else {
        alert("All input fields must be specified.");
    }
}

function validateInputDelete(id) {
    if (id != "") {
        remove(id);
    }
    else {
        alert("Poem ID must be specified.");
    }
}