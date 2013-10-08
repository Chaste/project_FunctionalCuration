
var pages = [ "model", "protocol", "experiment" ];


function initDb ()
{
	for (var i = 0; i < pages.length; i++)
	{
		var btn = document.getElementById(pages[i] + "chooser");
		registerSwitchPagesListener (btn, pages[i]);
	}
	switchPage ("model");
}
function switchPage (page)
{
	console.log ("switching to " + page);
	for (var i = 0; i < pages.length; i++)
	{
		if (pages[i] == page)
			document.getElementById(pages[i] + "db").style.display = "block";
		else
			document.getElementById(pages[i] + "db").style.display = "none";
	}
}
function registerSwitchPagesListener (btn, page)
{
	console.log ("register switch listener: " + page);
	btn.addEventListener("click", function () {
		console.log ("switch listener triggered " + page);
		switchPage (page);
	}, true);
}

document.addEventListener("DOMContentLoaded", initDb, false);