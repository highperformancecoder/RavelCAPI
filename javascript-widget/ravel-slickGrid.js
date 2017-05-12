/*
  © Ravelation Pty Ltd 2017
  Example code released under the MIT license.
*/
var grid;

function processData(ravel) {
    var gridData=[];

    var h=ravel.handle;
    h.get(ravel.handleId(0));
    var colLabels=h.sliceLabels.get();
    var xCollapsed=h.collapsed();
    var columns=[{id: "ylabel", name: "", field: "ylabel"}];
    var xtitle=h.getDescription();
    
    h.get(ravel.handleId(1));
    var rowLabels=h.sliceLabels.get();
    var ymask=h.mask();
    
    var plotlyData=[{
        type: 'surface',
        hoverinfo: 'text',
        text: [],
        z: []
    }];

    if (h.collapsed())
    {
        gridData.push({});
        plotlyData[0].text.push([]);
        plotlyData[0].text.push([]);
        plotlyData[0].z.push([]);
        plotlyData[0].z.push([]);
    }
    if (!h.collapsed())
    {
        for (var i=0; i<rowLabels.length; ++i)
        {
            gridData.push({});
            plotlyData[0].text.push([]);
            plotlyData[0].z.push([]);
            h.get(ravel.handleId(0));
            for (var j=0; !xCollapsed && j<colLabels.length; ++j)
            {
                plotlyData[0].z[i].push(0);
           }
        }
    }

    ravel.setDataCallback(function (idx,v) {
        gridData[idx[1]]["c"+idx[0]]=v;
        plotlyData[0].z[idx[1]][idx[0]]=v;
        plotlyData[0].text[idx[1]][idx[0]]=colLabels[idx[0]]+","+rowLabels[idx[1]]+"="+v;
    });
    ravel.populateData();

    // set up column labels, skipping empty columns
    h.get(ravel.handleId(0));
    var xmask=h.mask();
    for (var i=0, j=0; !h.collapsed() && i<colLabels.length; ++i)
    {
        if (!xmask[i])
        {
            columns.push({id: "c"+j, name: colLabels[i], field: "c"+j});
            j++;
        }
    }
    
    // set up row labels, skipping empty rows
    h.get(ravel.handleId(1));
    var rowLabels=h.sliceLabels.get();
    var ymask=h.mask();
    if (!h.collapsed())
        for (var i=0, j=0; i<rowLabels.length; ++i)
        {
            if (!ymask[i])
            {
                gridData[j].ylabel=rowLabels[i];
                j++;
            }
        }
    
    var options = {
        enableCellNavigation: true,
        enableColumnReorder: false
    };

    grid=new Slick.Grid("#myGrid",gridData,columns,options);

    var xtickvals=[], xticklabels=[];
    var incr=Math.floor(colLabels.length/10);
    if (incr==0) incr=1;
    for (var i=0; i<colLabels.length; i+=incr)
    {
        xtickvals.push(i);
        xticklabels.push(colLabels[i]);
    }

    var ytickvals=[], yticklabels=[];
    incr=Math.floor(rowLabels.length/10);
    if (incr==0) incr=1;
    for (var i=0; i<rowLabels.length; i+=incr)
    {
        ytickvals.push(i);
        yticklabels.push(rowLabels[i]);
    }

    var layout = {
        title: ravel.table,
//        xaxis: {
//            ticktext: colLabels
//        },
//        yaxis: {
//            ticktext: rowLabels
//        },
        scene: {
            xaxis: {
                showgrid: false,
                zeroline: false,
                tickmode: "array",
                tickvals: xtickvals,
                ticktext: xticklabels,
                title: xtitle
            },
            yaxis: {
                title: h.getDescription(),
                showline: false,
                tickmode: "array",
                tickvals: ytickvals,
                ticktext: yticklabels
            }
        }
    };
    var plot=document.getElementById("plot");
    Plotly.purge(plot);
    Plotly.plot(plot,plotlyData,layout);
};
