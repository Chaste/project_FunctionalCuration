package fcs.shared;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.IsSerializable;

public class Serie implements IsSerializable{

	private String name;
	private ArrayList<Point> points;

	public Serie(){
		this.points = new ArrayList<Point>();
	}
	public Serie(String name) {
		this.name = name;
		this.points = new ArrayList<Point>();
	}

	public void addPoint(double x, double y) {
		points.add(new Point(x,y));
	}
	public void addPoint(Point p){
		points.add(p);
	}
	@Override
	public String toString(){
		return name+": "+points.toString();
	}
	public String getName(){
		return name;
	}
	public ArrayList<Point> getPoints(){
		return points;
	}
}
