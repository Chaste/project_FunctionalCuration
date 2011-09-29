package fcs.server;

import java.io.BufferedReader;
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

import com.google.gwt.user.server.rpc.RemoteServiceServlet;

/**
 * The server side implementation of the RPC service.
 */


@SuppressWarnings("serial")
public class GetResultsServiceImpl extends RemoteServiceServlet implements
		GetResultsService {

	public static final int MAX_ARRAY_SIZE = 1000;
	public static final String modelsDir = "../CellModelTestOutput";
	public static final String protocolsDir="CellModelTestInput/Protocols";
	
	
	static final String defaultPlots = "default-plots.csv";
	static final String contents = "contents.csv";
	static final String plotPrefix = "Plot: ";
	static final String xPrefix = "X: ";
	static final String yPrefix = "Y: ";
	static final String tracesPrefix = "Traces: ";

	@Override
	public PlotData getResults(ArrayList<String> models, String protocol, String xVar,
			String yVar) throws IllegalArgumentException {
		PlotData data = new PlotData();
		BufferedReader br = null;
		for(String model:models){
		String dataPath = modelsDir + "/" + model + "/" + protocol + "/";
		 // Create a pattern to match breaks
        //Pattern pattern = Pattern.compile("[,\\s]+");
        
		if (xVar == null || yVar == null) {
			try {

				br = new BufferedReader(new FileReader(dataPath + defaultPlots));

				try {
					// read title line
					String input = br.readLine();
					String[]split = input.split("," , MAX_ARRAY_SIZE);
					int numberParams = split.length;

					input = br.readLine();

					String[] plotInfo;
					while (input != null) {
						//System.out.println(input.substring(1, input.length()-1));
						plotInfo = input.split(",", numberParams);
						String traces = null;
						if (plotInfo.length >= 4) {
							traces = plotInfo[3];
						}
						Plot p = new Plot(dataPath, plotInfo[0], model, plotInfo[1],
								plotInfo[2], traces);
						populatePlot(p);

						data.addPlot(p);
						input = br.readLine();
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
		}
		// System.out.println("before return");
		return data;
	}

	@Override
	public MenuData getMenuData() {
		MenuData data = new MenuData();
		File dir = new File(modelsDir);
		String[] child = dir.list();
		if (child != null) {
			for (String model : child) {
				if (model.charAt(0) != '.') {
					data.addModel(model);
				}
			}
		}
		// System.out.println(modelsDir + "/" + child[child.length - 1]);
		File dir2 = new File(modelsDir + "/" + child[child.length - 1]);
		String[] child2 = dir2.list();
		if (child2 != null) {
			for (String protocol : child2) {
				if (protocol.charAt(0) != '.') {
					data.addProtocol(protocol);
				}
			}
		}
		return data;
	}

	private void populatePlot(Plot p) throws IOException {
		if (checkTypes(p)) {
			if (p.getDimension() == 0) {
				// ?
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
					Serie s = new Serie("Trace :"+j);
					for (int i = 0; i < dataArray.size(); i++) {
						s.addPoint(dataArray.get(i).get(j));
					}
					p.addSeries(s);
				}
			} else if (p.getDimension() == 3) {
				//ArrayList<ArrayList<ArrayList<Point>>> dataArray = getData3D(p);
			} else {
				// ?
			}

		}

	}

	private boolean checkTypes(Plot p) throws IOException {
		BufferedReader br = new BufferedReader(new FileReader(p.getDataPath()
				+ contents));
		boolean xChecked = false;
		boolean yChecked = false;
		int xDimension = -1;
		int yDimension = -1;
		try {
			// read title line
			String input = br.readLine();
			input = br.readLine();
			System.out.println(input);
			System.out.println("xTitle :"+p.getxTitle());
			System.out.println("yTitle :"+p.getyTitle());
			while ((!(xChecked && yChecked)) && input != null) {
				String[] values = input.split(",",
						GetResultsServiceImpl.MAX_ARRAY_SIZE);
				/*
				if (values.length != 5) {
					throw new IOException("Incorrect " + contents
							+ " File format");
				}*/
				System.out.println(values[0]);
				if (values[0].equals(p.getxTitle())) {
					p.setxUnits(values[1]);
					xDimension = Integer.valueOf(values[2]);
					p.setxFile(p.getDataPath()+values[3]);
					p.setxType(values[4]);
					xChecked = true;
				} else if (values[0].equals(p.getyTitle())) {
					p.setyUnits(values[1]);
					yDimension = Integer.valueOf(values[2]);
					p.setyFile(p.getDataPath()+values[3]);
					p.setyType(values[4]);
					yChecked=true;
				}

				input = br.readLine();
			}

		} finally {
			br.close();
		}

		if (xChecked && yChecked) {
			System.out.println("one");
			if (xDimension != -1 && (xDimension == yDimension)) {
				System.out.println("two");
				p.setDimension(xDimension);
				return true;
			}
		}
		System.out.println("bad three");
		return false;
	}

	private ArrayList<Point> getData1D(Plot p) throws IOException {

		BufferedReader brX = new BufferedReader(new FileReader(p.getxFile()));
		BufferedReader brY = new BufferedReader(new FileReader(p.getyFile()));

		ArrayList<Point> data = new ArrayList<Point>();
		try {

			// read title line
			String inputX = brX.readLine();
			String inputY = brY.readLine();

			inputX = brX.readLine();
			inputY = brY.readLine();

			while (inputX != null && inputY != null) {
				String[] valuesX = inputX.split("\t",
						GetResultsServiceImpl.MAX_ARRAY_SIZE);
				String[] valuesY = inputY.split("\t",
						GetResultsServiceImpl.MAX_ARRAY_SIZE);

				for (int i = 0; i < valuesX.length; i++) {
					try {
						data.add(new Point(Double.valueOf(valuesX[i]), Double
								.valueOf(valuesY[i])));

					} catch (NumberFormatException e) {
						// just skip line if format problem
					}
				}

				inputX = brX.readLine();
				inputY = brY.readLine();
			}

		} finally {
			brX.close();
			brY.close();
		}
		return data;
	}

	private ArrayList<ArrayList<Point>> getData2D(Plot p) throws IOException {

		BufferedReader brX = new BufferedReader(new FileReader(p.getxFile()));
		BufferedReader brY = new BufferedReader(new FileReader(p.getyFile()));

		ArrayList<ArrayList<Point>> data = new ArrayList<ArrayList<Point>>();
		try {

			// read title line
			String inputX = brX.readLine();
			String inputY = brY.readLine();

			inputX = brX.readLine();
			inputY = brY.readLine();

			while (inputX != null && inputY != null) {
				String[] valuesX = inputX.split("\t",
						GetResultsServiceImpl.MAX_ARRAY_SIZE);
				String[] valuesY = inputY.split("\t",
						GetResultsServiceImpl.MAX_ARRAY_SIZE);

				ArrayList<Point> l = new ArrayList<Point>();
				for (int i = 0; i < valuesX.length; i++) {
					try {
						l.add(new Point(Double.valueOf(valuesX[i]), Double
								.valueOf(valuesY[i])));

					} catch (NumberFormatException e) {
						// just skip line if format problem
					}
				}
				data.add(l);

				inputX = brX.readLine();
				inputY = brY.readLine();
			}

		} finally {
			brX.close();
			brY.close();
		}
		return data;
	}

//	private ArrayList<ArrayList<ArrayList<Point>>> getData3D(Plot p)
	//		throws IOException {

		//BufferedReader brX = new BufferedReader(new FileReader(p.getxFile()));
		//BufferedReader brY = new BufferedReader(new FileReader(p.getyFile()));
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
		 * valuesX.length; i++) { try { l.add(new
		 * Point(Double.valueOf(valuesX[i]), Double.valueOf(valuesY[i])));
		 * 
		 * } catch (NumberFormatException e) { // just skip line if format
		 * problem } } data.add(l);
		 * 
		 * inputX = brX.readLine(); inputY = brY.readLine(); }
		 * 
		 * } finally { brX.close(); brY.close(); }
		 */
		//return null;
	//}
}
