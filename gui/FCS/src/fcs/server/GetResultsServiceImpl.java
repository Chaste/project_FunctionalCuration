package fcs.server;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import fcs.client.GetResultsService;
import fcs.shared.MenuData;
import fcs.shared.Plot;
import fcs.shared.PlotData;
import fcs.shared.Point;
import fcs.shared.Serie;

import au.com.bytecode.opencsv.CSVReader;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;

/**
 * The server side implementation of the RPC service.
 */
@SuppressWarnings("serial")
public class GetResultsServiceImpl extends RemoteServiceServlet implements
		GetResultsService {

	public static final int MAX_ARRAY_SIZE = 1000;
	public static final String resultsDir = "CellModelTestOutput";
	public static final String protocolsSourceDir = "CellModelTestInput/Protocols";
	public static final String modelsSourceDir = "CellModelTestInput/Models";

	static final String defaultPlots = "outputs-default-plots.csv";
	static final String contents = "outputs-contents.csv";
	static final String timeFile = "outputs-steppers.csv";
	static final String timeName = "Time";
	static final String timeUnits = "ms";
	static final String timeType = "no_type";
	static final String plotPrefix = "Plot: ";
	static final String xPrefix = "X: ";
	static final String yPrefix = "Y: ";
	static final String tracesPrefix = "Traces: ";

	@Override
	public PlotData getResults(ArrayList<String> models, String protocol)
			throws IllegalArgumentException {
		PlotData data = new PlotData();
		CSVReader br = null;
		
		for (String model : models) {
			String dataPath = resultsDir + "/" + model + "/" + protocol + "/";
			// Create a pattern to match breaks
			// Pattern pattern = Pattern.compile("[,\\s]+");

			try {

				br = new CSVReader(new FileReader(dataPath + defaultPlots));

				try {
					// read title line
					String[] line = br.readNext();

					// read 1st data line
					line = br.readNext();

					while (line != null) {
						// System.out.println(input.substring(1,
						// input.length()-1));
						String optionalSecondVar = null;
						if (line.length == 3) {
							optionalSecondVar = line[2];
						}
						//System.out.println("New plot:" + dataPath + ";"
							//	+ line[0] + ";" + model + ";" + line[1] + ";"
								//+ optionalSecondVar);

						Plot p = new Plot(dataPath, line[0], model, line[1],
								optionalSecondVar);
						populatePlot(p);

						data.addPlot(p);
						line = br.readNext();
					}

				} finally {
					br.close();
				}

			} catch (IOException ioe) {
				ioe.printStackTrace();
				throw new IllegalArgumentException(
						"Your arguments are invalid!");
			}

		}
		return data;
	}

	@Override
	public MenuData getMenuData() {
		MenuData data = new MenuData();
		// add the models
		File dir = new File(modelsSourceDir);
		String[] child = dir.list();
		if (child != null) {
			for (String model : child) {
				
				if (model.charAt(0) != '.') {
					
					// remove .cellml
					String name = model.substring(0, model.length() - 7);
					if(checkResultsAvailable(name)){
					data.addModel(name);
				}
				}
			}
		}
		// add the protocols
		dir = new File(protocolsSourceDir);
		child = dir.list();
		if (child != null) {
			for (String protocol : child) {
				if (protocol.charAt(0) != '.') {
					// remove .xml
					String name = protocol.substring(0, protocol.length() - 4);
					data.addProtocol(name);
				}
			}
		}
		return data;
	}

	private void populatePlot(Plot p) throws IOException {
		if (checkTypes(p)) {
			if (p.getDimension() == 0) {
				// Not handled yet.
			} else if (p.getDimension() == 1) {
				ArrayList<Point> dataArray = getData1D(p);
				Serie s = new Serie(p.getName());
				for (Point point : dataArray) {
					s.addPoint(point);
				}
				p.addSeries(s);
			} else if (p.getDimension() == 2) {
				ArrayList<ArrayList<Point>> dataArray = getData2D(p);
				for (int j = 0; j < dataArray.get(0).size(); j++) {
					Serie s = new Serie("Trace :" + j);
					for (int i = 0; i < dataArray.size(); i++) {
						s.addPoint(dataArray.get(i).get(j));
					}
					p.addSeries(s);
				}
			} else if (p.getDimension() == 3) {
				// ArrayList<ArrayList<ArrayList<Point>>> dataArray =
				// getData3D(p);
			} else {
				// ?
			}

		}

	}
	
	private boolean checkResultsAvailable(String model){
		//System.out.println(resultsDir+"/"+model);
		File dir = new File(resultsDir+"/"+model);
		boolean success=true;
		try{
		String[] protocols = dir.list();
		
		for(String prot:protocols){
			//System.out.println(resultsDir+"/"+model+"/"+prot);
			File dir2 = new File(resultsDir+"/"+model+"/"+prot);
			String[] files = dir2.list();
			boolean contain=false;
			for(String file:files){
				//System.out.println("File: "+file);
				if(file.equals("success")){
					contain=true;
				}
			}
			if(!contain){
				success=false;
			}
		}
		}catch(NullPointerException n){
			success=false;
		}
		return success;
	}

	private boolean checkTypes(Plot p) throws IOException {

		CSVReader csvReader = new CSVReader(new FileReader(p.getDataPath()
				+ contents));
		boolean xChecked = false;
		boolean yChecked = false;
		int xDimension = -1;
		int yDimension = -1;

		if (p.getyTitle() == null) {
			yChecked = true;
			p.setyTitle(timeName);
			p.setyFile(p.getDataPath() + timeFile);
			p.setyUnits(timeUnits);
			p.setyType(timeType);
		}
		try {
			// read title line
			String[] line = csvReader.readNext();
			line = csvReader.readNext();
			while ((!(xChecked && yChecked)) && line != null) {

				if (line[0].equals(p.getxTitle())) {
					p.setxUnits(line[2]);
					xDimension = Integer.valueOf(line[3]);
					p.setxFile(p.getDataPath() + line[4]);
					p.setxType(line[5]);
					xChecked = true;
				} else if (line[0].equals(p.getyTitle())) {
					p.setyUnits(line[2]);
					yDimension = Integer.valueOf(line[3]);
					p.setyFile(p.getDataPath() + line[4]);
					p.setyType(line[5]);
					yChecked = true;
				}

				line = csvReader.readNext();
			}

		} finally {
			csvReader.close();
		}

		if (xChecked && yChecked) {
			//System.out.println("one");
			if (yDimension == -1) {
				yDimension = xDimension;
			}
			if (xDimension != -1 && (xDimension == yDimension)) {
			//	System.out.println("two");
				p.setDimension(xDimension);
				return true;
			}
		}
		System.out.println("bad three");
		return false;
	}

	private ArrayList<Point> getData1D(Plot p) throws IOException {

		CSVReader brX = new CSVReader(new FileReader(p.getxFile()));
		CSVReader brY = new CSVReader(new FileReader(p.getyFile()));

		ArrayList<Point> data = new ArrayList<Point>();
		try {

			// read 2 title lines
			String[] inputX = brX.readNext();
			String[] inputY = brY.readNext();
			inputX = brX.readNext();
			inputY = brY.readNext();

			inputX = brX.readNext();
			inputY = brY.readNext();

			while (inputX != null && inputY != null) {

				for (int i = 0; i < inputX.length; i++) {
					try {
						data.add(new Point(Double.valueOf(inputX[i]), Double
								.valueOf(inputY[i])));

					} catch (NumberFormatException e) {
						// just skip line if format problem
					}
				}

				inputX = brX.readNext();
				inputY = brY.readNext();
			}

		} finally {
			brX.close();
			brY.close();
		}
		return data;
	}

	private ArrayList<ArrayList<Point>> getData2D(Plot p) throws IOException {
		CSVReader brX = new CSVReader(new FileReader(p.getxFile()));
		ArrayList<ArrayList<Point>> data = new ArrayList<ArrayList<Point>>();

		if (p.getyTitle().equals(timeName)) {
			ArrayList<String> timeValues = getTimeValues(p);

			// read title line
			String[] inputX = brX.readNext();
			inputX = brX.readNext();
			int j=0;
			while (inputX != null) {
				
				ArrayList<Point> l = new ArrayList<Point>();
				for (int i = 0; i < inputX.length; i++) {
					try {
						
						l.add(new Point(Double
								.valueOf(timeValues.get(j)),Double.valueOf(inputX[i])));

					} catch (NumberFormatException e) {
						// just skip line if format problem
						//e.printStackTrace();
						//System.out.println(timeValues.get(j));
					}
				}
				
				data.add(l);
				//System.out.println(j);
				j++;
				inputX = brX.readNext();
			}
		} else {
			CSVReader brY = new CSVReader(new FileReader(p.getyFile()));
			try {

				// read title line
				String[] inputX = brX.readNext();
				String[] inputY = brY.readNext();

				inputX = brX.readNext();
				inputY = brY.readNext();

				while (inputX != null && inputY != null) {

					ArrayList<Point> l = new ArrayList<Point>();
					for (int i = 0; i < inputX.length; i++) {
						try {
							l.add(new Point(Double.valueOf(inputX[i]), Double
									.valueOf(inputY[i])));

						} catch (NumberFormatException e) {
							// just skip line if format problem
						}
					}
					data.add(l);

					inputX = brX.readNext();
					inputY = brY.readNext();
				}

			} finally {
				brX.close();
				brY.close();
			}
		}
		return data;
	}

	private ArrayList<String> getTimeValues(Plot p) throws IOException {
		CSVReader reader = new CSVReader(new FileReader(p.getDataPath()
				+ timeFile));
		ArrayList<String> values = null;
		// read title line
		String[] line = reader.readNext();
		while (line != null) {
			if (line[0].equals("time")) {
				values = new ArrayList<String>();
				for (int i = 2; i < line.length; i++) {
					values.add(line[i]);
				}
				break;
			}

			line = reader.readNext();
		}
		return values;

	}

	// private ArrayList<ArrayList<ArrayList<Point>>> getData3D(Plot p)
	// throws IOException {

	// BufferedReader brX = new BufferedReader(new FileReader(p.getxFile()));
	// BufferedReader brY = new BufferedReader(new FileReader(p.getyFile()));
	/*
	 * ArrayList<ArrayList<ArrayList<Point>>> data = new
	 * ArrayList<ArrayList<ArrayList<Point>>>(); try {
	 * 
	 * 
	 * // read title line String inputX = brX.readLine(); String inputY =
	 * brY.readLine();
	 * 
	 * inputX = brX.readLine(); inputY = brY.readLine();
	 * 
	 * while (inputX != null && inputY != null) { String[] valuesX =
	 * inputX.split("\t", GetResultsServiceImpl.MAX_ARRAY_SIZE); String[]
	 * valuesY = inputY.split("\t", GetResultsServiceImpl.MAX_ARRAY_SIZE);
	 * 
	 * ArrayList<Point> l = new ArrayList<Point>(); for (int i = 0; i <
	 * valuesX.length; i++) { try { l.add(new Point(Double.valueOf(valuesX[i]),
	 * Double.valueOf(valuesY[i])));
	 * 
	 * } catch (NumberFormatException e) { // just skip line if format problem }
	 * } data.add(l);
	 * 
	 * inputX = brX.readLine(); inputY = brY.readLine(); }
	 * 
	 * } finally { brX.close(); brY.close(); }
	 */
	// return null;
	// }
}
