package fcs.client.graph;

import java.util.ArrayList;
import java.util.List;

import com.google.gwt.core.client.JavaScriptObject;
import com.google.gwt.core.client.JsArrayInteger;
import com.google.gwt.json.client.JSONObject;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.Event;
import com.google.gwt.user.client.Random;
import com.google.gwt.user.client.ui.Widget;

import fcs.client.JsonConverter;
import fcs.client.LegendContainer;
import fcs.shared.Plot;

public class Graph extends Widget {
	private ArrayList<Plot> plots;
	private LegendContainer legend;
	private LegendContainer hiddenLegend;
	private LegendContainer overview;
	private JSONObject m_defaultOptions;
	private List<GraphListener> m_listeners = new ArrayList<GraphListener>();

	public Graph(ArrayList<Plot> p, JSONObject options, LegendContainer l,
			LegendContainer l2, LegendContainer l3) {
		super();
		plots = p;
		legend = l;
		hiddenLegend = l2;
		overview = l3;
		String id = "" + Random.nextInt();
		Element divEle = DOM.createDiv();
		setElement(divEle);
		divEle.setId(id);

		m_defaultOptions = options;
	}

	@Override
	protected void onLoad() {
		super.onLoad();
		Plot plot = plots.get(0);
		JavaScriptObject data = null;
		if (plots.size() == 1) {
			data = JsonConverter.convert(plot).getJavaScriptObject();
		} else {
			data = JsonConverter.convert(plots).getJavaScriptObject();
		}
		createGraphJS(this, getElement().getId(), legend.getId(),
				hiddenLegend.getId(), overview.getId(), plot.getxTitle() + "["
						+ plot.getxUnits() + "]",
				plot.getyTitle() + "[" + plot.getyUnits() + "]", data, null);
	}

	private native void createGraphJS(Graph x, String id, String id2,
			JavaScriptObject data, JavaScriptObject options) /*-{}-*/;

	private void fireClickEvent(Event e, JsArrayInteger a, JavaScriptObject o) {
		System.out.println("clicked");
	}

	private void fireHoverEvent(Event e, JsArrayInteger a, JavaScriptObject o) {
		System.out.println("hovered");
	}

