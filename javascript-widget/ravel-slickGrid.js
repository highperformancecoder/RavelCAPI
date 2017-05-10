/*
  © Ravelation Pty Ltd 2017
  Example code released under the MIT license.
*/
var grid;

function processData(ravel) {
    var gridData=[];

    var h=ravel.handle;
    h.get(ravel.handleId(0));
    var xmask=h.mask();
    var colLabels=h.sliceLabels.get();
    var xCollapsed=h.collapsed();
    var columns=[{id: "ylabel", name: "", field: "ylabel"}];
    for (var i=0, j=0; !h.collapsed() && i<colLabels.length; ++i)
        if (!xmask[i])
    {
        columns.push({id: "c"+j, name: colLabels[i], field: "c"+j});
        j++
    }
    var xtitle=h.getDescription();
    
    h.get(ravel.handleId(1));
    var rowLabels=h.sliceLabels.get();
    var ymask=h.mask();
    
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
    if (!h.collapsed())
    {
        for (var i=0; i<rowLabels.length; ++i)
        {
            if (!ymask[i])
                gridData.push({ylabel: rowLabels[i]});
            plotlyData[0].z.push([]);
            h.get(ravel.handleId(0));
            for (var j=0; !xCollapsed && j<rowLabels.length; ++j)
                plotlyData[0].z[i].push(0);
        }
    }

    var maxRow=0;
    ravel.setDataCallback(function (col,row,v) {
        gridData[row]["c"+col]=v;
        plotlyData[0].z[row][col]=v;
        if (maxRow<col)
            maxRow=col;
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
