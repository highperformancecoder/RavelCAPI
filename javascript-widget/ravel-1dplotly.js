/*
  © Ravelation Pty Ltd 2017
  Example code released under the MIT license.
*/

function makeRadio(row, ravel, axis, radioName, checkedName, selections)
{
    var radioBox=document.createElement("td");
    row.appendChild(radioBox);
    radioBox.setAttribute("style","border:1px solid black");
    for (var i=0; i<selections.length; ++i)
    {
        var item=document.createElement("div");
        radioBox.appendChild(item);
        item.setAttribute("class","tooltip");
        var input=document.createElement("input");
        item.appendChild(input);
        input.setAttribute("type","radio");
        input.setAttribute("name",axis+"-"+radioName);
        input.setAttribute("value",i);
        if (selections[i].value===checkedName)
        {
            input.setAttribute("checked","1");
        }
        input.ravel=ravel;
        input.axis=axis;
        input.setAttribute("onchange",'radio'+radioName+'Pushed(this)');
        var span=document.createElement("span");
        item.appendChild(span);
        span.setAttribute("class","tooltiptext");
        span.innerHTML=selections[i].tooltiptext;
    }
}

function makeSelect(row, ravel, axis, radioName, checkedName, selections)
{
    var item=document.createElement("td");
    row.appendChild(item);
//    radioBox.setAttribute("style","border:1px solid black");
    var select=document.createElement("select");
    item.appendChild(select);
    for (var i=0; i<selections.length; ++i)
    {
        var option=document.createElement("option");
        select.appendChild(option);
        option.innerHTML=selections[i].tooltiptext;
        option.setAttribute("value",i);
        if (selections[i].value===checkedName)
        {
            option.setAttribute("selected","1");
        }
        select.ravel=ravel;
        select.axis=axis;
        select.setAttribute("onchange",'radio'+radioName+'Pushed(this)');
    }
}

function Ravel1D(canvas) {
    var master=this.master=newRavel(canvas);
    var lhs=this.lhs=newRavel("hiddenCanvas");
    var rhs=this.rhs=newRavel("hiddenCanvas");
    var ravel1D=this;
    master.setRank(1);
    lhs.setRank(1);
    rhs.setRank(1);
//    lhs.master=this;
//    rhs.master=this;
    this.delete=function() {
        this.lhs.delete();
        this.rhs.delete();
        this.master.delete();
    }
    var synchroniseMaster = function() {
        var axisData={};
        function addAxisDataFrom(ravel) {
            for (var i=0; i<ravel.numHandles(); ++i)
            {
                var h=ravel.handles(i);
                var ad=axisData[h.description]=[];
                for (var j=0; j<h.numSliceLabels(); ++j)
                {
                    ad.push(h.sliceLabels(j));
                }
                h.delete();
            }
        }
        addAxisDataFrom(lhs);
        addAxisDataFrom(rhs);
        master.clear();
        for (i in axisData)
        {
            master.addHandle(i,initialiseVector(new Module.VectorString, axisData[i]));
        }
        makeCountryDefaultX(master);
        plotData(ravel1D);
    }
    lhs.dataLoadHook=synchroniseMaster;
    rhs.dataLoadHook=synchroniseMaster;
    master.onRedraw=function() {plotData(ravel1D);}
}

Ravel1D.prototype=new Object;

function radiosortPushed(input)
{
    console.log(input.axis+' '+input.value);
    input.ravel.setSort(input.axis,parseInt(input.value));
    input.ravel.onRedraw();
    input.ravel.redraw();
}

function radioreducePushed(input)
{
    input.ravel.setReductionOp(input.axis,parseInt(input.value));
    input.ravel.onRedraw();
    input.ravel.redraw();
}

function hideAxisMenus(menuID) {
    var menu=document.getElementById(menuID);
    while (menu.hasChildNodes())
        menu.removeChild(menu.firstChild);
}

function toggleAxisMenus(menuID, ravel) {
    var menu=document.getElementById(menuID);
    if (menu.hasChildNodes())
    {
        while (menu.hasChildNodes())
            menu.removeChild(menu.firstChild);
        return;
    }

    var header=document.createElement("tr");
    menu.appendChild(header);
    var item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Axis";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Reduce";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Sort";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Filtering";

    
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        var h=ravel.handles(i);
        var row=document.createElement("tr");
        menu.appendChild(row);
        var item=document.createElement("td");
        row.appendChild(item);
        item.innerHTML=h.description;

        var reductionSelector=
            [
                {"value": "sum", "tooltiptext": "sum"},
                {"value": "prod", "tooltiptext": "product"},
                {"value": "av", "tooltiptext": "average"},
                {"value": "stddev", "tooltiptext": "standard deviation"},
                {"value": "min", "tooltiptext": "minimum"},
                {"value": "max", "tooltiptext": "maximum"},
            ];
        makeSelect(row, ravel, i, "reduce", "sum", reductionSelector);
        var sortSelector=
            [
                {"value":"none", "tooltiptext": "None"},
                {"value":"forward", "tooltiptext": "Forward"},
                {"value":"reverse", "tooltiptext": "Reverse"},
                {"value":"numForward", "tooltiptext": "Numerically Forward"},
                {"value":"numReverse", "tooltiptext": "Numerically Reverse"}
            ];
        makeSelect(row, ravel, i, "sort", "reverse", sortSelector);

        item=document.createElement("td");
        row.appendChild(item);
        var input=document.createElement("input");
        item.appendChild(input);
        input.setAttribute("type","checkbox");
        input.setAttribute("name","filter");
        input.setAttribute("onchange","toggleFilter(this)");
        input.axis=i;
        input.ravel=ravel;

        h.delete;
    }
}

