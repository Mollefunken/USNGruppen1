//window.onload=alert('Velkommen til Javascript-klienten!')

var HTTP_COOKIE = document.cookie;

function validate() {
	cookie();

	let sesjonsID = document.cookie;
	let arr = sesjonsID.split("=");
	let part = arr[1];
	console.log(sesjonsID);
	console.log(part);

	fetch("http://158.248.21.214:9090/Dikt/sesjon/" + part, {
                method: "GET"
        })
                .then ((response) => response.text())
                .then (data => {
					console.log(data);
					let parser = new DOMParser(),
					xmlDoc = parser.parseFromString(data, "text/xml");

					doku = xmlDoc.getElementsByTagName("epostadresse")[0];
					if(doku)
					{
						email = doku.childNodes[0];
						email = email.nodeValue;

						console.log(email);
						document.getElementById("email-valid").innerHTML = email;

						document.getElementById("loggedInAs").innerHTML = "Logget inn som: ";
        					document.getElementById("email-valid").style.display = "inline";


					} else {

					}

				})
	
}

function testParagraph () {
	document.getElementById("testVerdi").innerHTML = document.cookie;

}

function cookie () {
	let lengde = HTTP_COOKIE.length;
	if (lengde == 0) {
		randVal = Math.floor(Math.random()*99999999);
		document.cookie = "jscookie=" + randVal;
	}
}

async function postToApi (postObject) {

	fullUrl = "http://158.248.21.214:9090" + postObject.url;

        data = "<?xml version=\"1.0\" encoding =\"utf-8\"?>"                            +
                "<login "                                                               +
                "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "              +
                "xsi:noNamespaceSchemaLocation=\"http://158.248.21.214/schema.xsd\">"   +
                "</login>";

        console.log("Attempted request-method: " + postObject.method)

        let parser = new DOMParser(),
        xmlDoc = parser.parseFromString(data, "text/xml");

        tableEle = xmlDoc.createElement(postObject.table);

        var i;
        for(i = 0; i < postObject.elements.length; i++) {
                element = xmlDoc.createElement(postObject.elements[i]);
                value = xmlDoc.createTextNode(postObject.values[i]);
                element.appendChild(value);
                tableEle.appendChild(element);
        }

        xmlDoc.getElementsByTagName("login")[0].appendChild(tableEle);

        console.log(xmlDoc);

        fetch(fullUrl, {
                method: postObject.method,
                headers: {"Cookie": document.cookie},
                credentials: "include",
                body: new XMLSerializer().serializeToString(xmlDoc)
                })
                .then (x => x.text())
                .then (y => document.getElementById("testVerdi").innerHTML = y);


}

async function deleteFromApi () {
	console.log("Skjer her");
}

//Kan kalles på med eks. <button onclick = 'hentfil()'>Placeholder</button>
function hentFil(){
                fetch('fetch-test.txt') 
                        .then( function (respons) { return respons.text() }           )
                        .then( function (kropp)   { document.querySelector('#beholder')
                        .innerHTML+=kropp } )
}

function getPoem() {
	var diktID = document.getElementById("getPoem").value;


	fetch("http://158.248.21.214:9090/Dikt/dikt/" + diktID, {
                method: "GET"
        })
                .then ((response) => response.text())
		.then (data => document.getElementById("ResultBox").innerHTML = data);
}

function getAllPoems() {
        fetch("http://158.248.21.214:9090/Dikt/dikt/", {
                method: "GET"
        })
                .then ((response) => response.text())
                .then (data => document.getElementById("ResultBox").innerHTML = data);
}

function addPoem() {

	let postObject = {
		method:"POST",
		url:"/Dikt/Dikt/",
		table: "Dikt",
		elements: ["dikt"],
		values: [ document.getElementById("leggTilDikt").value ]
	};

	postToApi(postObject);

}

function updatePoem() {
	let diktID = document.getElementById("updatePoemId").value;
        let dikt = document.getElementById("updatePoemText").value;



        let postObject = {
                method:"PUT",
                url:"/Dikt/Dikt/" + diktID,
                table: "Dikt",
                elements: ["dikt"],
                values: [ document.getElementById("updatePoemText").value ]
        };

        postToApi(postObject);



}

function deletePoem() {
	var diktID = document.getElementById("deletePoem").value;

	let url = "http://158.248.21.214:9090/Dikt/Dikt/" + diktID;
	console.log(url);

	fetch("http://158.248.21.214:9090/Dikt/Dikt/" + diktID, {
                method: "DELETE",
                credentials: "include",
                headers: {'Cookie': document.cookie}
        })
                .then (x => x.text())
                .then (y => console.log);
	console.log("Kommer hit");
}

function deleteAllPoems() {

        fetch("http://158.248.21.214:9090/Dikt/Dikt/", {
                method: "DELETE",
                credentials: "include",
                headers: {'Cookie': document.cookie}
        })
                .then (x => x.text())
                .then (y => console.log);
        console.log("Kommer hit All");
}


function login() {

	let postObject = {
		method:"POST",
		url:"/Dikt/sesjon/",
		table: "bruker",
		elements: ["epostadresse", "passordhash"],
		values: [
			document.getElementById("loginEmail").value,
			document.getElementById("loginPassword").value
		]
	};

	postToApi(postObject);


}

function logout() {
	cookie();


	fetch("http://158.248.21.214:9090/Dikt/sesjon/", {
		method: "DELETE",
		credentials: "include",
		headers: {'Cookie': document.cookie}
	})
		.then (x => x.text())
		.then (y => console.log);

}

function openForm() {
  document.getElementById("myForm").style.display = "block";
}

function closeForm() {
  document.getElementById("myForm").style.display = "none";
}