	private native void createGraphJS(Graph x, String plotCont,
			String legendCont, String hLegCont, String overviewCont,
			String xTitle, String yTitle, JavaScriptObject datasets,
			JavaScriptObject options) /*-{

$wnd.plotObject = (function(){
	
		d = $doc.getElementById(plotCont);
		d.style.width = "400px";
		d.style.height = "400px";

		d2 = $doc.getElementById(overviewCont);
		d2.style.width = "200px";
		d2.style.height = "200px";
				
		main_options = getDefaultMainPanelOptions();

		function getDefaultMainPanelOptions() {
			var default_options = {
				legend : {
					show : false
				},
				series : {
					lines : {
						show : true
					},
					points : {
						show : true
					}
				},grid : {
				clickable : true,
				hoverable : true
			},
				selection : {
					mode : "xy"
				},xaxis: {
            axisLabel: xTitle,
            axisLabelUseCanvas: true,
        },
        yaxis: {
            axisLabel: yTitle,
            axisLabelUseCanvas: true
        }
			};
			
			return default_options;
		}
		;

		overview_options = {
			legend : {
				show : true,
				container : $wnd.$("#" + hLegCont)
			},
			series : {
				lines : {
					show : true,
					lineWidth : 1
				},
				shadowSize : 0
			},
			xaxis : {
				//ticks : 2,
				//axisLabel: ' ',
           // axisLabelUseCanvas: true,
			},
			yaxis : {
				//ticks : 2,
				//axisLabel: ' ',
            //axisLabelUseCanvas: true,
			},
			grid : {
				//color : '#111'
				clickable : true,
				autoHighlight: false,
				hoverable : false
			},
			selection : {
				mode : 'xy'
			}
		};

		// hard-code color indices to prevent them from shifting as things are turned on/off 
		colors=[]
		i = 0
		$wnd.$.each(datasets, function(key, val) {
			colors.push(i)
			val.color = i
			++i
		})
		
		
		
		// insert checkboxes 
		choiceContainer = $wnd.$("#" + legendCont);
		
			$wnd.$.each(datasets, function(key, val) {
			li = $wnd.$('<li />').appendTo(choiceContainer);

			checkBox = $wnd.$('<input name="'+ key
						+ '" id="'+val.color
						+ '" type="checkbox" checked="checked" OnClick="plotObject.click();"/>').appendTo(li);
						
			
			$wnd.$('<label>', { text : val.label, 'for': val.color}).appendTo(li);
		});
		
		data=[]
		choiceContainer.find("input:checked").each(function () {
                                                   var key = $wnd.$(this).attr("name");
                                                   if (key && datasets[key]) data.push(datasets[key]);
                                                 });
						
		
		//console.log("colors:"+colors)

		//plot the original graph now and retain a holder for it.
		plot = $wnd.$.plot($wnd.$("#" + plotCont), data, main_options);
		
		currentRanges = { xaxis: { from: plot.getAxes().xaxis.min, to: plot.getAxes().xaxis.max},
                   yaxis: {from: plot.getAxes().yaxis.min , to: plot.getAxes().yaxis.max} };
		// Make the plot overview retain the original axes, even when different data is plotted.
		overview_options = $wnd.$.extend(true, {}, overview_options, {
			xaxis : {
				min : plot.getAxes().xaxis.min,
				max : plot.getAxes().xaxis.max
			},
			yaxis : {
				min : plot.getAxes().yaxis.min,
				max : plot.getAxes().yaxis.max
			}
		});
		
		//plot the overview graph and legend and retain a holder for it.
		overview = $wnd.$.plot($wnd.$("#" + overviewCont), datasets,
				overview_options);
		overview.setSelection(currentRanges, true);

		//console.log(overview_options)
		//console.log('#'+legendCont+' > .legendColorBox > div')
		$wnd.$('#'+hLegCont+' td.legendColorBox > div').each(
			function(i) {
				//console.log(i)
				div = $wnd.$(this).clone()
				//div.onClick = 
				div.insertAfter(choiceContainer.find("input").eq(i));
		});
				
		function processRanges(ranges){
			// clamp the zooming to prevent eternal zoom 
			if (ranges.xaxis.to - ranges.xaxis.from < 0.00001)
				ranges.xaxis.to = ranges.xaxis.from + 0.00001;
			if (ranges.yaxis.to - ranges.yaxis.from < 0.00001)
				ranges.yaxis.to = ranges.yaxis.from + 0.00001;
				
			main_options = $wnd.$.extend(true, {}, main_options, {
				xaxis : {
					min : ranges.xaxis.from,
					max : ranges.xaxis.to
				},
				yaxis : {
					min : ranges.yaxis.from,
					max : ranges.yaxis.to
				}
			});
			
			currentRanges=ranges
								
		}
	
		
		
		



    var count = 0
   
	function dataChanged(){
		count++
		console.log('replot '+ count)
		data =[]
		choiceContainer.find("input:checked").each(function () {
                                                   var key = $wnd.$(this).attr("name");
                                                   if (key && datasets[key]) data.push(datasets[key]);
                                                 });
		rePlot()
      
    }
    
    function rePlot(){
  
    if (data.length > 0) {
      	//console.log('plot')
        plot = $wnd.$.plot($wnd.$("#"+plotCont), data, main_options);
        overview = $wnd.$.plot($wnd.$("#"+overviewCont), data, overview_options);
     	overview.setSelection(currentRanges, true);
      }else{
    	alert('error')
    }
    
    }
    

   

    // Reset button
    //$('#<portlet_2_0:namespace />_clear_selection_trace_' + job_identifier).click(resetEverything);

    


		var previousPoint = null;
		$wnd
				.$("#" + plotCont)
				.bind(
						"plothover",
						function(event, pos, item) {
							$wnd.$("#x").text(pos.x.toFixed(2));
							$wnd.$("#y").text(pos.y.toFixed(2));

							if (item) {
								if (previousPoint != item.datapoint) {
									previousPoint = item.datapoint;

									$wnd.$("#tooltip").remove();
									var x = item.datapoint[0].toFixed(2), y = item.datapoint[1]
											.toFixed(2);

									showTooltip(item.pageX, item.pageY,
											item.series.label + " ["+x+"," + y+"]");
								}
							} else {
								$wnd.$("#tooltip").remove();
								previousPoint = null;
							}
						});



function showTooltip(x, y, contents) {
			$wnd.$('<div id="tooltip">' + contents + '</div>').css({
				position : 'absolute',
				display : 'none',
				top : y + 5,
				left : x + 15,
				border : '1px solid #fdd',
				padding : '2px',
				backgroundColor : '#fee',
				//opacity : 0.90
			}).appendTo("body").fadeIn(200);
		}
	
	// now connect the two - this method sets the "selection" ranges on both plots, and replots the main plot. 
		$wnd.$("#"+plotCont).bind("plotselected", function(event, ranges) {
			processRanges(ranges)
			rePlot()
		});
	//This method just calls the method on the main plot above.
	$wnd.$("#"+overviewCont).bind("plotselected", function (event, ranges) {
		plot.setSelection(ranges, false);
	});
	
	$wnd.$("#"+overviewCont).bind("plotclick", function (event, ranges) {
		overview.setSelection(currentRanges, true);
	});
 //If inputs are detected on the checkboxes fire off this method:
    //choiceContainer.find("input").click(plotAccordingToChoices);
    //choiceContainer.find("input").change(dataChanged);
    //leave the hiding until last (otherwise flot may complain
    //$wnd.$("#"+overviewCont).hide();
    $wnd.$("#"+hLegCont).hide();
    
  return{
  	reset : function () {
      console.log('inside reset')
      // Check all the boxes.
      choiceContainer.find("input").each(function () {
                                           if ($wnd.$(this).attr("checked") == false) {
                                             $wnd.$(this).click();
                                           }
                                         });
      //Let the main plot choose its own axes again
	main_options = getDefaultMainPanelOptions();
	currentRanges = { xaxis: { from: overview.getAxes().xaxis.min, to: overview.getAxes().xaxis.max},
                   yaxis: {from: overview.getAxes().yaxis.min , to: overview.getAxes().yaxis.max} };
	dataChanged();
      
    },
    click : function(){
    	dataChanged()
    }
  };

})()
	}-*/;

