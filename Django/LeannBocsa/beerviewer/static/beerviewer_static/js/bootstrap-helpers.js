function addDynamicPanel(Parent, Child, Header, Body)
{ 


	 //Add our panel content
	var header = document.createElement("div");
	header.setAttribute("class", "panel-heading");
	header.appendChild(document.createTextNode(Header));
	Child.appendChild(header);

	var body = document.createElement("div");
	body.setAttribute("class", "panel-body");
	body.appendChild(document.createTextNode(Body));
	Child.appendChild(body);

	var parent = document.getElementById(Parent);
	parent.appendChild(Child);
}
