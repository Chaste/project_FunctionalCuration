package fcs.shared;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.IsSerializable;


public class Plot implements IsSerializable {
	private ArrayList<Serie> series;
	private String dataPath;
	private String modelName;
	private String name;
	private String xTitle;
	private String xUnits;
	private String xFile;
	private String xType;
	private String yTitle;
	private String yUnits;
	private String yFile;
	private String yType;
	private int dimension;

	private String traces;

	public Plot() {
		series = new ArrayList<Serie>();
		name = "defaultPlotName";
	}

	public Plot(String dataPath, String name, String modelName, String xTitle, String yTitle,
			String traces) {
		this.dataPath = dataPath;
		this.name = name;
		this.modelName=modelName;
		this.xTitle = xTitle;
		this.yTitle = yTitle;
		this.traces = traces;
		series= new ArrayList<Serie>();
	}

	public String getxTitle() {
		return xTitle;
	}

	public void setxTitle(String xTitle) {
		this.xTitle = xTitle;
	}

	public String getyTitle() {
		return yTitle;
	}

	public void setyTitle(String yTitle) {
		this.yTitle = yTitle;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getName() {
		return name;
	}

	public String getTraces() {
		return traces;
	}

	public void setTraces(String traces) {
		this.traces = traces;
	}

	public void addSeries(Serie s) {
		series.add(s);
	}

	public ArrayList<Serie> getSeries() {
		return series;
	}
	public String getDataPath() {
		return dataPath;
	}

	public void setDataPath(String dataPath) {
		this.dataPath = dataPath;
	}
	public String getxUnits() {
		return xUnits;
	}

	public void setxUnits(String xUnits) {
		this.xUnits = xUnits;
	}

	public String getyUnits() {
		return yUnits;
	}

	public void setyUnits(String yUnits) {
		this.yUnits = yUnits;
	}

	public String getxFile() {
		return xFile;
	}

	public void setxFile(String xFile) {
		this.xFile = xFile;
	}

	public String getyFile() {
		return yFile;
	}

	public void setyFile(String yFile) {
		this.yFile = yFile;
	}

	public String getxType() {
		return xType;
	}

	public void setxType(String xType) {
		this.xType = xType;
	}

	public String getyType() {
		return yType;
	}

	public void setyType(String yType) {
		this.yType = yType;
	}

	public int getDimension() {
		return dimension;
	}

	public void setDimension(int dimension) {
		this.dimension = dimension;
	}

	public String getModelName() {
		return modelName;
	}

	public void setModelName(String modelName) {
		this.modelName = modelName;
	}
}
