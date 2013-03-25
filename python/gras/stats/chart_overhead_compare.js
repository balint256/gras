function GrasChartOverheadCompare(args)
{
    //save enables
    this.ids = args.block_ids;

    //input checking
    if (this.ids.length <= 1) throw gras_error_dialog(
        "GrasChartOverheadCompare",
        "Error making overhead compare chart.\n"+
        "Specify at least 2 blocks for this chart."
    );

    //make new chart
    this.chart = new google.visualization.PieChart(args.panel);

    this.title = "Overhead Comparison";
}

GrasChartOverheadCompare.prototype.update = function(point)
{
    var data_set = new Array();
    data_set.push(['Task', 'Percent']);
    $.each(this.ids, function(index, id)
    {
        var percents = gras_extract_percent_times(point, id);
        data_set.push([id, percents['total']]);
    });

    var data = google.visualization.arrayToDataTable(data_set)

    var options = {
        width:$('#page').width()/5,
        chartArea:{left:5,top:0,right:5,bottom:0,width:"100%",height:"100%"},
    };

    this.chart.draw(data, options);
};