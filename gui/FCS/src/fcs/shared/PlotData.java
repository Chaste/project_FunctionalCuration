package fcs.shared;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.IsSerializable;


public class PlotData implements IsSerializable{

	
	private ArrayList<Plot> plots;
	
	public PlotData() {
		plots= new ArrayList<Plot>();
	}

	public void addPlot(Plot p) {
		plots.add(p);
	}
	public ArrayList<Plot> getPlots(){
		return plots;
	}
}
