/*
  © Ravelation Pty Ltd 2017
  Example code released under the MIT license.
*/
var grid;

function processData(ravel) {
    var gridData=[];

    var h=ravel.handle;
    h.get(ravel.handleId(0));
    var columns=[{id: "ylabel", name: "", field: "ylabel"}];
    for (var i=0; !h.collapsed() && i<h.sliceLabels.size(); ++i)
        columns.push({id: "c"+i, name: h.sliceLabelAt(i), field: "c"+i});
    var xtitle=h.getDescription();
    
    h.get(ravel.handleId(1));
    
    var plotlyData=[{
        type: 'surface',
        z: []
    }];

    if (h.collapsed())
    {
        gridData.push({});
        plotlyData[0].z.push([]);
        plotlyData[0].z.push([]);
    }
    for (var i=0; !h.collapsed() && i<h.sliceLabels.size(); ++i)
    {
        gridData.push({ylabel: h.sliceLabelAt(i)});
        plotlyData[0].z.push([]);
    }

    var maxRowLength=0;
    ravel.setDataCallback(function (col,row,v) {
        gridData[row]["c"+col]=v;
        plotlyData[0].z[row][col]=v;
        if (maxRowLength<=col)
            maxRowLength=col+1;
    });
    ravel.populateData();

    var options = {
        enableCellNavigation: true,
        enableColumnReorder: false
    };
    
    grid=new Slick.Grid("#myGrid",gridData,columns,options);

    var layout = {
        title: ravel.table,
        xaxis: {
            showgrid: false,
            zeroline: false,
            title: xtitle
        },
        yaxis: {
            title: h.getDescription(),
            showline: false,
        }
    };
    var plot=document.getElementById("plot");
    Plotly.purge(plot);
    Plotly.plot(plot,plotlyData,layout);
};