function toggleFilter(checkBox)
{
    checkBox.ravel.setDisplayFilter(checkBox.axis,checkBox.checked);
    checkBox.ravel.redraw();
}

function plotData(ravel) {
    // in this case, we ignore the ravel argument, but use the global ravel1, ravel2 variables

    var plotlyData=[{
        type: document.getElementById("plotType").value,
        mode: 'lines',
        x: [], y: []
   }];

    if (ravel.master.numHandles()<=ravel.master.handleIds(0)) return; //nothing to do
    var xh=ravel.master.handles(ravel.master.handleIds(0));
    var title=ravel.lhs.table;

    if (xh.collapsed())
    {
        ravel.lhs.toggleCollapsed(ravel.lhs.handleIds(0));
        ravel.rhs.toggleCollapsed(ravel.rhs.handleIds(0));
    }
    
    // arithmetic operation extracted out, as we may need to try this twice
    function tryCombine()
    {
        var slice1=ravel.lhs.hyperSlice();
        var slice2=ravel.rhs.hyperSlice();
        
        var xh=ravel.lhs.handles(ravel.lhs.handleIds(0));

        var op=document.getElementById("op1").value;
        for (var i=0; i<xh.numSliceLabels(); ++i)
        {
            var key=[{axis:xh.description, slice:xh.sliceLabels(i)}];
            var value=slice1.val(key);
            if (isFinite(value))
            {
                if (ravel.rhs.numHandles()>ravel.rhs.handleIds(0))
                {
                    var h2=ravel.rhs.handles(ravel.rhs.handleIds(0));
                    key[0].axis=h2.description;
                    h2.delete();
                    var value2=slice2.val(key);
                    if (isFinite(value2))
                    {
                        title=ravel.lhs.table+" "+op+" "+ravel.rhs.table;
                        switch (op)
                        {
                            case '+': value+=value2; break;
                            case '-': value-=value2; break;
                            case '*': value*=value2; break;
                            case '/': value/=value2; break;
                        }
                    }
                }
            }
            if (isFinite(value) || document.getElementById("allDomain").checked)
            {
                plotlyData[0].x.push(xh.sliceLabels(i));
                plotlyData[0].y.push(value);
            }
        }
        
        if (plotlyData[0].x.length==0)
        {
            // no second ravel data matched, so replace by that of ravel 1
            for (var i=0; i<xh.numSliceLabels(); ++i)
            {
                var key=[{axis:xh.description, slice:xh.sliceLabels(i)}];
                var value=slice1.val(key);
                if (isFinite(value))
                {
                    plotlyData[0].x.push(xh.sliceLabels(i));
                    plotlyData[0].y.push(value);    
                }
            }
        }
                
        slice1.delete();
        slice2.delete();
        xh.delete();
    }

    function alignHandles(slave) {
        if (slave.numHandles()>slave.handleIds(0))
        {
            // didn't combine, try rotating slave ravel to match orientation of master
            for (var i=0; i<slave.numHandles(); ++i)
            {
                if (slave.handles(i).description===xh.description)
                {
                    slave.setHandleIds([i]);
                    slave.redistributeHandles();
                }
                else // check that slicers match
                {
                    var h2=slave.handles(i);
                    for (var j=0; j<ravel.master.numHandles(); ++j)
                    {
                        var h1=ravel.master.handles(j);
                        if (h2.description===h1.description)
                        {
                            slave.setSlicer(i, h1.sliceLabel());
                            if (h2.collapsed()!=h1.collapsed())
                            {
                                slave.toggleCollapsed(i);
                            }
                        }
                        h1.delete();
                    }
                    h2.delete();
                }
            }
            slave.redraw();
        }
    }

    alignHandles(ravel.lhs);
    alignHandles(ravel.rhs);
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

    // allow manual ranges to be set
    var xmin=document.getElementById("xmin").value;
    var xmax=document.getElementById("xmax").value;
    var ymin=document.getElementById("ymin").value;
    var ymax=document.getElementById("ymax").value;
    if (xmin!="" && isFinite(xmin) && xmax!="" && isFinite(xmax))
    {layout.xaxis.range=[xmin,xmax];}
    else
    {delete layout.xaxis.range;}
    if (ymin!="" && isFinite(ymin) && ymax!="" && isFinite(ymax))
    {layout.yaxis.range=[ymin,ymax];}
    else
    {delete layout.yaxis.range;}

    var plot=document.getElementById("plot");
    Plotly.purge(plot);
    Plotly.plot(plot,plotlyData,layout);
    xh.delete();
    // for debugging memory leak problems caused by lack of finalisers
    //    in javascript
    // console.log("mem usage="+DYNAMICTOP);
};

// make country the default x-axis
function makeCountryDefaultX(ravel) {
    for (var i=0; i<ravel.numHandles(); ++i) {
        var h=ravel.handles(i);
        if (h.description==="country") {
            ravel.setHandleIds([i]);
            ravel.redistributeHandles();
        } else {
            // select reverse sorting
            ravel.setSort(i,2);
        }
        h.delete;
    }
    ravel.redraw();
}
    
function processData(ravel) {/* not used */}
function plotAllData() {plotData(ravel1); /* plotData(ravel2) */}