	public native void reset() /*-{

		console.log('reset')
		// Reset method.
		$wnd.plotObject.reset()

	}-*/;

}
/*
 * 
 * 
 * 
 * 
 * choiceContainer.text("");
 * 
 * $wnd.$.each(datasets, function(key, val) { // class legendColorBox defined in
 * jquery.flot.js in function insertLegend() var legendColorBox =
 * $wnd.$('.legendColorBox:eq(' + val.color + ') div div'); var new_span =
 * $wnd.$('<span>&nbsp;&nbsp;</span>'); // moz wasn't happy using border-color,
 * which ie didn't mind! var colour = legendColorBox.css('border-left-color');
 * new_span.css('background-color', colour);
 * 
 * choiceContainer.append('<input type="checkbox"\ name="' + key + '" \
 * checked="checked" \ id="id' + key + '" />&nbsp;');
 * choiceContainer.append(new_span); choiceContainer.append('&nbsp;<label
 * for="id' + key + '"> ' + val.label + '</label>'); choiceContainer.append('<br
 * />'); });
 * 
 * 
 * //console.log('key: '+key+' val: '+val+'color:'+val.color) //var
 * legendColorBox = $wnd.$('.legendColorBox:eq(' + val.color + ') div div');
 * //var colorBoxCopy = legendColorBox.clone()
 * //legendColorBox.insertAfter(checkBox)
 */

