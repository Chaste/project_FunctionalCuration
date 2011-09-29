package fcs.shared;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.IsSerializable;

public class MenuData implements IsSerializable{

	private ArrayList<String> models;
	private ArrayList<String> protocols;
	public MenuData(){
		models = new ArrayList<String>();
		protocols= new ArrayList<String>();
	}
	public void addModel(String model){
		models.add(model);
	}
	public void addProtocol(String protocol){
		protocols.add(protocol);
	}
	public ArrayList<String> getModels(){
		return models;
	}
	public ArrayList<String> getProtocols(){
		return protocols;
	}
}
