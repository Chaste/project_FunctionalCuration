var versions = new Array ();
var files = new Array ();
var doc;
var basicurl;

function parseUrl (href)
{
	var t = href.split ("/");
	for (var i = 0; i < t.length; i++)
		if ("/" + t[i] == contextPath && i + 4 < t.length && t[i+1] == "protocol")
		{
			basicurl = t.slice (0, i + 4).join ("/") + "/";
			return t.slice (i + 4);
		}
	return null;
}

function getCurVersionId (url)
{
	if (url.length < 2)
		return null;
	return url[1];
}

function getCurFileId (url)
{
	if (url.length < 4)
		return null;
	return url[3];
}

function displayVersion (id)
{
	var v = versions[id];
	if (!v)
	{
		addNotification ("no such version", "error");
		return;
	}
    var dv = doc.version;
	
	dv.name.innerHTML = v.name;
	dv.author.innerHTML = v.author;
	dv.time.setAttribute ("datetime", v.created);
	dv.time.innerHTML = beautifyTimeStamp (v.created);
	
	removeChildren (dv.filestable);

	var tr = document.createElement("tr");
	var td = document.createElement("th");
	td.appendChild(document.createTextNode ("Name"));
	tr.appendChild(td);
	td = document.createElement("th");
	td.appendChild(document.createTextNode ("Type"));
	tr.appendChild(td);
	td = document.createElement("th");
	td.appendChild(document.createTextNode ("Size"));
	tr.appendChild(td);
	td = document.createElement("th");
	td.appendChild(document.createTextNode ("Actions"));
	tr.appendChild(td);
	dv.filestable.appendChild(tr);
	
	
	for (var i = 0; i < v.files.length; i++)
	{
		var file = files[v.files[i]];
		tr = document.createElement("tr");
		td = document.createElement("td");
		td.appendChild(document.createTextNode (file.name));
		tr.appendChild(td);
		td = document.createElement("td");
		td.appendChild(document.createTextNode (file.type));
		tr.appendChild(td);
		td = document.createElement("td");
		td.appendChild(document.createTextNode (humanReadableBytes (file.size)));
		tr.appendChild(td);
		td = document.createElement("td");
		var a = document.createElement("a");
		a.href = basicurl + convertForURL (v.name) + "/" + v.id + "/" + convertForURL (file.name) + "/" + file.id + "/";
		a.appendChild(document.createTextNode ("view"));
		registerFileDisplayer (a);//, basicurl + convertForURL (v.name) + "/" + v.id + "/");
		td.appendChild(a);
		td.appendChild(document.createTextNode (" "));
		var a = document.createElement("a");
		a.href = contextPath + "/download/p/" + convertForURL (v.name) + "/" + v.id + "/" + file.id + "/" + convertForURL (file.name);
		a.appendChild(document.createTextNode ("download"));
		td.appendChild(a);
		tr.appendChild(td);
		dv.filestable.appendChild(tr);
	}
	
	doc.protocol.details.style.display = "none";
	doc.protocol.version.style.display = "block";

	doc.version.details.style.display = "block";
	doc.version.filedetails.style.display = "none";
	// update address bar
	
}

function registerFileDisplayer (elem)
{
	elem.addEventListener("click", function (ev) {
		if (ev.which == 1)
		{
			ev.preventDefault();
			//doc.file.close.href = closeurl;
			nextPage (elem.href);
			//displayFile (fileid);
		}
    	}, true);
}

function registerVersionDisplayer (elem)
{
	elem.addEventListener("click", function (ev) {
		if (ev.which == 1)
		{
			ev.preventDefault();
			//doc.file.close.href = closeurl;
			nextPage (elem.href);
			//displayFile (fileid);
		}
    	}, true);
}

function updateVersion (rv)
{
	var v = versions[rv.id];
	if (!v)
	{
		v = new Array ();
		versions[rv.id] = v;
	}
	
	v.name = rv.version;
	v.author = rv.author;
	v.created = rv.created;
	v.id = rv.id;
	v.files = new Array ();
	for (var i = 0; i < rv.files.length; i++)
	{
		updateFile (rv.files[i]);
		v.files.push (rv.files[i].id);
	}
}

function updateFile (rf)
{
	var f = files[rf.id];
	if (!f)
	{
		f = new Array ();
		files[rf.id] = f;
	}
	
	f.id = rf.id;
	f.created = rf.created;
	f.type = rf.filetype;
	f.author = rf.author;
	f.name = rf.name;
	f.size = rf.size;
}

