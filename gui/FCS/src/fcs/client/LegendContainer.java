package fcs.client;

import com.google.gwt.dom.client.Element;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.ui.Widget;

public class LegendContainer extends Widget{
	String id;
	public LegendContainer(String id){
		super();
		Element elem = DOM.createDiv();
		setElement(elem);
		elem.setId(id);
		
		this.id=id;
		this.setStyleName("legendCont");
		
	}
	public String getId(){
		return id;
	}
}
