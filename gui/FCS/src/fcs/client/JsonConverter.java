package fcs.client;

import java.util.ArrayList;

import com.google.gwt.json.client.JSONArray;
import com.google.gwt.json.client.JSONNumber;
import com.google.gwt.json.client.JSONObject;
import com.google.gwt.json.client.JSONString;
import com.google.gwt.json.client.JSONValue;

import fcs.shared.Plot;
import fcs.shared.Serie;

public class JsonConverter {
	public static JSONArray convert(Plot plot) {
		ArrayList<Serie> list = plot.getSeries();
		JSONArray series = new JSONArray();
		for (int i = 0; i < list.size(); i++) {
			Serie s = list.get(i);
			JSONObject obj = new JSONObject();
			obj.put("label", new JSONString(s.getName()));
			JSONArray points = new JSONArray();
			for (int j = 0; j < s.getPoints().size(); j++) {
				JSONArray p = new JSONArray();
				p.set(0, new JSONNumber(s.getPoints().get(j).getX()));
				p.set(1, new JSONNumber(s.getPoints().get(j).getY()));
				points.set(j, p);
			}
			obj.put("data", points);
			series.set(i, obj);
		}
		return series;
	}

	public static JSONArray convert(ArrayList<Plot> plots) {
		JSONArray series = new JSONArray();
		int count =0;
		for (Plot plot : plots) {
			ArrayList<Serie> list = plot.getSeries();
			for (int i = 0; i < list.size(); i++) {
				Serie s = list.get(i);
				JSONObject obj = new JSONObject();
				obj.put("label", new JSONString(plot.getModelName()+": "+s.getName()));
				JSONArray points = new JSONArray();
				for (int j = 0; j < s.getPoints().size(); j++) {
					JSONArray p = new JSONArray();
					p.set(0, new JSONNumber(s.getPoints().get(j).getX()));
					p.set(1, new JSONNumber(s.getPoints().get(j).getY()));
					points.set(j, p);
				}
				obj.put("data", points);
				series.set(count++, obj);
			}
		}
		//System.out.println(series.size());
		return series;
	}
}
