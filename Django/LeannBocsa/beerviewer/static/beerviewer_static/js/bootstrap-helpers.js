function addDynamicPanel(Parent, Header, Body)
{ 


	 //Add our panel content
	var header = document.createElement("div");
	header.setAttribute("class", "panel-heading");
	header.appendChild(document.createTextNode(Header));
	panel.appendChild(header);

	var body = document.createElement("div");
	body.setAttribute("class", "panel-body");
	body.appendChild(document.createTextNode(Body));
	panel.appendChild(body);

	var parent = document.getElementById(Parent);
	parent.appendChild(panel);
}
