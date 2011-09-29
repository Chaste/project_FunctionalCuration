package fcs.client.graph;


import java.util.ArrayList;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.Panel;
import com.google.gwt.user.client.ui.SimplePanel;
import com.google.gwt.user.client.ui.VerticalPanel;

import fcs.client.LegendContainer;
import fcs.shared.Plot;

public class GraphComposite extends Composite{
	public static int nbGraphs =0;
	Graph g;
	public GraphComposite(ArrayList<Plot> plots){
		
		LegendContainer legend = new LegendContainer("legend"+nbGraphs);
		LegendContainer hiddenLegend = new LegendContainer("hidden"+nbGraphs);
		LegendContainer overView = new LegendContainer("overview"+nbGraphs);
		nbGraphs++;

		Label title = new Label(plots.get(0).getName());
		title.setStyleName("h1");
		//Label xAxis = new Label("x axis: "+plot.getxTitle());
		//Label yAxis = new Label("y axis: "+plot.getyTitle());
		
		Button resetButton = new Button("reset");
		
		Panel one = new VerticalPanel();
		Panel two = new HorizontalPanel();
		Panel graphContainer = new SimplePanel();
		
		one.add(title);
		
		//two.add(yAxis);
		two.add(graphContainer);
		two.add(resetButton);
		two.add(legend);
		two.add(hiddenLegend);
		two.add(overView);
		one.add(two);
		//one.add(xAxis);
		
		g = new Graph(plots, null, legend, hiddenLegend, overView);
		graphContainer.add(g);
		resetButton.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
				resetGraph();
			}
		});
		// All composites must call initWidget() in their constructors.
	    initWidget(one);
	}
	
	public void resetGraph(){
		System.out.println("reset");
		g.reset();
	}
	

}