function displayFile (id)
{
	var f = files[id];
	if (!f)
	{
		console.log (id);
		console.log (files);
		addNotification ("no such file", "error");
		return;
	}
    var df = doc.file;
	df.name.innerHTML = f.name;
	df.time.setAttribute ("datetime", f.created);
	df.time.innerHTML = beautifyTimeStamp (f.created);
	df.author.innerHTML = f.author;
	doc.version.details.style.display = "none";
	doc.version.filedetails.style.display = "block";
}

function requestInformation (jsonObject, onSuccess)
{
	// TODO: loading indicator.. so the user knows that we are doing something
    
	var xmlhttp = null;
    // !IE
    if (window.XMLHttpRequest)
    {
        xmlhttp = new XMLHttpRequest();
    }
    // IE -- microsoft, we really hate you. every single day.
    else if (window.ActiveXObject)
    {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    
    xmlhttp.open("POST", '', true);
    xmlhttp.setRequestHeader("Content-type", "application/json");

    xmlhttp.onreadystatechange = function()
    {
        if(xmlhttp.readyState != 4)
        	return;
        
    	var json = JSON.parse(xmlhttp.responseText);
    	console.log (json);
    	displayNotifications (json);
    	
        if(xmlhttp.status == 200)
        {
        	
        	if (json.protocolversion)
        	{
        		var rv = json.protocolversion;
        		
        		updateVersion (rv);
        		onSuccess ();
        	}
        	/*if (json.modelversionfile)
        	{
        		onSuccess ();
        	}*/
        }
    };
    xmlhttp.send(JSON.stringify(jsonObject));
}

function nextPage (url)
{
	//if (//url)
	window.history.pushState(document.location.href, "", url);
	
	render ();
}

function render ()
{
	var url = parseUrl (document.location.href);
	console.log (basicurl);
	console.log (url);
	var curVersionId = getCurVersionId (url);
	
	if (curVersionId)
	{
		var v = versions[curVersionId];
		if (!v)
		{
			// request info about version only
			requestInformation ({
		    	task: "getProtocolInfo",
		    	protocolversion: curVersionId
			}, render);
			return;
		}
		else
			displayVersion (curVersionId);
		
		
		var curFileId = getCurFileId (url);
		if (curFileId)
		{
			displayFile (curFileId);
			doc.file.close.href = basicurl + convertForURL (v.name) + "/" + v.id + "/";
		}
	}
	else
	{
		doc.protocol.version.style.display = "none";
		doc.protocol.details.style.display = "block";
	}
}

function initModel ()
{
	doc = {
			protocol : {
				details : document.getElementById("protocoldetails"),
				version : document.getElementById("protocolversion")
			},
			version : {
				close : document.getElementById("protocolversionclose"),
				name : document.getElementById("protocolversionname"),
				time : document.getElementById("protocolversiontime"),
				author : document.getElementById("protocolversionauthor"),
				details : document.getElementById("protocolversiondetails"),
				filestable : document.getElementById("protocolversionfilestable"),
				filedetails : document.getElementById("protocolversionfiledetails")
			},
			file: {
				close : document.getElementById("protocolversionfileclose"),
				name : document.getElementById("protocolversionfilename"),
				time : document.getElementById("protocolversionfiletime"),
				author : document.getElementById("protocolversionfileauthor")
			}
	};
	
	render ();
	window.onpopstate = render;
	
	doc.version.close.href = basicurl;
	doc.version.close.addEventListener("click", function (ev) {
		if (ev.which == 1)
		{
			ev.preventDefault();
			doc.protocol.version.style.display = "none";
			doc.protocol.details.style.display = "block";
			nextPage (doc.version.close.href);
		}
    }, true);
		
	doc.file.close.addEventListener("click", function (ev) {
		if (ev.which == 1)
		{
			ev.preventDefault();
			doc.version.filedetails.style.display = "none";
			doc.version.details.style.display = "block";
			nextPage (doc.file.close.href);
		}
    }, true);
	
	
	// search for special links
	var elems = document.getElementsByTagName('a');
    for (var i = 0; i < elems.length; i++)
    {
    	var classes = ' ' + elems[i].className + ' ';
        if(classes.indexOf(' protocolversionlink ') > -1)
        {
        	// links to see the model details
        	//var link = elems[i].href;
        	registerVersionDisplayer (elems[i]);
        	/*elems[i].addEventListener("click", function (ev) {
        		if (ev.which == 1)
        		{
        			ev.preventDefault();
        			// set new url
        			// call action
        			//nextPage (elems[i].href);
        		}
        	}, true);
        	*/
            //elems[i].href = "";
            //console.log ("test");
        }

        if(classes.indexOf(' protocolversionfilelink ') > -1)
        {
        	// links to see the file details
        }
        
    }
}



document.addEventListener("DOMContentLoaded", initModel, false);