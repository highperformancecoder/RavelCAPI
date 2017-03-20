var grid;

function processData(ravel) {
    var gridData=[];

    var xh=ravel.handles(ravel.handleIds(0));
    var columns=[{id: "ylabel", name: "", field: "ylabel"}];
    for (var i=0; !xh.collapsed() && i<xh.numSliceLabels(); ++i)
        columns.push({id: "c"+i, name: xh.sliceLabels(i), field: "c"+i});
    
    var yh=ravel.handles(ravel.handleIds(1));

    var plotlyData=[{
        type: 'surface',
        //type: 'scatter3d',
        /*x: [], y: [],*/ z: []
    }];

    if (yh.collapsed())
    {
        gridData.push({});
//        plotlyData[0].x.push([]);
//        plotlyData[0].y.push([]);
        plotlyData[0].z.push([]);
        plotlyData[0].z.push([]);
    }
    for (var i=0; !yh.collapsed() && i<yh.numSliceLabels(); ++i)
    {
        gridData.push({ylabel: yh.sliceLabels(i)});
//        plotlyData[0].x.push([]);
//        plotlyData[0].y.push([]);
        plotlyData[0].z.push([]);
    }

    var maxRowLength=0;
    ravel.setDataCallback(function (col,row,v) {
        gridData[row]["c"+col]=v;
//        plotlyData[0].x[row][col]=xh.sliceLabels(col);
//        plotlyData[0].y[row][col]=yh.sliceLabels(row);
        plotlyData[0].z[row][col]=v;
        if (maxRowLength<=col)
            maxRowLength=col+1;
    });
    ravel.populateData();

//    //ensure arrays all the same size by adding some dummy element
//    for (var i=0; i<plotlyData[0].z.length; ++i)
//        if (plotlyData[0].z[i].length<maxRowLength)
//            plotlyData[0].z[i][maxRowLength-1]=0;
    
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
            title: xh.description
//            type: "category"
        },
        yaxis: {
            title: yh.description,
            showline: false,
//            type: "category"
        }
    };
    var plot=document.getElementById("plot");
    Plotly.purge(plot);
    Plotly.plot(plot,plotlyData,layout);
};
