var op='+';

function processData(ravel) {
    // in this case, we ignore the ravel argument, but use the global ravel1, ravel2 variables

    var plotlyData=[{
        type: 'scatter',
        mode: 'lines',
        x: [], y: []
   }];

    if (ravel1.numHandles()<=ravel1.handleIds(0)) return; //nothing to do
    var xh=ravel1.handles(ravel1.handleIds(0));
    var title=ravel1.table;

    if (xh.collapsed())
    {
        ravel1.toggleCollapsed(ravel1.handleIds(0));
        ravel1.redraw();
    }
    
    // arithmetic operation extracted out, as we may need to try this twice
    function tryCombine()
    {
        var slice1=ravel1.hyperSlice();
        var slice2=ravel2.hyperSlice();
        
        var xh=ravel1.handles(ravel1.handleIds(0));
            
        for (var i=0; i<xh.numSliceLabels(); ++i)
        {
            var key=[{axis:xh.description, slice:xh.sliceLabels(i)}];
            var value=slice1.val(key);
            if (isFinite(value))
            {
                if (ravel2.numHandles()>ravel2.handleIds(0))
                {
                    key[0].axis=ravel2.handles(ravel2.handleIds(0)).description;
                    var value2=slice2.val(key);
                    if (isFinite(value2))
                    {
                        title=ravel1.table+" "+op+" "+ravel2.table;
                        switch (op)
                        {
                            case '+': value+=value2; break;
                            case '-': value-=value2; break;
                            case '*': value*=value2; break;
                            case '/': value/=value2; break;
                        }
                    }
                }
                plotlyData[0].x.push(xh.sliceLabels(i));
                plotlyData[0].y.push(value);        
            }
        }
        slice1.delete();
        slice2.delete();
    }

    //tryCombine();
    //if (title===ravel1.table && ravel2.numHandles()>ravel2.handleIds(0))
    if (ravel2.numHandles()>ravel2.handleIds(0))
    {
        // didn't combine, try rotating ravel2 to match orientation of ravel1
        for (var i=0; i<ravel2.numHandles(); ++i)
        {
            if (ravel2.handles(i).description===xh.description)
            {
                ravel2.setHandleIds([i]);
                ravel2.redistributeHandles();
            }
            else // check that slicers match
            {
                var h2=ravel2.handles(i);
                for (var j=0; j<ravel1.numHandles(); ++j)
                {
                    var h1=ravel1.handles(j);
                    if (h2.description===h1.description)
                    {
                        ravel2.setSlicer(i, h1.sliceLabel());
                        if (h2.collapsed()!=h1.collapsed())
                        {
                            ravel2.toggleCollapsed(i);
                        }
                    }
                    h1.delete();
                }
                h2.delete();
            }
        }
        ravel2.redraw();
    }
    tryCombine();

    var layout = {
        title: title,
        xaxis: {
            showgrid: false,
            zeroline: false,
        },
        yaxis: {
            showline: false,
        }
    };
    layout.xaxis.title=xh.description;
    Plotly.newPlot(document.getElementById("plot"),plotlyData,layout);
    xh.delete();
};
